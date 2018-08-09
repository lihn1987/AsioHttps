#ifndef __ASIO_HTTPS_H__
#define __ASIO_HTTPS_H__
#include <string>
#include <memory>
#include <thread>
#include <functional>
#include <boost/asio.hpp>
#include <boost/unordered_map.hpp>

#include "HttpInterface.h"
typedef  HttpMsgStruct AsioHttpsRequest ;
struct AsioHttpsResponse{
  std::string response_;

};
class AsioHttpsSocket{
public:
  AsioHttpsSocket(boost::asio::io_service& ios);
  ~AsioHttpsSocket();
public:
  bool Process(std::shared_ptr<AsioHttpsRequest> request, std::function<void(std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)> response);
private:
  void ConnectToAddress(const std::string& ip,  uint16_t port);
  void ConnectToHost(const std::string& host);
  void OnResolveAddr(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator rit);
private:
  boost::asio::io_service& ios_;
  std::list<std::pair<std::shared_ptr<AsioHttpsRequest>, std::function<void(std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)>>> process_list_;
  ProxyConfig proxy_config_;
  bool ssl;
  std::string host_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver resolver_;
};

class AsioHttps{
public:
  /**
   * @brief 设置处理网络信息的线程数目
   * @param thread_count 线程数目
   */
  AsioHttps(uint8_t thread_count);
public:
  /**
   * @brief 创建一个socket
   * @return
   */
  std::shared_ptr<AsioHttpsSocket> CreateAsioHttpSocket();
private:
  uint8_t thread_count_;
  std::vector<std::shared_ptr<std::thread>> thread_list_;
  boost::asio::io_service ios_;
  boost::asio::io_service::work work_;


  boost::unordered_map<std::shared_ptr<AsioHttpsRequest>, std::function<void(std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)>>
    process_list_;
};
#endif
