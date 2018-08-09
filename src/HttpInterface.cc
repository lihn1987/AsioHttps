#include "HttpInterface.h"
#include <boost/format.hpp>
std::string HttpMsgStruct::ToString(){
  head_.attribute_["Content-Length"] = (boost::format("%1")%body_.size()).str();

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
    rtn += item.first+":"+item.second+"\r\n";
  }
  rtn += "\r\n";
  return rtn;
}
