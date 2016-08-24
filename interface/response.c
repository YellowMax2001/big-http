#include <response.h>
#include <char.h>
#include <config.h>
#include <format.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

void SocketPrintf(int iSockFd, const char *pcData)
{
    char strBuf[1024];

    sprintf(strBuf, "%s\r\n", pcData);
    send(iSockFd, strBuf, strlen(strBuf), 0);
}

void NoSuchFile(int iSockFd)
{	
    SocketPrintf(iSockFd, SERVER_PROTOCOL" 404 NOT FOUND");
    SocketPrintf(iSockFd, SERVER_SOFTWARE);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Not Found</title> \
            <body>                   \
                <h2>Can't find this file</h2> \
            </body>                  \
         </html>                     \
        ");
}

void ErrorExec(int iSockFd)
{
    SocketPrintf(iSockFd, SERVER_PROTOCOL" 500 Internal Server Error");
    SocketPrintf(iSockFd, SERVER_SOFTWARE);
    SocketPrintf(iSockFd, "Content-type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Error exec</title> \
            <body>                   \
                <h2>Error prohibited CGI execution.</h2> \
            </body>                  \
         </html>                     \
        ");
}

void BadGateway(int iSockFd)
{
    SocketPrintf(iSockFd, SERVER_PROTOCOL" 502 Bad Gateway");
    SocketPrintf(iSockFd, SERVER_SOFTWARE);
    SocketPrintf(iSockFd, "Content-type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Bad gateway</title> \
            <body>                   \
                <h2>The CGI was not CGI/1.1 compliant.</h2> \
            </body>                  \
         </html>                     \
        ");
}

void BadRequest(int iSockFd)
{
    SocketPrintf(iSockFd, SERVER_PROTOCOL" 400 BAD REQUEST");
    SocketPrintf(iSockFd, "Content-type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Error request</title> \
            <body>                   \
                <h2>Your browser sent a bad request, such as url is too long.</h2> \
            </body>                  \
         </html>                     \
        ");
}

void HtmlHeader(int iSockFd)
{
    SocketPrintf(iSockFd, SERVER_PROTOCOL" 200 OK");
    SocketPrintf(iSockFd, SERVER_SOFTWARE);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
}

void CgiHeader(int iSockFd)
{
    SocketPrintf(iSockFd, SERVER_PROTOCOL" 200 OK");
    SocketPrintf(iSockFd, SERVER_SOFTWARE);

    /* 在 cgi 程序当中添加报文头的结尾 */
}

void PlainHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_SOFTWARE);
    SocketPrintf(iSockFd, "Content-Type: text/plain\r\n");
}

/**********************************************************************
 * 函数名称： SetEnv
 * 功能描述： 为 CGI 程序创建环境变量
 * 输入参数： struct RequestHeader 请求报文头
 * 输出参数： 空
 * 返 回 值： 空
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/08/24	     V0.2	      黄泊翰        创建
 ***********************************************************************/
static void SetEnv(struct RequestHeader *ptReqHeader)
{
    char strTmp[256];

    /* 方法 */
    setenv("REQUEST_METHOD", ptReqHeader->strMethod, 1);

    /* 参数 */
    if(!strcmp(ptReqHeader->strMethod, "GET")){
        setenv("QUERY_STRING", ptReqHeader->strGetArgs, 1);
    }else if(!strcmp(ptReqHeader->strMethod, "POST")){
        /* 重要，这个参数要给 cgi 程序使用，以获得传进来的参数 */
        setenv("CONTENT_TYPE", ptReqHeader->strContType, 1);

        sprintf(strTmp, "%d", ptReqHeader->iContLen);
        setenv("CONTENT_LENGTH", strTmp, 1);
    }

    setenv("SERVER_PROTOCOL", SERVER_PROTOCOL, 1); 
    setenv("GATEWAY_INTERFACE", GATEWAY_INTERFACE, 1); 
    setenv("SERVER_NAME", SERVER_NAME, 1); 
}

/**********************************************************************
 * 函数名称： GetRequestHeader
 * 功能描述： 获取客户端发来的请求头部
 * 输入参数： iSockFd              客户端 socket 描述符
 *            struct RequestHeader 客户端请求头部
 * 输出参数： 空
 * 返 回 值： -1     - 失败
 *            0      - 成功
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/08/24	     V0.2	      黄泊翰        创建
 ***********************************************************************/
