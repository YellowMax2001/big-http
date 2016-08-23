#include <format.h>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

static int isSupportHtml(struct FileDesc *ptFileDesc);
static int HtmlWriteToClient(int iClient, struct FileDesc *ptFileDesc, struct RequestHeader *ptReqHeader);
//static int HtmlReadFromClient(int iClient, char *strPath);

#define SEND_BUFF_LEN 512

struct FormatMgr g_tHtmlFormatMgr = {
    .name = "html",
    .isSupport = isSupportHtml,
    .WriteToClient  = HtmlWriteToClient,
    //.ReadFromClient = HtmlReadFromClient,
};

static int isSupportHtml(struct FileDesc *ptFileDesc)
{
    unsigned char *pucMem = ptFileDesc->pucMem;
    char *pcTmp;

    const char aStrUtf8[]    = {0xef, 0xbb, 0xbf, 0};
    const char aStrUtf16le[] = {0xff, 0xfe, 0};
    const char aStrUtf16be[] = {0xfe, 0xff, 0};

    pcTmp = strrchr((char *)ptFileDesc->strFName, '.');
    DebugPrint("isSupportHtml = %s\n", pcTmp);
    if(pcTmp != NULL){
        if(!strncmp(pcTmp, ".htm", 3)){
            return 1;
        }
    }
    
    if (strncmp((char *)pucMem, aStrUtf8, 3) == 0){
        return 1;
    }else if (strncmp((char *)pucMem, aStrUtf16le, 2) == 0){
        /* UTF-16 little endian */
        return 1;
    }else if (strncmp((char *)pucMem, aStrUtf16be, 2) == 0){
        /* UTF-16 big endian */
        return 1;
    }
	
    return 0;
}

static int HtmlWriteToClient(int iClient, struct FileDesc *ptFileDesc, struct RequestHeader *ptReqHeader)
{
    unsigned char *pucMemStart = ptFileDesc->pucMem;
    unsigned char *pucMem;
    int iSendLen = 0;

    HtmlHeader(iClient);

    DebugPrint("Run to HtmlWriteToClient\n");

    pucMem = pucMemStart;

    while(pucMem < pucMemStart + ptFileDesc->tFStat.st_size){
        iSendLen = send(iClient, pucMem, SEND_BUFF_LEN, 0);
        if(iSendLen < 0){
            perror("RegularFileExec::send");
            return -1;
            break;
        }

        if(iSendLen < SEND_BUFF_LEN){    /* 说明到了文件末尾 */
            break;
        }

        pucMem += SEND_BUFF_LEN;
    }

    return 0;
}

int HtmlInit(void)
{
    RegisterFormatMgr(&g_tHtmlFormatMgr);
    return 0;
}

