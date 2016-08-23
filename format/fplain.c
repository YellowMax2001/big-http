#include <format.h>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

static int isSupportPlain(struct FileDesc *ptFileDesc);
static int PlainWriteToClient(int iClient, struct FileDesc *ptFileDesc, struct RequestHeader *ptReqHeader);
//static int PlainReadFromClient(int iClient, char *strPath);

#define SEND_BUFF_LEN 512

struct FormatMgr g_tPlainFormatMgr = {
    .name = "plain",
    .isSupport = isSupportPlain,
    .WriteToClient  = PlainWriteToClient,
    //.ReadFromClient = PlainReadFromClient,
};

static int isSupportPlain(struct FileDesc *ptFileDesc)
{
    /* 支持任何类型文件 */
    return 1;
}

static int PlainWriteToClient(int iClient, struct FileDesc *ptFileDesc, struct RequestHeader *ptReqHeader)
{
    unsigned char *pucMemStart = ptFileDesc->pucMem;
    unsigned char *pucMem;
    int iSendLen = 0;
    fd_set WriteSet;

    PlainHeader(iClient);

    DebugPrint("Run to PlainWriteToClient\n");

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

int PlainInit(void)
{
    RegisterFormatMgr(&g_tPlainFormatMgr);
    return 0;
}


