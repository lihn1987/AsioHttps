#ifndef __ASIO_HTTPS_H__
#define __ASIO_HTTPS_H__
#include <string>
#include <memory>
#include <thread>
#include <functional>
#include <boost/asio.hpp>
#include <boost/unordered_map.hpp>
struct AsioHttpsRequest{
  std::string url_;
};
struct AsioHttpsResponse{
  std::string response_;
};
class AsioHttpsSocket{
public:
  AsioHttpsSocket(boost::asio::io_service& ios);
  ~AsioHttpsSocket();
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

  /**
   * @brief 传入请求，在收到回复后调用回调函数
   * @param config
   */
  bool Process(std::shared_ptr<AsioHttpsRequest> config, std::function<void(std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)>);
private:
  uint8_t thread_count_;
  std::vector<std::shared_ptr<std::thread>> thread_list_;
  boost::asio::io_service ios_;
  boost::unordered_map<std::shared_ptr<AsioHttpsRequest>, std::function<void(std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>)>>
    process_list_;
};
#endif