int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader)
{
    int iRecvNum = 0;
    char strBuf[1024];
	char strTmp[4][256];
	int iPostArgLen = 0;
    char *strIndex;

    memset(strBuf, 0, sizeof(strBuf));
    memset(strTmp, 0, sizeof(strTmp[0]) * 4);

    /* 首先获取第一行，第一行总是包含了请求方法，url 以及 http 版本号 */
    iRecvNum = GetLineFromSock(iSockFd, strBuf, sizeof(strBuf));
    if(-1 == iRecvNum){
        DebugPrint("GetRequestHeader::GetLineFromSock\n");
        return -1;
    }
    /* 按空格分割字符 */
    GetNonSpaceBlock(strTmp, strBuf, sizeof(strTmp[0]));

    memcpy(ptReqHeader->strMethod, strTmp[0], sizeof(strTmp[0]));
    memcpy(ptReqHeader->strURL, strTmp[1], sizeof(strTmp[0]));
    memcpy(ptReqHeader->strVersion, strTmp[2], sizeof(strTmp[0]));

    /* 请求头部总是以一个新行作为结束 */
    while(strBuf[0] != '\n'){
        iRecvNum = GetLineFromSock(iSockFd, strBuf, sizeof(strBuf));
        if(-1 == iRecvNum){
            perror("GetRequestHeader::GetLineFromSock");
            return -1;
        }

        if(!strncasecmp("Content-Length:", strBuf, 15)){
            GetNonSpaceBlock(strTmp, strBuf, sizeof(strTmp[0]));
            iPostArgLen = atoi(strTmp[1]);
            DebugPrint("Content-Length = %d\n", iPostArgLen);
        }

        if(!strncasecmp("Content-Type:", strBuf, 13)){
            strIndex = strrchr(strBuf, ':');
            if(strIndex){
                strIndex += 2;   /* 跳过 ": " */
                memcpy(ptReqHeader->strContType, strIndex, sizeof(ptReqHeader->strContType));
            }
            DebugPrint("Content-Type = %s\n", ptReqHeader->strContType);
        }
    }

    if(iPostArgLen > SINGLE_UPLOAD_SIZE){
        BadRequest(iSockFd);
        return -1;
    }
    ptReqHeader->strPostArgs = malloc(iPostArgLen);    /* 分配空间 */
    if(NULL == ptReqHeader->strPostArgs){
        perror("GetRequestHeader::malloc");
        return -1;
    }
    memset(ptReqHeader->strPostArgs, 0, iPostArgLen);  /* 清空 */
    ptReqHeader->iContLen = iPostArgLen;

	if(iPostArgLen){
        iRecvNum = GetBytesFromSock(iSockFd, ptReqHeader->strPostArgs, iPostArgLen);
        if(-1 == iRecvNum){
            perror("GetRequestHeader::GetLineFromSock");
            return -1;
        }
    }

    DebugPrint("strPostArgs = %s \n", ptReqHeader->strPostArgs);

    return 0;
}

/**********************************************************************
 * 函数名称： PutResponseHeader
 * 功能描述： 获取客户端发来的请求头部
 * 输入参数： iSockFd              客户端 socket 描述符
 *            struct RequestHeader 客户端请求头部
 * 输出参数： 空
 * 返 回 值： -1     - 失败
 *            0      - 成功
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/08/24	     V0.2	      黄泊翰        创建
 ***********************************************************************/
int PutResponseHeader(int iSockFd, struct RequestHeader *ptReqHeader)
{
    int iError = 0;
    struct stat tReqFStat;
	char strPath[256]  = "\0";
    char *pcIndex;

    /* 如果有参数的话，把参数取出来 */
    pcIndex = strrchr(ptReqHeader->strURL, '?');
    if(pcIndex != NULL){
        *pcIndex = '\0';
        pcIndex ++;
        memcpy(ptReqHeader->strGetArgs, pcIndex, sizeof(ptReqHeader->strGetArgs));
    }

    snprintf(strPath, sizeof(strPath), SERVER_ROOT"%s", ptReqHeader->strURL);

    iError = stat(strPath, &tReqFStat);
    if(-1 == iError){
        NoSuchFile(iSockFd);
        return 0;
    }

    if ((tReqFStat.st_mode & S_IFMT) == S_IFDIR){  /* 该文件是一个文件夹 */
        strcat(strPath, "/"DEFAULT_FILE);			 /* 默认搜寻下面的 index.html */

		iError = stat(strPath, &tReqFStat);
        if(-1 == iError){
            NoSuchFile(iSockFd);
            return 0;
        }
    }

    DebugPrint("Method = %s \n", ptReqHeader->strMethod);

    SetEnv(ptReqHeader);    /* 设置环境变量 */
    RegularFileExec(iSockFd, strPath, ptReqHeader);

    free(ptReqHeader->strPostArgs);
    ptReqHeader->strPostArgs = NULL;

    DebugPrint("QUERY_STRING = %s \n", getenv("QUERY_STRING"));
    DebugPrint("REQUEST_METHOD = %s \n", getenv("REQUEST_METHOD"));

    return 0;
}

