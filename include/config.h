#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DebugPrint printf

#define SERVER_PORT 80    /* 默认端口是 80 端口，也是 www 的端口 */
#define MAX_LISTEN  256   /* 可监听处理的并发请求数量为 256 */
#define SINGLE_UPLOAD_SIZE 1024*1024*2    /* 限制一次上传大小为 2M */

#define SERVER_ROOT  "/www"
#define CONFIG_FILE  "big-http.conf"
#define CGI_ROOT     "/www/cgi-bin"
#define DEFAULT_FILE "index.html"

#define SERVER_SOFTWARE   "Server: big-http/0.1-bigyellow"
#define SERVER_NAME       "www.yellowmax.org"
#define GATEWAY_INTERFACE "CGI/1.1"
#define SERVER_PROTOCOL   "HTTP/1.1"

#endif    /* config.h */

