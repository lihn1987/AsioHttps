#ifndef __HTTP_INTERFACE_H__
#define __HTTP_INTERFACE_H__
#include <string>
#include <boost/unordered_map.hpp>

struct HttpRequestHead{
  //方法类型
  std::string method_ = "GET";
  std::string url_="/";           //uri部分
  std::string version="HTTP/1.1"; //版本
  boost::unordered_map<std::string, std::string> attribute_=
  {//这里并没有列出所有的属性，所列的只是常用的
    //浏览器可接受的MIME类型。
    std::pair<std::string, std::string>("Accept","*/*"),
    std::pair<std::string, std::string>("Content-type", "application/x-www-form-urlencoded"),
    //Keep-Alive长连
    //Close短连
    std::pair<std::string, std::string>("Connection","Keep-Alive"),
    //表示请求消息正文的长度。后面自动填写
    std::pair<std::string, std::string>("Content-Length",""),
    //cookie用于做身份证明
    std::pair<std::string, std::string>("Cookie",""),
    //初始URL中的主机和端口。
    std::pair<std::string, std::string>("Host","www.hao123.com"),
    //浏览器类型标识，没有特殊原因直接用默认的这个应该没啥问题
    std::pair<std::string, std::string>("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/68.0.3440.75 Chrome/68.0.3440.75 Safari/537.36")
  };
};
struct HttpRequestMsgStruct{
  HttpRequestHead head_;//http的头内容
  std::string body_;//http的body内容
  //组合一条http的消息包
  std::string ToString();
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

struct ProxyConfig{
  std::string url_;
  uint16_t port_;
};
#endif
