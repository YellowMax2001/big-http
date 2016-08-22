#include <char.h>
#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>

int GetLineFromSock(int iSockFd, char *pcBuf, int iSize)
{
    int iError = 0;
	char cByte = '\0';    /* 初始化为一个空字符 */
    int iRecvSize = 0;
    struct timeval tWaitVal;
    fd_set ReadSet;
	
    tWaitVal.tv_sec  = 0;
	tWaitVal.tv_usec = 1000 * 1000;    /* 对于每次读取仅仅等待 500 ms */
#if 0
    FD_ZERO(&ReadSet);
    FD_SET(iSockFd, &ReadSet);
	
    iError = select(iSockFd + 1, &ReadSet, NULL, NULL, &tWaitVal);
    if(!FD_ISSET(iSockFd, &ReadSet)){	/* 说明没有数据过来要返回 */
        DebugPrint("Out of receive time ****** \n");
        *pcBuf++ = '\n';	  /* 字符串结束 */
        *pcBuf = '\0';        /* 字符串结束 */
        return -1;
    }
#endif
    while((iRecvSize < iSize) && (cByte != '\n')){

        FD_ZERO(&ReadSet);
        FD_SET(iSockFd, &ReadSet);
        
        iError = select(iSockFd + 1, &ReadSet, NULL, NULL, &tWaitVal);
        if(!FD_ISSET(iSockFd, &ReadSet)){	/* 说明没有数据过来要返回 */
            DebugPrint("Out of receive time ****** \n");
            *pcBuf++ = '\n';	  /* 字符串结束 */
            *pcBuf = '\0';		  /* 字符串结束 */
            return -1;
        }

        iError = recv(iSockFd, &cByte, 1, 0);
        if(-1 == iError){
            perror("GetLineFromSock::recv");
            *pcBuf++ = '\n';	  /* 字符串结束 */
            break;
        }

		/* 结尾是 '\n' 或者 "\r\n" 或者 '\r' */
        if(cByte == '\r'){
            iError = recv(iSockFd, &cByte, 1, MSG_PEEK);
            if((iError > 0) && (cByte == '\n')){
                iError = recv(iSockFd, &cByte, 1, 0);
			}else{
                cByte = '\n';
            }
        }

        *pcBuf++ = cByte;
        iRecvSize ++;
    }

    *pcBuf = '\0';    /* 字符串结束 */

    return iRecvSize;
}

int GetNonSpaceBlock(char pcDes[][256], char *pcSrc, int iLenSize)
{
    int i = 0;
    int j = 0;
	int k = 0;
static char bOutRange = 0;

    if((pcSrc[0] == '\n') || (pcSrc[0] == '\r') || (pcSrc[0] == '\0')){    /* 说明此行为空 */
        pcDes[0][0] = '\0';
        return 0;
    }    

    i = 0;
    j = 0;
    k = 0;

    DebugPrint("%s", pcSrc);

    while(1){
        while(isspace(pcSrc[k])){    /* 吃掉所有的空格字符 */
            if((pcSrc[k] == '\n') || (pcSrc[k] == '\0')){  /* 遇到换行或者空字符,说明字符串解析完毕 */
                return 0;
            }

            k ++;
        }

        while(!isspace(pcSrc[k])){
            if(i < iLenSize){
                pcDes[j][i] = pcSrc[k];
                k ++;
                i ++;
            }else{
                if(0 == bOutRange){
                    DebugPrint("pcSrc out of range\n");
                    bOutRange = 1;
                }
            }
        }

        pcDes[j][i] = '\0';    /* 为每一行加上结束字符 */
        j ++;    /* 二维数组换行 */
		i = 0;   /* i 重新置 0 */
    }

    return 0;
}

int GetRequestHeader(int iSockFd, struct RequestHeader *ptReqHeader)
{
    int iRecvNum = 0;
    char strBuf[1024];
	char strTmp[4][256];
	int iPostArgLen = 0;

    memset(strBuf, 0, sizeof(strBuf));
    memset(strTmp, 0, sizeof(strTmp[0]) * 4);

    /* 首先获取第一行，第一行总是包含了请求方法，url 以及 http 版本号 */
    iRecvNum = GetLineFromSock(iSockFd, strBuf, sizeof(strBuf));
    if(-1 == iRecvNum){
        DebugPrint("GetRequestHeader::GetLineFromSock\n");
        return -1;
    }

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
    }

	if(iPostArgLen){
        iRecvNum = GetLineFromSock(iSockFd, strBuf, 
            iPostArgLen < sizeof(strBuf) ? iPostArgLen : sizeof(strBuf));
        if(-1 == iRecvNum){
            perror("GetRequestHeader::GetLineFromSock");
            return -1;
        }

        DebugPrint("strPostArgs = %s\n", strBuf);
        memcpy(ptReqHeader->strPostArgs, strBuf, iPostArgLen);
    }

    return 0;
}

