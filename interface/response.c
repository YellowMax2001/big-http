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
    SocketPrintf(iSockFd, "HTTP/1.0 404 NOT FOUND");
    SocketPrintf(iSockFd, SERVER_NAME);
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
    SocketPrintf(iSockFd, "HTTP/1.0 500 Internal Server Error");
    SocketPrintf(iSockFd, SERVER_NAME);
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

void bad_request(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 400 BAD REQUEST");
    SocketPrintf(iSockFd, "Content-type: text/html\r\n");
    SocketPrintf(iSockFd, 
        "<html>                      \
            <title>Error request</title> \
            <body>                   \
                <h2>Your browser sent a bad request.</h2> \
            </body>                  \
         </html>                     \
        ");
}

void HtmlHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
}

void CgiHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/html\r\n");
}

void PlainHeader(int iSockFd)
{
    SocketPrintf(iSockFd, "HTTP/1.0 200 OK");
    SocketPrintf(iSockFd, SERVER_NAME);
    SocketPrintf(iSockFd, "Content-Type: text/plain\r\n");
}

static void SetEnv(struct RequestHeader *ptReqHeader)
{
    char strArgs[1024] = "\0";
    char strMethod[256] = "\0";

    /* 参数 */
    if(!strcmp(ptReqHeader->strMethod, "GET")){
        snprintf(strArgs, 256, "QUERY_STRING=%s", ptReqHeader->strGetArgs);
        strArgs[255] = '\0';
        putenv(strArgs);
    }else if(!strcmp(ptReqHeader->strMethod, "POST")){
        snprintf(strArgs, 1024, "QUERY_STRING=%s", ptReqHeader->strPostArgs);
        strArgs[1023] = '\0';
        putenv(strArgs);
    }

    /* 方法 */
    snprintf(strMethod, 255, "REQUEST_METHOD=%s", ptReqHeader->strMethod);
    strMethod[255] = '\0';
    putenv(strMethod);
}

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

    DebugPrint("QUERY_STRING = %s \n", getenv("QUERY_STRING"));
    DebugPrint("REQUEST_METHOD = %s \n", getenv("REQUEST_METHOD"));

    return 0;
}

