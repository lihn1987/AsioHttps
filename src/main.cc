#include <iostream>
#include "AsioHttps.h"
void callback(std::shared_ptr<HttpRequestMsgStruct> request, std::shared_ptr<HttpResponseMsgStruct> response){
  if(response->error_ == ""){
    std::cout<<"get response success!"<<std::endl;
  }else{
    std::cout<<"get response faild:"<<response->error_<<std::endl;
  }
}
int main(){
  std::cout<<"hello world!"<<std::endl;
  AsioHttps https(4);
  std::shared_ptr<AsioHttpsSocket> socket = https.CreateAsioHttpSocket();
  for(int i = 0; i < 1; i++){
    std::shared_ptr<AsioHttpsRequest> request_ptr =
        std::make_shared<AsioHttpsRequest>();
    request_ptr->head_.attribute_["Host"]="github.com";
    socket->Process(request_ptr, callback);
  }
  getchar();
  exit(0);
  return 0;
}
