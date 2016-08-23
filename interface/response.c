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
    char strContType[256] = "\0";

    /* 参数 */
    if(!strcmp(ptReqHeader->strMethod, "GET")){
        snprintf(strArgs, 256, "QUERY_STRING=%s", ptReqHeader->strGetArgs);
        strArgs[255] = '\0';
        putenv(strArgs);
    }else if(!strcmp(ptReqHeader->strMethod, "POST")){
        /* 重要，这个参数要给 cgi 程序使用，以获得传进来的参数 */
        snprintf(strContType, 256, "CONTENT_TYPE=%s", ptReqHeader->strContType);
        strContType[255] = '\0';
        putenv(strContType);

        snprintf(strArgs, 256, "CONTENT_LENGTH=%d", ptReqHeader->iContLen);
        strArgs[255] = '\0';
        putenv(strArgs);
    }

    /* 方法 */
    snprintf(strMethod, 255, "REQUEST_METHOD=%s", ptReqHeader->strMethod);
    strMethod[255] = '\0';
    putenv(strMethod);
}

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

