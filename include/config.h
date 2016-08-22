#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DebugPrint printf

#define SERVER_PORT 80    /* 默认端口是 80 端口，也是 www 的端口 */
#define MAX_LISTEN  256   /* 可监听处理的并发请求数量为 256 */

#define SERVER_NAME  "Server: big-http/0.1-bigyellow"
#define SERVER_ROOT  "./www"
#define DEFAULT_FILE "index.html"

#endif    /* config.h */

