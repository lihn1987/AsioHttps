#ifndef __HTTP_INTERFACE_H__
#define __HTTP_INTERFACE_H__
#include <string>
#include <boost/unordered_map.hpp>

struct HttpRequestHead{
public:
  HttpRequestHead();
public:
  std::string method_ = "GET";
  std::string url_="/";           //uri部分
  std::string version="HTTP/1.1"; //版本
  void SetAttribute(const std::string& name, const std::string& value);
  void DeleteAttribute(const std::string& name);
  std::string GetAttribute(const std::string& name);
  boost::unordered_map<std::string, std::string> GetAllAttribute();
private:
  /*
    不能直接操作attribute，而需要通过接口
    之所以如此操作，是为了在使用attribute的时候能够不区分大小写
  */
  boost::unordered_map<std::string, std::string> attribute_;
};

struct ProxyConfig{
  bool operator == (const ProxyConfig& it){
    return (url_ == it.url_ &&
            port_ == it.port_);
  }
  bool operator != (const ProxyConfig& it){
    return !(*this == it);
  }
  std::string url_;
  uint16_t port_;
};

struct HttpRequestConfig{
  ProxyConfig proxy_config_;
  bool ssl_;
};

struct HttpRequestMsgStruct{
  HttpRequestHead head_;//http的头内容
  std::string body_;//http的body内容
  //组合一条http的消息包
  std::string ToString();
  HttpRequestConfig config_;
};

struct HttpResponseHead{
  std::string version_;
  std::string code_;
  std::string status;
  boost::unordered_map<std::string, std::string> attribute_;
};

struct HttpResponseMsgStruct{
  HttpResponseHead head_;
  std::string body_;
  std::string str_ori_;//原始的http返回的字符串
  std::string error_;//接收到该信息是，若error_为空表示消息正确，否则表示通讯过程中发生了失败
  //返回值表示还需要多少个字节才是完整的包
  //-1表示无法计算
  int32_t FromString(const std::string& str_in);
};
#endif
