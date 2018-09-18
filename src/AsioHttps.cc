#include "AsioHttps.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
AsioHttpsSocket::AsioHttpsSocket(boost::asio::io_service& ios, boost::asio::ssl::context& ctx):
  ios_(ios),socket_(ios),
  socket_ssl_(ios, ctx),
  resolver_(ios){
}

AsioHttpsSocket::~AsioHttpsSocket()
{

}

bool AsioHttpsSocket::Process(const std::string &url, ResponseCallback response){
  boost::cmatch what;
  boost::regex regex("^(.+)://([a-zA-Z0-9\\.]+)(.*)$");
  if(boost::regex_match(url.c_str(), what, regex)){
    std::shared_ptr<AsioHttpsRequest> request = std::make_shared<AsioHttpsRequest>();
    request->head_.SetAttribute("host", what[2].str());
    request->head_.url_ = what[3].str();
    std::string proto = what[1].str();
    //std::cout<<"host:"<<request->head_.GetAttribute("host")<<std::endl;
    //std::cout<<"url:"<<request->head_.url_<<std::endl;

    if(proto == "http"){
      request->config_.ssl_ = false;
    }else if(proto == "https") {
      request->config_.ssl_ = true;
    }else{
      return false;
    }
    return Process(request, response);
  }else{
    return false;
  }
}

bool AsioHttpsSocket::Process(const std::string &url, const ProxyConfig &config, ResponseCallback response){
  boost::cmatch what;
  boost::regex regex("^(.+)://([a-zA-Z0-9\\.]+)(.*)$");
  if(boost::regex_match(url.c_str(), what, regex)){
    std::shared_ptr<AsioHttpsRequest> request = std::make_shared<AsioHttpsRequest>();
    request->head_.SetAttribute("host", what[2].str());
    request->head_.url_ = what[3].str();
    std::string proto = what[1].str();
    //std::cout<<"host:"<<request->head_.GetAttribute("host")<<std::endl;
    //std::cout<<"url:"<<request->head_.url_<<std::endl;

    if(proto == "http"){
      request->config_.ssl_ = false;
    }else if(proto == "https") {
      request->config_.ssl_ = true;
    }else{
      return false;
    }
    request->config_.use_proxy_ = true;
    request->config_.proxy_ = config;
    return Process(request, response);
  }else{
    return false;
  }
}

bool AsioHttpsSocket::Process(std::shared_ptr<AsioHttpsRequest> request, ResponseCallback response){
  std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  if(request->config_.websocket_){
    websocekt_ = true;
  }
  process_list_.push_back(std::make_pair(request, response));
  if(process_list_.size() == 1){//之前的都已处理
    DoProcess();
  }
  return true;
}

bool AsioHttpsSocket::SocketSend(const std::string &str_send)
{
  if(ssl_){
    boost::system::error_code ec;
    std::string send_buf = str_send;
    while(send_buf.size()){
      size_t size = socket_ssl_.write_some(boost::asio::buffer(send_buf), ec);
      if(ec){
        std::cout<<ec.message()<<std::endl;
        return false;
      }
      send_buf.erase(0, size);
      if(send_buf.size()){
        boost::this_thread::sleep(boost::posix_time::millisec(1));
      }
    }
    return true;
  }else{
    boost::system::error_code ec;
    std::string send_buf = str_send;
    while(send_buf.size()){
      size_t size = socket_.write_some(boost::asio::buffer(send_buf), ec);
      if(ec){
        std::cout<<ec.message()<<std::endl;
        return false;
      }
      send_buf.erase(0, size);
      if(send_buf.size()){
        boost::this_thread::sleep(boost::posix_time::millisec(1));
      }
    }
    return true;
  }
}

