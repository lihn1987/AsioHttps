#include <iostream>
#include "AsioHttps.h"
void callback(std::shared_ptr<HttpRequestMsgStruct> request, std::shared_ptr<HttpResponseMsgStruct> response){
  if(response->error_ == ""){
    std::cout<<"get response success!"<<request->head_.GetAttribute("host")<<std::endl;
  }else{
    std::cout<<"get response faild:"<<request->head_.GetAttribute("host")<<response->error_<<std::endl;
  }
}
int main(){
  std::cout<<"hello world!"<<std::endl;
  AsioHttps https(4);
  std::shared_ptr<AsioHttpsSocket> socket1 = https.CreateAsioHttpSocket();
  for(int i = 0; i < 10; i++){
    std::shared_ptr<AsioHttpsRequest> request_ptr =
        std::make_shared<AsioHttpsRequest>();
    socket1->Process("http://china.nba.com", callback);
  }
  std::shared_ptr<AsioHttpsSocket> socket2 = https.CreateAsioHttpSocket();
  for(int i = 0; i < 10; i++){
    std::shared_ptr<AsioHttpsRequest> request_ptr =
        std::make_shared<AsioHttpsRequest>();
    socket2->Process("https://www.github.com", callback);
  }
  getchar();
  exit(0);
  return 0;
}
