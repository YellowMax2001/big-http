#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <char.h>

struct RequestHeader
{
    char strMethod[10];     /* 请求方法 */
	char strURL[256];       /* 请求资源链接 */
	char strVersion[15];    /* http 版本号 */
    char strContType[256];  /* 请求的 Content-Type */
    char strGetArgs[256];   /* 如果是 get 方式，其请求参数，有长度限制 */
	char *strPostArgs;      /* 如果是 post 方式，其请求参数 */
    int iContLen;           /* 报文体长度 */
};

int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader);
int PutResponseHeader(int iSockFd, struct RequestHeader *ptReqHeader);
void NoSuchFile(int iSockFd);
void bad_request(int iSockFd);
void ErrorExec(int iSockFd);
void HtmlHeader(int iSockFd);
void CgiHeader(int iSockFd);
void PlainHeader(int iSockFd);

#endif    /* response.h */

