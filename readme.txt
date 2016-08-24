这是一个简单的 http 服务器，用 C 语言编写，基于嵌入式 linux
目前支持 html，cgi
支持图片，音乐，视频等多媒体文件

还有很多不足，有待改进

文件说明：
    format     /* 与文件格式有关 */
    include    /* 各个头文件 */
    interface  /* 字符处理接口，相应客户端接口 */
    system     /* 链表操作 */
    big-http.c /* 包含 main 函数 */
    big-http   /* 用 arm-linux-gcc 编译出来的可执行文件 */
    www        /* 测试用的文件 */
    
music.html    /* 媒体文件测试 */
upload.html   /* 上传文件测试 */
script.cgi    /* cgi 脚本文件测试 */
test.cgi      /* 二进制可执行文件测试 */
    
测试：
1. 选定编译工具，PC 机就选择 gcc，开发板就选择 arm-linux-gcc。在根目录的 Makefile 第一行修改
2. make
3. 把 www 目录里面的 cgi 文件重新用对应的编译工具编译一遍
4. 执行 big-http 
5. 在浏览器中输入服务器地址即可看到首页

使用上的不足：
返回给客户端的报文头没有时间
没有 log 数据，只有打印(打印可以关掉，在 config.h 里面更改宏定义)
没有可更改的配置文件，只能在源程序上面改动
