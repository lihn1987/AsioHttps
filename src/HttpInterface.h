#ifndef __HTTP_INTERFACE_H__
#define __HTTP_INTERFACE_H__
#include <string>
#include <boost/unordered_map.hpp>
enum HttpMothod{
  HM_GET = 0,//请求服务器的文档，未支持
  HM_HEAD = 1,//请求关于文档的信息，但不是这个文档本身，未支持
  HM_POST = 2,//从客户向服务器发送一些信息，未支持
  HM_PUT= 3,//从服务器向客户发送文档，未支持
  HM_TRACE = 4,//把到达的请求回送，未支持
  HM_CONNECT = 5,//保留，未支持
  HM_DELETE = 6,//删除Web网页，未支持
  HM_OPTIONs = 7//询问关于可用的选项，未支持
};


struct HttpHead{
  //方法类型
  HttpMothod method_ = HM_GET;
  std::string url_="/";           //uri部分
  std::string version="HTTP/1.1"; //版本
  boost::unordered_map<std::string, std::string> attribute_=
  {//这里并没有列出所有的属性，所列的只是常用的
    //浏览器可接受的MIME类型。
    std::pair<std::string, std::string>("Accept","*/*"),

   /*
      application/xhtml+xml ：XHTML格式
      application/xml     ： XML数据格式
      application/atom+xml  ：Atom XML聚合格式
      application/json    ： JSON数据格式
      application/pdf       ：pdf格式
      application/msword  ： Word文档格式
      application/octet-stream ： 二进制流数据（如常见的文件下载）
      application/x-www-form-urlencoded ： <form encType=””>中默认的encType，form表单数据被编码为key/value格式发送到服务器（表单默认的提交数据的格式）
      另外一种常见的媒体格式是上传文件之时使用的：
      multipart/form-data ： 需要在表单中进行文件上传时，就需要使用该格式
    */
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
    std::pair<std::string, std::string>("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/68.0.3440.75 Chrome/68.0.3440.75 Safari/537.36\r\n")
  };
};
struct HttpMsgStruct{
  HttpHead head_;//http的头内容
  std::string body_;//http的body内容
  //组合一条http的消息包
  std::string ToString();
};

struct ProxyConfig{
  std::string url_;
  uint16_t port_;
};
#endif
