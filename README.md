使用说明
==

# 编译

```command line
cmake -G "MinGW Makefiles" -B cmake-build
cd cmake-build
mingw32-make -j8
```
# 运行
```
.\VNet.exe <listening port>
```
# 内置命令

* connect
    - 功能 ：连接另外一个客户端
    - 用法 ：connect <目标IP> <目标端口>
* msg
    - 功能 : 向其他客户端广播一段信息
    - 用法 : msg <一段话>
* enum
    - 功能 ：列出路由表中所有的地址
    - 用法 : enum
* nickname
    - 功能 ：修改本客户端的昵称，可以重名
    - 用法 ：nickname <新用户明>

# 异常

> terminate called after throwing an instance of 'VNet::ControllerStartupException'
>    what():  cannot initialize the controller,reason=bind socket failed,wsa error=10048
> 
> 原因 ：端口被占用

> [!] IO exception occurred : xxxxxxx
>
> 原因 : 底层套接字异常

> terminate called after throwing an instance of 'std::invalid_argument'
> what():  stoi
> 
> 原因 : 输入的端口不是有效的数字