#ifndef PF_MANAGER_H_H
#define PF_MANAGER_H_H

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <windows.h>

#include "RC.h"

#define PF_PAGE_SIZE ((1<<12)-4)
#define PF_FILESUBHDR_SIZE (sizeof(PF_FileSubHeader))
#define PF_BUFFER_SIZE 50//页面缓冲区的大小
#define PF_PAGESIZE (1<<12)
typedef unsigned int PageNum;

typedef struct{
	PageNum pageNum;
	char pData[PF_PAGE_SIZE];
}Page;                            // 描述一个page上的信息或者一个内存缓冲区槽，会持久化到磁盘

typedef struct{
	PageNum pageCount;
	int nAllocatedPages;
}PF_FileSubHeader;               // 文件的首页紧跟Page.pageNum之后的数据，会持久化到磁盘

typedef struct{
	bool bDirty;
	unsigned int pinCount;
	clock_t  accTime;
	char *fileName;
	int fileDesc;
	Page page;
}Frame;                         // 包含了缓冲区Page的缓冲区槽描述器，仅存在内存中

typedef struct{
	bool bopen;
	char *fileName;
	int fileDesc;
	Frame *pHdrFrame;
	Page *pHdrPage;
	char *pBitmap;
	PF_FileSubHeader *pFileSubHeader;
}PF_FileHandle;                // 文件打开之后便于操作设计的文件句柄

typedef struct{
	int nReads;
	int nWrites;
	Frame frame[PF_BUFFER_SIZE];
	bool allocated[PF_BUFFER_SIZE];
}BF_Manager;                  // 缓冲区管理

typedef struct{
	bool bOpen;
	Frame *pFrame;
}PF_PageHandle;               // 文件的一个页面上的句柄

const RC CreateFile(const char *fileName);
const RC openFile(char *fileName,PF_FileHandle *fileHandle);
const RC CloseFile(PF_FileHandle *fileHandle);

const RC GetThisPage(PF_FileHandle *fileHandle,PageNum pageNum,PF_PageHandle *pageHandle);
const RC AllocatePage(PF_FileHandle *fileHandle,PF_PageHandle *pageHandle);
const RC GetPageNum(PF_PageHandle *pageHandle,PageNum *pageNum);

const RC GetData(PF_PageHandle *pageHandle,char **pData);
const RC DisposePage(PF_FileHandle *fileHandle,PageNum pageNum);

const RC MarkDirty(PF_PageHandle *pageHandle);

const RC UnpinPage(PF_PageHandle *pageHandle);

const RC GetLastPageNum(PF_FileHandle* fileHandle, PageNum* pageNum);

#endif