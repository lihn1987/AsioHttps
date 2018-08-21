# AsioHttps
使用boost的asio写一个用于http和https通讯的功能库
## 联系我
lihn1011@163.com
## 下一步计划
能够设置超时，若超时没收到数据则调用错误回调函数
测试所有http,https,加代理的http，https
将其封装成lib库
## 目前进度
- 2018-08-08能够解析dns同host链接，并且发送简单的http请求
- 2018-08-10已经能够解析http回复
- 2018-08-21已经能够解析https以及连接socket5代理访问google之类的外网
            能够调用回调


## 项目依赖
boost

## 依赖的安装
- boost的安装
- sudo apt-get install libboost-all-dev
