#ifndef __CHAR_H__
#define __CHAR_H__

/*
struct ResponseHeader
{
    
};
*/

int GetLineFromSock(int iSockFd, char *pcBuf, int iSize);
int GetNonSpaceBlock(char pcDes[][256], char *pcSrc, int iSize);
int GetBytesFromSock(int iSockFd, char *pcBuf, int iSize);

#endif    /* char.h */

