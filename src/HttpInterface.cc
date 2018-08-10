#include "HttpInterface.h"
#include <iostream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
std::string HttpRequestMsgStruct::ToString(){
  head_.attribute_["Content-Length"] = boost::lexical_cast<std::string>(body_.size());

  std::string rtn;
  switch(head_.method_){
    case HM_GET:
      {
        rtn += "GET";
        break;
      }
    case HM_HEAD:
    case HM_POST:
    case HM_PUT:
    case HM_TRACE:
    case HM_CONNECT:
    case HM_DELETE:
    case HM_OPTIONs:
    default:
      //error
      return "";
      break;
  }
  rtn += " "+head_.url_;
  rtn += " "+head_.version+"\r\n";

  for(auto item:head_.attribute_){
    if(item.second !=""){
      rtn += item.first+": "+item.second+"\r\n";
    }
  }
  rtn += "\r\n";
  return rtn;
}

int32_t HttpResponseMsgStruct::FromString(const std::string &str_in){
  auto finded_iter = boost::algorithm::find_first(str_in, "\r\n\r\n");
  if(finded_iter.empty()){
    //http头还没有收完
    return -1;
  }else{
    std::string str_head(str_in.begin(), finded_iter.begin());
    body_.assign(finded_iter.end(), str_in.end());
    boost::cmatch what;
    boost::regex regex("^(.*?)\\s+(.*?)\\s+(.*?)$(.*)$");
    std::cout<<str_head<<std::endl;
    boost::regex_match(str_head.c_str(), what, regex);
    if(what.size() != 5){
      throw "Bad Status line";
      return -1;
    }else{
      //解析状态行
      std::cout<<"???"<<what[1].str()<<std::endl;
      std::cout<<"???"<<what[2].str()<<std::endl;
      std::cout<<"???"<<what[3].str()<<std::endl;
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
      throw "Did not found content-length";
    }
    size_t content_length = boost::lexical_cast<size_t>(head_.attribute_["content-length"]);
    return content_length-body_.size();
  }
}
