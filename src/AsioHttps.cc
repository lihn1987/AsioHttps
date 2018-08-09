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

void AsioHttpsSocket::ConnectToHost(const std::string &host)
{
  std::vector<std::string> splite_string;
  boost::algorithm::split(splite_string, host, boost::algorithm::is_any_of(":"));
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
  boost::asio::ip::tcp::resolver rslv(ios_);
  boost::asio::ip::tcp::resolver::query qry(addr, port);
  resolver_.async_resolve(qry, boost::bind(&AsioHttpsSocket::OnResolveAddr, this, _1, _2));
}

void AsioHttpsSocket::OnResolveAddr(const boost::system::error_code &err, boost::asio::ip::tcp::resolver::iterator rit){
  if(err){
    std::cout<<"resolver faild..."<<err.message()<<std::endl;
  }else{
    std::cout<<"resolver success:"<<rit->endpoint().address()
             <<":"<<rit->endpoint().port()<<std::endl;
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
