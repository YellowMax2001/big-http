#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <common_st.h>
#include <config.h>
#include <response.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct FileDesc
{
    char *strFName;
    int iFd;
    struct stat tFStat;
    unsigned char *pucMem;
};

struct FormatMgr
{
    char *name;
    int (*isSupport)(struct FileDesc *ptFileDesc);
    int (*WriteToClient)(int iClient, struct FileDesc *ptFileDesc);
    int (*ReadFromClient)(int iClient, char *strPath);
    struct list_head tFormatMgr;
};

int RegisterFormatMgr(struct FormatMgr *ptFormatMgr);
void UnregisterFormatMgr(struct FormatMgr *ptFormatMgr);
struct FormatMgr *GetSupportedFormatMgr(struct FileDesc *ptFileDesc);
void ShowFormatMgr(void);
int FormatMgrInit(void);
int RegularFileExec(int iClient, char *strPath, struct RequestHeader *ptReqHeader);
int PlainInit(void);
int HtmlInit(void);
int CgiInit(void);

#endif    /* format.h */

