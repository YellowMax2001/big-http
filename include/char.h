#ifndef __CHAR_H__
#define __CHAR_H__

struct RequestHeader
{
    char strMethod[10];     /* 请求方法 */
	char strURL[256];       /* 请求资源链接 */
	char strVersion[15];    /* http 版本号 */
    char strGetArgs[256];   /* 如果是 get 方式，其请求参数 */
	char strPostArgs[1024]; /* 如果是 post 方式，其请求参数 */
};

/*
struct ResponseHeader
{
    
};
*/

int GetLineFromSock(int iSockFd, char *pcBuf, int iSize);
int GetNonSpaceBlock(char pcDes[][256], char *pcSrc, int iSize);
int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader);

#endif    /* char.h */

