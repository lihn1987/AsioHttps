#include "AsioHttps.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
AsioHttpsSocket::AsioHttpsSocket(boost::asio::io_service& ios):ios_(ios),socket_(ios),resolver_(ios){

}

AsioHttpsSocket::~AsioHttpsSocket()
{

}

bool AsioHttpsSocket::Process(std::shared_ptr<AsioHttpsRequest> request, std::function<void (std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)> response){
  process_list_.push_back(std::make_pair(request, response));
  if(process_list_.size() == 1){
    //之前的都已处理
    if(request->head_.attribute_["Host"] != host_){
      socket_.close();
      std::cout<<request->head_.attribute_["Host"]<<std::endl;
      ConnectToHost(request->head_.attribute_["Host"]);
    }
  }
  return true;
}

void AsioHttpsSocket::ConnectToHost(const std::string& host_url)
{
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
  }

  boost::asio::ip::tcp::resolver::query qry(addr, port);
  resolver_.async_resolve(qry, boost::bind(&AsioHttpsSocket::OnResolveAddr, this, _1, _2));
}

void AsioHttpsSocket::OnResolveAddr(const boost::system::error_code &err, boost::asio::ip::tcp::resolver::iterator rit){
  if(err){
    std::cout<<"resolver faild..."<<err.message()<<std::endl;
  }else{
    std::cout<<"resolver success:"<<rit->endpoint().address()
             <<":"<<rit->endpoint().port()<<std::endl;
    ConnectToIP(rit->endpoint());
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
  if(err){
    std::cout<<"Connect faild:"<<err.message()<<std::endl;
  }else{
    std::cout<<"Connect success!"<<std::endl;
    send_buf_ = process_list_.front().first->ToString();
    socket_.async_send(boost::asio::buffer(send_buf_), boost::bind(&AsioHttpsSocket::OnSend, this, _1, _2));
  }
}

void AsioHttpsSocket::OnSend(const boost::system::error_code &err, std::size_t size){
  if(err){
    std::cout<<"Send error:"<<err.message()<<std::endl;
  }else{
    std::cout<<"Send Success:"<<size<<":"<<send_buf_<<std::endl;
    send_buf_.erase(0, size);
    if(send_buf_.size()){
      socket_.async_send(boost::asio::buffer(send_buf_), boost::bind(&AsioHttpsSocket::OnSend, this, _1, _2));
    }else{
      //发送完成，开始接收
      read_buf_.clear();
      socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
    }
  }
}

void AsioHttpsSocket::OnRead(const boost::system::error_code &err, std::size_t size){
  if(err){
    std::cout<<"Read error:"<<err.message()<<std::endl;
  }else{
    read_buf_.insert( read_buf_.end(), (char*)read_buf_tmp_, (char*)read_buf_tmp_ + size);
    HttpResponseMsgStruct msg;
    int32_t tmp_len = msg.FromString(read_buf_);
    if(tmp_len == -1){
      //http头还没接收完
      socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
    }else if(tmp_len > 0){
      //http头接收完了，然后接收剩余的body
      socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, std::min<int32_t>(MAX_BUF_SIZE, tmp_len)), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
    }else if(tmp_len == 0){
      //接收完成
      //TODO
    }else{
      assert(0);
    }
  }
}



AsioHttps::AsioHttps(uint8_t thread_count):thread_count_(thread_count), work_(ios_){
  for(decltype(thread_count) i = 0; i <= thread_count; i++){
    thread_list_.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &ios_)));
  }
}

std::shared_ptr<AsioHttpsSocket> AsioHttps::CreateAsioHttpSocket(){
  return std::shared_ptr<AsioHttpsSocket>(new AsioHttpsSocket(ios_));
}
