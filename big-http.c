#include <format.h>
#include <config.h>
#include <char.h>
#include <response.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

/**********************************************************************
 * 函数名称： CreateServerSocket
 * 功能描述： 为该服务器进程初始化一个 socket
 * 输入参数： 空
 * 输出参数： socket 文件描述符
 * 返 回 值： -1     - 创建失败
 *            0      - 创建成功
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/08/24	     V0.2	      黄泊翰        创建
 ***********************************************************************/
static int CreateServerSocket(void)
{
    int iError = 0;
    int iServerSocket = -1;
    struct sockaddr_in tServerAddr;

    /* ipv4 默认 tcp 协议 */
    iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == iServerSocket){
        perror("CreateServerSocket::socket");
        return -1;
    }

    tServerAddr.sin_family      = AF_INET;    /* 属于IPv4 */
    tServerAddr.sin_port        = htons(SERVER_PORT); /* host to net, short */
    tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* 监控所有的ip INADDR_ANY = 0.0.0.0*/
    memset(tServerAddr.sin_zero, 0, 8);       /* 固定为 0 的字符 */

    iError = bind(iServerSocket, (const struct sockaddr *)&tServerAddr,
                sizeof(struct sockaddr));
    if(iError){
        perror("CreateServerSocket::bind");
		close(iServerSocket);
		return -1;
    }

    iError = listen(iServerSocket, MAX_LISTEN);
    if(iError){
        perror("CreateServerSocket::listen");
		close(iServerSocket);
		return -1;
    }

	return iServerSocket;
}

/**********************************************************************
 * 函数名称： HandleRequest
 * 功能描述： 响应客户端的请求
 * 输入参数： 客户端的 socket 
 * 输出参数： 空
 * 返 回 值： 空
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/08/24	     V0.2	      黄泊翰        创建
 ***********************************************************************/
#if 0
static void *HandleRequest(void *Data)
{
    int iClientSocketFd = *(int *)Data;
    struct RequestHeader tReqHeader;
    int iError = 0;

    memset(&tReqHeader, 0, sizeof(struct RequestHeader));

    DebugPrint("Run to begin of HandleRequest ----\n");

    /* 获取客户端请求头部 */
    iError = GetRequestHeader(iClientSocketFd, &tReqHeader);
    if(iError){
        close(iClientSocketFd);
        pthread_exit(NULL);
    }

    /* 响应请求 */
    PutResponseHeader(iClientSocketFd, &tReqHeader);
    /* 1.cgi */
    /* 2.regular file */

    DebugPrint("Method = %s\n",   tReqHeader.strMethod);
    DebugPrint("URL = %s\n",      tReqHeader.strURL);
    DebugPrint("Version = %s\n",  tReqHeader.strVersion);
    DebugPrint("PostArgs = %s\n", tReqHeader.strPostArgs);
    DebugPrint("GetArgs = %s\n",  tReqHeader.strGetArgs);

    DebugPrint("Run to end of HandleRequest\n");

    close(iClientSocketFd);
    pthread_exit(NULL);
}
#else
static void HandleRequest(int iClientSocketFd)
{
    struct RequestHeader tReqHeader;
    int iError = 0;

    memset(&tReqHeader, 0, sizeof(struct RequestHeader));

    DebugPrint("Run to begin of HandleRequest ----\n");

    /* 获取客户端请求头部 */
    iError = GetRequestHeader(iClientSocketFd, &tReqHeader);
    if(iError){
        close(iClientSocketFd);
        exit(0);
    }

    /* 响应请求 */
    PutResponseHeader(iClientSocketFd, &tReqHeader);
    /* 1.cgi */
    /* 2.regular file */

    DebugPrint("Method = %s\n",   tReqHeader.strMethod);
    DebugPrint("URL = %s\n",      tReqHeader.strURL);
    DebugPrint("Version = %s\n",  tReqHeader.strVersion);
    DebugPrint("PostArgs = %s\n", tReqHeader.strPostArgs);
    DebugPrint("GetArgs = %s\n",  tReqHeader.strGetArgs);

    DebugPrint("Run to end of HandleRequest\n");

    close(iClientSocketFd);
    exit(0);
}
#endif

/**********************************************************************
 * 函数名称： ReadyEnv
 * 功能描述： 为服务器进程准备环境
 * 输入参数： 空
 * 输出参数： 空
 * 返 回 值： -1     - 失败
 *            0      - 成功
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/08/24	     V0.2	      黄泊翰        创建
 ***********************************************************************/
static int ReadyEnv(void)
{
    int iError = 0;

    /* 防止由于子进程先于父进程退出，
     *而父进程没有调用 waitpid/wait 导致僵尸进程 
     *这里忽略 SIGCHLD 信号
     */
    signal(SIGCHLD,SIG_IGN);

	umask(077);    /* new_access = old_access & ~077 */

    iError = chdir(SERVER_ROOT);

    return iError;
}

int main(int argc, char *argv[])
{
    int iError = 0;
    int iServerSocketFd = -1;
    int iClientSocketFd = -1;
    struct sockaddr_in tClientSockaddr;
	socklen_t ClientSockLen = 0;
    pthread_t HandleRequestFd;

    iError = FormatMgrInit();
    if(iError){
        DebugPrint("FormatMgrInit error\n");
        return -1;
    }

    iError = ReadyEnv();
    if(iError){
        DebugPrint("ReadyEnv error\n");
        return -1;
    }

    iServerSocketFd = CreateServerSocket();
	if(-1 == iServerSocketFd){
        perror("main::CreateServerSocket");
		return -1;
    }

    if(fork()){    /* 如果是非子进程，就退出，同时关闭 socket */
        close(iServerSocketFd);
        return 0; 
    }

    while(1){
        iClientSocketFd = accept(iServerSocketFd, (struct sockaddr *)&tClientSockaddr, &ClientSockLen);
        if(-1 == iClientSocketFd){
            perror("main::accept");
			
        }else{
#if 0
            DebugPrint("Got a connect\n");
            iError = pthread_create(&HandleRequestFd, NULL, HandleRequest, &iClientSocketFd);
            if(iError){
                perror("main::pthread_create");
				close(iClientSocketFd);
            }
#else
            if(fork() == 0){
                HandleRequest(iClientSocketFd);
            }else{
                /* 子进程里面关闭，父进程里面也要关闭
                   * 只有全部关闭了，整个客户端才能够被关闭
                   */
                close(iClientSocketFd);
            }
#endif
        }
    }

    close(iServerSocketFd);

    return 0;
}