void AsioHttpsSocket::ConnectToHost(const std::string& host_url){
  if(use_proxy_){
    //链接socks5代理
    boost::system::error_code ec;
    boost::asio::ip::tcp::endpoint end_point(boost::asio::ip::address::from_string(proxy_.url_), proxy_.port_);
    if(ssl_){
      socket_ssl_.lowest_layer().connect(end_point,
                                        ec);
    }else{
      socket_.connect(end_point, ec);
    }
    if(ec){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();

      return;
    }

    unsigned char buf[1024] = {0x05, 0x01, 0x00};
    if(ssl_){
      boost::asio::write(socket_ssl_.next_layer(), boost::asio::buffer(buf, 3), boost::asio::transfer_all(), ec);
    }else{
      boost::asio::write(socket_, boost::asio::buffer(buf, 3), boost::asio::transfer_all(), ec);
    }
    if(ec){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }

    if(ssl_){
      boost::asio::read(socket_ssl_.next_layer(), boost::asio::buffer(buf, 2), boost::asio::transfer_exactly(2), ec);
    }else{
      boost::asio::read(socket_, boost::asio::buffer(buf, 2), boost::asio::transfer_exactly(2), ec);
    }
    if(ec){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }
    if(buf[0] != 0x05 || buf[1] != 0x00){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }
    //开始链接
    int idx = 0;
    buf[idx++] = 0x05;
    buf[idx++] = 0x01;
    buf[idx++] = 0x00;
    buf[idx++] = 0x03;

    std::string url = process_list_.front().first->head_.GetAttribute("host");
    buf[idx++] = url.size();
    memcpy(&buf[idx], url.data(), url.size());
    idx += url.size();
    uint16_t port = process_list_.front().first->config_.ssl_?443:80;
    //memcpy(&buf[idx], (char*)&port, sizeof(port));
    buf[idx++]=((char*)&port)[1];
    buf[idx++]=((char*)&port)[0];
    if(ssl_){
      boost::asio::write(socket_ssl_.next_layer(), boost::asio::buffer(buf, idx), boost::asio::transfer_all(), ec);
    }else{
      boost::asio::write(socket_, boost::asio::buffer(buf, idx), boost::asio::transfer_all(), ec);
    }
    if(ec){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }

    if(ssl_){
      boost::asio::read(socket_ssl_.next_layer(), boost::asio::buffer(buf, 10), boost::asio::transfer_exactly(10), ec);
    }else{
      boost::asio::read(socket_, boost::asio::buffer(buf, 10), boost::asio::transfer_exactly(10), ec);
    }
    if(ec){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }
    int version = buf[0];
    int response = buf[1];
    if(version != 5){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }
    if (response != 0){
      std::thread trd(boost::bind(&AsioHttpsSocket::ErrorProcess, this, ec.message()));
      trd.detach();
      return;
    }
    OnConnect(boost::system::error_code());
    return;
  }
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
    //std::cout<<"ip:"<<rit->endpoint().address()<<std::endl;
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
      socket_ssl_.set_verify_mode(boost::asio::ssl::verify_none);
      socket_ssl_.set_verify_callback(boost::asio::ssl::rfc2818_verification(process_list_.front().first->head_.GetAttribute("host")));
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
    //std::cout<<send_buf_<<std::endl;
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
      send_buf_.clear();
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
    if(websocekt_){
      std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
      std::pair<std::shared_ptr<AsioHttpsRequest>, ResponseCallback> process_pair =  process_list_.front();
      std::shared_ptr<HttpResponseMsgStruct> msg = std::make_shared<HttpResponseMsgStruct>();
      if(websocket_head_ == false){
        read_buf_.insert( read_buf_.end(), (char*)read_buf_tmp_, (char*)read_buf_tmp_ + size);
        size_t pos = read_buf_.find("\r\n\r\n");

        if(pos!= std::string::npos){
          if(pos+4 != read_buf_.size()){
            std::string str_tmp;
            str_tmp = read_buf_.substr(pos+4);
            msg->str_ori_ = str_tmp;
            process_pair.second(process_pair.first, msg);
          }
          websocket_head_ = true;
        }
      }else{
        msg->str_ori_.assign(read_buf_tmp_, size);
        process_pair.second(process_pair.first, msg);
      }
      if(ssl_){
        socket_ssl_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }else{
        socket_.async_read_some(boost::asio::buffer(read_buf_tmp_, MAX_BUF_SIZE), boost::bind(&AsioHttpsSocket::OnRead, this, _1, _2));
      }
      return;
    }
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
      process_pair.second(process_pair.first, msg);
      process_list_.pop_front();
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
    if(pair.first->head_.GetAttribute("host") != host_ ||
       pair.first->config_.ssl_ != ssl_ ||
       pair.first->config_.proxy_ != proxy_ ||
       pair.first->config_.use_proxy_ != use_proxy_ || last_error_)
    {///要连接的对象和原来的不相等了
      //进行配置
      read_buf_.clear();
      send_buf_.clear();
      websocket_head_ = false;
      last_error_ = false;
      host_ = pair.first->head_.GetAttribute("host");
      ssl_ = pair.first->config_.ssl_;
      use_proxy_ = pair.first->config_.use_proxy_;
      proxy_ = pair.first->config_.proxy_;

      CloseSocket();
      ConnectToHost(pair.first->head_.GetAttribute("host"));

    }else{
      //相同的地址可以复用socket,直接跳过连接部分
      last_error_ = false;
      websocket_head_ = false;
      read_buf_.clear();
      if(!ssl_){
        OnConnect(boost::system::error_code());
      }else{
        OnHandShake(boost::system::error_code());
      }
    }
  }
}

void AsioHttpsSocket::ErrorProcess(const std::string &str_err){
  std::lock_guard<std::recursive_mutex> lk(process_list_mutex_);
  //接收失败，调用函数返回
  last_error_ = true;
  std::pair<std::shared_ptr<AsioHttpsRequest>, ResponseCallback> process_pair =  process_list_.front();
  //process_list_.pop_front();
  std::shared_ptr<HttpResponseMsgStruct> msg = std::make_shared<HttpResponseMsgStruct>();
  if(str_err != ""){
    msg->error_ = str_err;
  }else{
    msg->error_ = "unknow error";
  }
  CloseSocket();
  process_pair.second(process_pair.first, msg);
  process_list_.pop_front();
  //进行下一个process
  DoProcess();
}

void AsioHttpsSocket::CloseSocket(){
  try{
    boost::system::error_code ec;
    socket_.close(ec);
    socket_ssl_.shutdown(ec);
    socket_ssl_.lowest_layer().shutdown(boost::asio::socket_base::shutdown_both,ec);
    socket_ssl_.lowest_layer().close(ec);
    socket_ssl_.next_layer().shutdown(boost::asio::socket_base::shutdown_both,ec);
    socket_ssl_.next_layer().close(ec);
  }catch(...){

  }
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
