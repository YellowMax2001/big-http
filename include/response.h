#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <char.h>

int PutResponseHeader(int iSockFd, struct RequestHeader *ptReqHeader);
void NoSuchFile(int iSockFd);
void bad_request(int iSockFd);
void ErrorExec(int iSockFd);
void HtmlHeader(int iSockFd);
void CgiHeader(int iSockFd);
void PlainHeader(int iSockFd);

#endif    /* response.h */

