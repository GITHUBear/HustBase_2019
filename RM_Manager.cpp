//
// File:        RM_Manger.cpp
// Description: A implementation of RM_Manager.h to handle RM file
// Authors:     dim dew
//

#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"
#include <cassert>


RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//初始化扫描
{

	return SUCCESS;
}

RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{

	return SUCCESS;
}

RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{

	return SUCCESS;
}

RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
	return SUCCESS;
}

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	return SUCCESS;
}

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	return SUCCESS;
}

// 
// 目的: 获取文件句柄 fd, 设置 RM 控制页 (第 1 页)
// 1. 根据 recordSize 计算出每一页可以放置的记录个数
// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
// 5. 初始化一个 RM_FileHdr 结构，写到 pData 指向的内存区
// 6. 标记 第 1 页 为脏
// 7. PF_Manager::CloseFile() 将调用 ForceAllPage
//
RC RM_CreateFile (char *fileName, int recordSize)
{
	int maxRecordsNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	RM_FileHdr* rmFileHdr;
	// 检查 recordSize 是否合法 
	// (由于采用链表的方式所以需要在记录之前保存 nextFreeSlot 字段所以加上 sizeof(int))
	if (recordSize + sizeof(int) > PF_PAGE_SIZE || 
		recordSize < 0)
		return RM_INVALIDRECSIZE;

	// 1. 计算每一页可以放置的记录个数
	maxRecordsNum = ( ( PF_PAGE_SIZE - sizeof(RM_PageHdr) ) << 3 ) / ( (recordSize << 3) + 33 );
	if ((((maxRecordsNum + 7) >> 3) +
		((recordSize + 4) << 3) +
		sizeof(RM_PageHdr)) > PF_PAGE_SIZE)
		maxRecordsNum--;
	maxRecordsNum = ((maxRecordsNum + 7) >> 3) << 3;

	// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
	// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile(fileName, &pfFileHandle)))
		return rc;

	// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
	if ((rc = AllocatePage(&pfFileHandle, &pfPageHandle)))
		return rc;

	assert(pfPageHandle.pFrame->page.pageNum == 1);
	rmFileHdr = (RM_FileHdr*)(pfPageHandle.pFrame->page.pData);

	// 5. 初始化一个 RM_FileHdr 结构，写到 pData 指向的内存区
	rmFileHdr->firstFreePage = RM_NO_MORE_FREE_PAGE;
	rmFileHdr->recordSize = recordSize;
	rmFileHdr->recordsPerPage = maxRecordsNum;
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + (maxRecordsNum >> 3);

	// 6. 标记 第 1 页 为脏
	// 7. PF_Manager::CloseFile() 将调用 ForceAllPage
	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)) || 
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}

// 
// 目的: 打开文件，建立 RM_fileHandle
// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
// 3. 初始化 RM_FileHandle 成员
//
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;

	// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
	// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
	if ((rc = openFile(fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle)))
		return rc;

	// 3. 初始化 RM_FileHandle 成员
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	fileHandle->rmFileHdr = *((RM_FileHdr*)(pfPageHandle.pFrame->page.pData));

	// 4. 释放内存缓冲区
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}


// 
// 目的: 关闭文件，判断 RM_FileHdr 是否被修改
// 如果发生了修改， GetThisPage到内存缓冲区，写入该页, 标记为脏之后
// 调用 PF_Manager::CloseFile() 关闭 RM 文件
//
RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	// 判断是否发生了修改
	if (fileHandle->isRMHdrDirty) {
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 1, &pfPageHandle)))
			return rc;

		*(pfPageHandle.pFrame->page.pData) = *((char*)&(fileHandle->rmFileHdr));
		
		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

	// 调用 PF_Manager::CloseFile() 关闭 RM 文件
	if ((rc = CloseFile(&(fileHandle->pfFileHandle))))
		return rc;

	fileHandle->bOpen = false;
	fileHandle->isRMHdrDirty = false;

	return SUCCESS;
}
