#include <iostream>
#include "AsioHttps.h"
void callback(std::shared_ptr<AsioHttpsRequest>, std::shared_ptr<AsioHttpsResponse>){

}
int main(){
  std::cout<<"hello world!"<<std::endl;
  AsioHttps https(4);
  std::shared_ptr<AsioHttpsSocket> socket = https.CreateAsioHttpSocket();
  std::shared_ptr<AsioHttpsRequest> request_ptr =
      std::make_shared<AsioHttpsRequest>();
  request_ptr->head_.attribute_["Host"]="www.w3school.com.cn";
  socket->Process(request_ptr, callback);
  getchar();
  exit(0);
  return 0;
}
