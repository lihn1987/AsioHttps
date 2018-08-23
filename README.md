# AsioHttps
使用boost的asio写一个用于http和https通讯的功能库
## 联系我
lihn1011@163.com
## 下一步计划
能够设置超时，若超时没收到数据则调用错误回调函数
多线程测试
## 目前进度
- 2018-08-08能够解析dns同host链接，并且发送简单的http请求
- 2018-08-10已经能够解析http回复
- 2018-08-21已经能够解析https以及连接socket5代理访问google之类的外网
            能够调用回调
- 2018-08-23经过测试，单线程连续访问(代理+https),(代理+http),(代理访问同上一次相同http),(无代理http),(无代理https)


## 项目依赖
boost

## 依赖的安装
- boost的安装
- sudo apt-get install libboost-all-dev
