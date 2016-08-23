#include <format.h>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

DECLARE_HEAD(g_tFormatMgrHead);

int RegisterFormatMgr(struct FormatMgr *ptFormatMgr)
{	
    ListAddTail(&ptFormatMgr->tFormatMgr, &g_tFormatMgrHead);

    return 0;
}

void UnregisterFormatMgr(struct FormatMgr *ptFormatMgr)
{
    if(NULL != ptFormatMgr){
        ListDelTail(&ptFormatMgr->tFormatMgr);
    }
}

void ShowFormatMgr(void)
{	
    int iPTNum = 0;
    struct list_head *ptLHTmpPos;	//LH = lis_head
    struct FormatMgr *ptFMTmpPos;
	
    LIST_FOR_EACH_ENTRY_H(ptLHTmpPos, &g_tFormatMgrHead){
        ptFMTmpPos = LIST_ENTRY(ptLHTmpPos, struct FormatMgr, tFormatMgr);

        printf("%d <---> %s\n", iPTNum++, ptFMTmpPos->name);
    }
}

int FormatMgrInit(void)
{
    int iError = 0;

    iError = CgiInit();
    iError |= HtmlInit();
    iError |= PlainInit();

    if(iError){
        DebugPrint("FormatMgrInit error\n");
        return -1;
    }

	return 0;
}

struct FormatMgr *GetSupportedFormatMgr(struct FileDesc *ptFileDesc)
{
	
    struct list_head *ptLHTmpPos;	//LH = lis_head
    struct FormatMgr *ptFMTmpPos;
	
    LIST_FOR_EACH_ENTRY_H(ptLHTmpPos, &g_tFormatMgrHead){
        ptFMTmpPos = LIST_ENTRY(ptLHTmpPos, struct FormatMgr, tFormatMgr);

        if(ptFMTmpPos->isSupport(ptFileDesc)){
            return ptFMTmpPos;
        }
    }

    return NULL;
}

int RegularFileExec(int iClient, char *strPath, struct RequestHeader *ptReqHeader)
{
    struct stat tFStat;
    int iFd;
	struct FileDesc tFileDesc;
    struct FormatMgr *ptFormatMrg;
    unsigned char *pucMemStart;
    int iError = 0;

    iFd = open(strPath, O_RDWR);
    if(iFd < 0){
        perror("RegularFileExec::open");
        return -1;
    }

    iError = fstat(iFd, &tFStat);
    if(iError){
        close(iFd);
        perror("RegularFileExec::fstat");
        return -1;
    }

    pucMemStart = mmap(0, tFStat.st_size, PROT_READ, MAP_SHARED,
                  iFd, 0);
    if(NULL == pucMemStart){
        close(iFd);
        perror("RegularFileExec::mmap");
        return -1;
    }
#if 0
    pucMem = pucMemStart;

    while(pucMem < pucMemStart + tFStat.st_size){
        iSendLen = send(iClient, pucMem, iLen, 0);
        if(iSendLen < 0){
            perror("RegularFileExec::send");
            break;
        }

        DebugPrint("%d", iSendLen);
        if(iSendLen < iLen){    /* 说明到了文件末尾 */
            break;
        }

        pucMem += iLen;
    }
#endif
    tFileDesc.strFName = strPath;
    tFileDesc.tFStat  = tFStat;
    tFileDesc.iFd      = iFd;
    tFileDesc.pucMem   = pucMemStart;

    ptFormatMrg = GetSupportedFormatMgr(&tFileDesc);
    ptFormatMrg->WriteToClient(iClient, &tFileDesc, ptReqHeader);

    munmap(pucMemStart, tFStat.st_size);
    close(iFd);

    return 0;
}

