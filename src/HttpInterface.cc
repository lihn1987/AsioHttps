#include "HttpInterface.h"
#include <iostream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
HttpRequestHead::HttpRequestHead():
  attribute_(
    {//这里并没有列出所有的属性，所列的只是常用的
      //浏览器可接受的MIME类型。
      std::pair<std::string, std::string>("accept","*/*"),
      std::pair<std::string, std::string>("content-type", "application/x-www-form-urlencoded"),
      //Keep-Alive长连
      //Close短连
      std::pair<std::string, std::string>("connection","Keep-Alive"),
      //表示请求消息正文的长度。后面自动填写
      std::pair<std::string, std::string>("content-length",""),
      //cookie用于做身份证明
      std::pair<std::string, std::string>("cookie",""),
      //初始URL中的主机和端口。
      std::pair<std::string, std::string>("host","www.hao123.com"),
      //浏览器类型标识，没有特殊原因直接用默认的这个应该没啥问题
      std::pair<std::string, std::string>("user-agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/68.0.3440.75 Chrome/68.0.3440.75 Safari/537.36")
    }){

}

void HttpRequestHead::SetAttribute(const std::string &name, const std::string &value){
  attribute_[boost::algorithm::to_lower_copy(name)] = value;
}

void HttpRequestHead::DeleteAttribute(const std::string &name){
  attribute_.erase(attribute_[boost::algorithm::to_lower_copy(name)]);
}

std::string HttpRequestHead::GetAttribute(const std::string &name){
  return attribute_[boost::algorithm::to_lower_copy(name)];
}

boost::unordered_map<std::string, std::string> HttpRequestHead::GetAllAttribute(){
  return attribute_;
}

std::string HttpRequestMsgStruct::ToString(){
  head_.SetAttribute("Content-Length", boost::lexical_cast<std::string>(body_.size()));

  std::string rtn;
  rtn += head_.method_;
  rtn += " "+head_.url_;
  rtn += " "+head_.version+"\r\n";

  for(auto item:head_.GetAllAttribute()){
    if(item.second !=""){
      rtn += item.first+": "+item.second+"\r\n";
    }
  }
  rtn += "\r\n";
  return rtn;
}

int32_t HttpResponseMsgStruct::FromString(const std::string &str_in){
  str_ori_ = str_in;
  auto finded_iter = boost::algorithm::find_first(str_in, "\r\n\r\n");
  if(finded_iter.empty()){
    //http头还没有收完
    return -1;
  }else{
    std::string str_head(str_in.begin(), finded_iter.begin());
    std::string str_body(finded_iter.end(), str_in.end());

    boost::cmatch what;
    boost::regex regex("^(.*?)\\s+(.*?)\\s+(.*?)$(.*)$");
    boost::regex_match(str_head.c_str(), what, regex);
    if(what.size() != 5){
      throw "Bad Status line";
      return -1;
    }else{
      //解析状态行
      head_.version_ = boost::algorithm::trim_copy(what[1]);
      head_.code_ = boost::algorithm::trim_copy(what[2]);
      head_.status = boost::algorithm::trim_copy(what[3]);
    }
    regex = boost::regex("^\\s*(.*?)\\s*:\\s*(.*?)\\s*$\\s*(.*)\\s*$");
    str_head = boost::algorithm::trim_copy(what[4].str());

    if(!boost::regex_match(str_head.c_str(), what, regex)){
      throw "Bad Head";
    }
    while (what.size() == 4) {
      head_.attribute_[boost::algorithm::to_lower_copy(what[1].str())] =
          boost::algorithm::to_lower_copy(what[2].str());
      str_head = what[3].str();
      if(!boost::regex_match(str_head.c_str(), what, regex) ){
        if(str_head == ""){
          break;
        }
        throw "Bad Head";
      }
    }
    if(head_.attribute_.find("content-length") == head_.attribute_.end()){
      //此为采用chunked编码将内容分块输出
      std::string  str_body_tmp = str_body;
      regex = boost::regex("^(.*?)\\r\\n(.*)$");
      body_ = "";
      while(boost::regex_match(str_body_tmp.c_str(), what, regex)){
        std::string chuck_len_str = what[1].str();
        std::istringstream stm(chuck_len_str);
        uint32_t chuck_len;
        stm>>std::hex>>chuck_len;
        if(chuck_len==0){
          return 0;
        }else{
          if(str_body_tmp.size()>chuck_len+2){
            str_body_tmp = what[2].str();
            body_.append(str_body_tmp.c_str()+2, chuck_len);
            str_body_tmp.erase(0, chuck_len+2);
          }else{
            break;
          }
        }
      }
      return -1;
      throw "Did not found content-length";

    }
    size_t content_length = boost::lexical_cast<size_t>(head_.attribute_["content-length"]);
    size_t remaind_len = content_length-str_body.size();

    if(remaind_len == 0)
      body_.assign(finded_iter.end(), str_in.end());
    return remaind_len;
  }
}



