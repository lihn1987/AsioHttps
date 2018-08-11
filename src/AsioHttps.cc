#include "AsioHttps.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

AsioHttpsSocket::AsioHttpsSocket(boost::asio::io_service& ios, boost::asio::ssl::context& ctx):
  ios_(ios),socket_(ios),
  socket_ssl_(ios, ctx),
  resolver_(ios){
}

AsioHttpsSocket::~AsioHttpsSocket()
{

}

bool AsioHttpsSocket::Process(std::shared_ptr<AsioHttpsRequest> request, ResponseCallback response){
  std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  process_list_.push_back(std::make_pair(request, response));
  if(process_list_.size() == 1){//之前的都已处理
    DoProcess();
  }
  return true;
}

void AsioHttpsSocket::ConnectToHost(const std::string& host_url){
  /*std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  std::vector<std::string> splite_string;
  boost::algorithm::split(splite_string, host_url, boost::algorithm::is_any_of(":"));
  std::string addr,port;
  if(splite_string.size() == 1){
    addr = splite_string[0];
    port = "80";
  }else if(splite_string.size() == 2){
    addr = splite_string[0];
    port = splite_string[1];
  }else{
    process_list_.pop_front();
    socket_.close();
    return;
  }*/
  std::string proto;
  if(ssl_){
    proto = "https";
  }else{
    proto = "http";
  }
  boost::asio::ip::tcp::resolver::query qry(host_url, proto);
  resolver_.async_resolve(qry, boost::bind(&AsioHttpsSocket::OnResolveAddr, this, _1, _2));
}

void AsioHttpsSocket::OnResolveAddr(const boost::system::error_code &err, boost::asio::ip::tcp::resolver::iterator rit){
  if(err){
    ErrorProcess(err.message());
  }else{
    if(ssl_){
      boost::asio::async_connect(socket_ssl_.lowest_layer(), rit,
              boost::bind(&AsioHttpsSocket::OnConnect, this, _1));

    }else{
      ConnectToIP(rit->endpoint());
    }
  }
}

void AsioHttpsSocket::ConnectToIP(const boost::asio::ip::tcp::endpoint &end_point){
  socket_.async_connect(end_point, boost::bind(&AsioHttpsSocket::OnConnect, this, _1));
}

void AsioHttpsSocket::ConnectToIP(const std::string &ip, uint16_t port){
  boost::asio::ip::tcp::endpoint end_point(boost::asio::ip::tcp::v4(), port);
  end_point.address(boost::asio::ip::address_v4::from_string(ip));
  ConnectToIP(end_point);
}

void AsioHttpsSocket::OnConnect(const boost::system::error_code &err){
  std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  if(err){
    ErrorProcess(err.message());
  }else{
    send_buf_ = process_list_.front().first->ToString();
    if(ssl_){
      socket_ssl_.set_verify_mode(boost::asio::ssl::verify_peer);

      //socket_ssl_.set_verify_callback(verify_callback);
      socket_ssl_.set_verify_callback(boost::asio::ssl::rfc2818_verification(process_list_.front().first->head_.attribute_["Host"]));
      socket_ssl_.async_handshake(boost::asio::ssl::stream_base::client, boost::bind(&AsioHttpsSocket::OnHandShake, this, _1));
    }else{
      socket_.async_send(boost::asio::buffer(send_buf_), boost::bind(&AsioHttpsSocket::OnSend, this, _1, _2));
    }
  }
}

void AsioHttpsSocket::OnHandShake(const boost::system::error_code &err){
  if(err){
    ErrorProcess(err.message());
  }else{
    send_buf_ = process_list_.front().first->ToString();
    socket_ssl_.async_write_some(boost::asio::buffer(send_buf_), boost::bind(&AsioHttpsSocket::OnSend, this, _1, _2));
  }
}

void AsioHttpsSocket::OnSend(const boost::system::error_code &err, std::size_t size){
  if(err){
    ErrorProcess(err.message());
  }else{
    send_buf_.erase(0, size);
    if(send_buf_.size()){
      if(ssl_){
        socket_ssl_.async_write_some(boost::asio::buffer(send_buf_), boost::bind(&AsioHttpsSocket::OnSend, this, _1, _2));
      }else{
        socket_.async_send(boost::asio::buffer(send_buf_), boost::bind(&AsioHttpsSocket::OnSend, this, _1, _2));
      }
    }else{
      //发送完成，开始接收
      read_buf_.clear();
      if(ssl_){
        socket_ssl_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }else{
        socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }
    }
  }
}

void AsioHttpsSocket::OnRead(const boost::system::error_code &err, std::size_t size){
  if(err){
    ErrorProcess(err.message());
  }else{
    read_buf_tmp_[size] = 0;
    //std::cout<<"*************"<<read_buf_tmp_<<"**********"<<std::endl;
    read_buf_.insert( read_buf_.end(), (char*)read_buf_tmp_, (char*)read_buf_tmp_ + size);
    std::shared_ptr<HttpResponseMsgStruct> msg = std::make_shared<HttpResponseMsgStruct>();
    int32_t tmp_len = msg->FromString(read_buf_);
    if(tmp_len == -1){
      //http头还没接收完
      if(ssl_){
        socket_ssl_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }else{
        socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }
    }else if(tmp_len > 0){
      //http头接收完了，然后接收剩余的body
      if(ssl_){
        socket_ssl_.async_read_some(boost::asio::buffer(read_buf_tmp_, std::min<int32_t>(MAX_BUF_SIZE, tmp_len)), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }else{
        socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, std::min<int32_t>(MAX_BUF_SIZE, tmp_len)), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }
    }else if(tmp_len == 0){
      //接收完成
      std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
      std::pair<std::shared_ptr<AsioHttpsRequest>, ResponseCallback> process_pair =  process_list_.front();
      process_list_.pop_front();
      process_pair.second(process_pair.first, msg);
      DoProcess();
    }else{
      assert(0);
    }
  }
}

void AsioHttpsSocket::DoProcess(){
  std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  if(!process_list_.empty()){
    std::pair<std::shared_ptr<AsioHttpsRequest> , ResponseCallback> pair = process_list_.front();
    if(pair.first->head_.attribute_["Host"] != host_){
      socket_.close();
      ConnectToHost(pair.first->head_.attribute_["Host"]);
    }else{
      //相同的地址可以复用socket,直接跳过连接部分
      OnConnect(boost::system::error_code());
    }
  }
}

void AsioHttpsSocket::ErrorProcess(const std::string &str_err){
  std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  //接收失败，调用函数返回
  std::pair<std::shared_ptr<AsioHttpsRequest>, ResponseCallback> process_pair =  process_list_.front();
  process_list_.pop_front();
  std::shared_ptr<HttpResponseMsgStruct> msg = std::make_shared<HttpResponseMsgStruct>();
  if(str_err != ""){
    msg->error_ = str_err;
  }else{
    msg->error_ = "unknow error";
  }
  process_pair.second(process_pair.first, msg);
  //进行下一个process
  DoProcess();
}



AsioHttps::AsioHttps(uint8_t thread_count):thread_count_(thread_count), work_(ios_){
  for(decltype(thread_count) i = 0; i <= thread_count; i++){
    thread_list_.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &ios_)));
  }
}

std::shared_ptr<AsioHttpsSocket> AsioHttps::CreateAsioHttpSocket(){
  boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
  ctx.set_default_verify_paths();
  return std::shared_ptr<AsioHttpsSocket>(new AsioHttpsSocket(ios_, ctx));
}
