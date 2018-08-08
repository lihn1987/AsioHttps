#include "AsioHttps.h"
#include <boost/bind.hpp>
AsioHttpsSocket::AsioHttpsSocket(boost::asio::io_service& ios){

}

AsioHttpsSocket::~AsioHttpsSocket()
{

}

AsioHttps::AsioHttps(uint8_t thread_count):thread_count_(thread_count){
  for(decltype(thread_count) i = 0; i <= thread_count; i++){
    thread_list_.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &ios_)));
  }
}

std::shared_ptr<AsioHttpsSocket> AsioHttps::CreateAsioHttpSocket(){
  return std::shared_ptr<AsioHttpsSocket>(new AsioHttpsSocket(ios_));
}

bool AsioHttps::Process(std::shared_ptr<AsioHttpsRequest> config, std::function<void (std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)>){
  return false;
}
