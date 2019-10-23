//
// File:        RM_Manger.cpp
// Description: A implementation of RM_Manager.h to handle RM file
// Authors:     dim dew
//

#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"
#include <cassert>
#include <string.h>
#include <iostream>

bool getBit (char *bitMap, int bitIdx)
{
	return ((bitMap[bitIdx >> 3] >> (bitIdx & 0x7)) & 1) == 1;
}

void setBit(char* bitMap, int bitIdx, bool val)
{
	if (val)
		bitMap[bitIdx >> 3] |= (1 << (bitIdx & 0x7));
	else
		bitMap[bitIdx >> 3] &= (~(1 << (bitIdx & 0x7)));
}

int cmp(AttrType attrType, void *ldata, void *rdata, int llen, int rlen)
{
	switch (attrType) 
	{
	case chars:
	{
		char* l = (char*)ldata;
		char* r = (char*)rdata;
		int min_len = MIN(llen, rlen);
		int cmp_res = strncmp(l, r, min_len);
		if (cmp_res == 0)
			cmp_res = (llen > rlen) ? 1 : (llen == rlen) ? 0 : -1;
		return cmp_res;
	}
	case ints:
	{
		int l = *(int*)ldata;
		int r = *(int*)rdata;
		return (l > r) ? 1 : (l == r) ? 0 : -1;
	}
	case floats:
	{
		float l = *(float*)ldata;
		float r = *(float*)rdata;
		return (l > r) ? 1 : (l == r) ? 0 : -1;
	}
	}
}

bool matchComp (int cmp_res, CompOp compOp)
{
	switch (compOp)
	{
	case EQual:
		return cmp_res == 0;
	case LEqual:      
		return cmp_res <= 0;
	case NEqual:    
		return cmp_res != 0;
	case LessT:
		return cmp_res < 0;
	case GEqual:  
		return cmp_res >= 0;
	case GreatT:
		return cmp_res > 0;
	case NO_OP:
		return true;
	}
}

bool innerCmp(Con condition, char *pData)
{
	void* lval, * rval;

	lval = (condition.bLhsIsAttr == 1) ? (pData + condition.LattrOffset) : condition.Lvalue;
	rval = (condition.bRhsIsAttr == 1) ? (pData + condition.RattrOffset) : condition.Rvalue;

	return matchComp(cmp(condition.attrType, lval, rval, 
			condition.LattrLength, condition.RattrLength), condition.compOp);
}

//
// 初始化 RM_FileScan 结构
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//初始化扫描
{
	RC rc;

	if ((rmFileScan->bOpen))
		return RM_FSOPEN;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	rmFileScan->bOpen = true;
	rmFileScan->conditions = conditions;
	rmFileScan->conNum = conNum;
	rmFileScan->pRMFileHandle = fileHandle;
	rmFileScan->pn = 2;
	rmFileScan->sn = 0;

	//if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 2, &(rmFileScan->PageHandle)))) {
	//	if (rc == PF_INVALIDPAGENUM)
	//		return RM_NOMORERECINMEM;
	//	return rc;
	//}

	return SUCCESS;
}

//
// 目的: 
//
RC CloseScan(RM_FileScan* rmFileScan)
{
	if (!rmFileScan->bOpen)
		return RM_FSCLOSED;

	rmFileScan->bOpen = false;
	return SUCCESS;
}

//
// 目的: 
//
RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{
	RC rc;
	PageNum lastPageNum;
	PageNum nextPageNum;
	SlotNum nextSlotNum;
	PF_PageHandle pfPageHandle;
	RM_PageHdr* rmPageHdr;
	char* data;
	char* records;
	int recordSize = rmFileScan->pRMFileHandle->rmFileHdr.recordSize;

	if ((rc = GetLastPageNum(&(rmFileScan->pRMFileHandle->pfFileHandle), &lastPageNum)))
		return rc;

	nextPageNum = rmFileScan->pn;
	nextSlotNum = rmFileScan->sn;

	for (; nextPageNum <= lastPageNum; nextPageNum++) {
		std::cout << "============================== Page " << nextPageNum << "================================" << std::endl;
		if ((rc = GetThisPage(&(rmFileScan->pRMFileHandle->pfFileHandle), nextPageNum, &pfPageHandle)))
		{
			if (rc == PF_INVALIDPAGENUM)
				continue;
			return rc;
		}

		if ((rc = GetData(&pfPageHandle, &data)))
			return rc;

		rmPageHdr = (RM_PageHdr*)data;
		records = data + rmFileScan->pRMFileHandle->rmFileHdr.slotsOffset;

		for (; nextSlotNum < rmPageHdr->slotCount; nextSlotNum++) {
			if (!getBit(rmPageHdr->slotBitMap, nextSlotNum))
				continue;

			int i = 0;
			for (; i < rmFileScan->conNum; i++) {
				if (!innerCmp(rmFileScan->conditions[i],
					WHICH_REC(recordSize, records, nextSlotNum))) {
					break;
				}
			}

			if (i != rmFileScan->conNum)
				continue;

			rec->bValid = true;
			rec->rid.pageNum = nextPageNum;
			rec->rid.slotNum = nextSlotNum;
			memcpy(rec->pData, WHICH_REC(recordSize, records, nextSlotNum), recordSize);

			std::cout << "PageNum: " << nextPageNum << " SlotNum: " << nextSlotNum << std::endl;
			std::cout << "Content: " << rec->pData << std::endl;

			if (nextSlotNum == rmPageHdr->slotCount - 1) {
				rmFileScan->pn = nextPageNum + 1;
				rmFileScan->sn = 0;
			} else {
				rmFileScan->pn = nextPageNum;
				rmFileScan->sn = nextSlotNum + 1;
			}

			if ((rc = UnpinPage(&pfPageHandle)))
				return rc;

			return SUCCESS;
		}

		if ((rc = UnpinPage(&pfPageHandle)))
			return rc;

		nextSlotNum = 0;
	}

	return RM_EOF;
}

//
// 目的: 根据给定的 rid, 获取相应的文件记录保存到 rec 指向的地址
// 1. 通过 rid 获得指定的 pageNum 和 slotNum
// 2. 通过 PF_FileHandle::GetThisPage() 获得 pageNum 页
// 3. 获得该页上的 PageHdr 信息, 定位记录数组起始地址
// 4. 检查该 slotNum 项是否是可用的
// 5. 设置返回的 rec
//
RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	RC rc;
	PF_PageHandle pfPageHandle;
	RM_PageHdr rmPageHdr;
	char *data;
	char *records;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	rec->bValid = false;
	rec->pData = NULL;
	// 1. 通过 rid 获得指定的 pageNum 和 slotNum
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;

	if (pageNum < 2)
		return RM_INVALIDRID;

	// 2. 通过 PF_FileHandle::GetThisPage() 获得 pageNum 页
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. 获得该页上的 PageHdr 信息, 定位记录数组起始地址
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmPageHdr = *((RM_PageHdr*)data);
	records = data + fileHandle->rmFileHdr.slotsOffset;

	// 4. 检查该 slotNum 项是否是可用的
	if (!getBit(rmPageHdr.slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. 设置返回的 rec
	rec->bValid = true;
	rec->pData = records + (fileHandle->rmFileHdr.recordSize + sizeof(int)) * slotNum + sizeof(int);
	rec->rid = *rid;

	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

//
// 目的: 向文件中写入记录项
// 1. 通过 RM_FileHandle 的 RM_FileHdr 中的 firstFreePage 获得第一个可用的 pageNum
// 如果 pageNum != RM_NO_MORE_FREE_PAGE
//    1.1 通过 GetThisPage() 获得该页 pageHdr 的地址
//    1.2 通过 pageHdr 中的 firstFreeSlot  
//    1.3 写入，标记脏，更新 freeSlot 链, 修改bitmap (即更新 pageHdr 的 firstFreeSlot)
//    1.4 如果此时 之前该 slot 位置的 nextSlot 是 RM_NO_MORE_FREE_SLOT
//          1.4.1 那么 FileHandle 的 RM_FileHdr 的 firstFreePage 同样进行修改 维护 FreePage 链
//          1.4.2 维护之后 设置 FileHdr 为脏
// 如果 pageNum == RM_NO_MORE_FREE_PAGE
//    2.1 获得 最后一页 的 pageNum， GetThisPage到内存缓冲区
//    2.2 获得该页 PageHdr 中的 slotNum, 如果 slotNum 尚未达到数量限制，就在文件后面写入新的记录
//    2.3 如果 slotNum 数量达到了 recordsPerPage
//          2.3.1 通过 AllocatePage 向缓冲区申请新的页面
//          2.3.2 初始化 PageHdr
//          2.3.3 写入，更新 bitmap, 更新slotCount，并设置 该页 为脏
//
RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
	RC rc;
	PageNum firstFreePage, lastPageNum;
	SlotNum firstFreeSlot, lastSlotNum;
	PF_PageHandle pfPageHandle, lastPageHandle, newPageHandle;
	RM_PageHdr* rmPageHdr;
	char* data;
	char* records;
	int* nextFreeSlot;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	firstFreePage = fileHandle->rmFileHdr.firstFreePage;

	if (firstFreePage != RM_NO_MORE_FREE_PAGE) {
		//    1.1 通过 GetThisPage() 获得该页 pageHdr 的地址
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), firstFreePage, &pfPageHandle)) || 
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

		//    1.2 通过 pageHdr 中的 firstFreeSlot  
		rmPageHdr = (RM_PageHdr*)data;
		firstFreeSlot = rmPageHdr->firstFreeSlot;

		records = data + fileHandle->rmFileHdr.slotsOffset;

		//    1.3 写入，标记脏，更新 freeSlot 链 (即更新 pageHdr 的 firstFreeSlot)
		memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, firstFreeSlot),
			pData, fileHandle->rmFileHdr.recordSize);
		rmPageHdr->firstFreeSlot = *((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, firstFreeSlot));
		*((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, firstFreeSlot)) = RM_NO_MORE_FREE_SLOT;
		rid->pageNum = firstFreePage;
		rid->slotNum = firstFreeSlot;

		setBit(rmPageHdr->slotBitMap, firstFreeSlot, true);

		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;

		//    1.4 如果此时 之前该 slot 位置的 nextSlot 是 RM_NO_MORE_FREE_SLOT
		if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
			fileHandle->rmFileHdr.firstFreePage = rmPageHdr->nextFreePage;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			fileHandle->isRMHdrDirty = true;
		}
	} else {
		// 如果 pageNum == RM_NO_MORE_FREE_PAGE
		if ((rc = GetLastPageNum(&(fileHandle->pfFileHandle), &lastPageNum)) ||
			(rc = GetThisPage(&(fileHandle->pfFileHandle), lastPageNum, &lastPageHandle)) ||
			(rc = GetData(&lastPageHandle, &data))) {
			return rc;
		}

		// 2.2 获得该页 PageHdr 中的 slotNum, 如果 slotNum 尚未达到数量限制，就在文件后面写入新的记录
		rmPageHdr = (RM_PageHdr*)data;
		lastSlotNum = rmPageHdr->slotCount;
		records = data + fileHandle->rmFileHdr.slotsOffset;

		if (lastPageNum >= 2 && lastSlotNum < fileHandle->rmFileHdr.recordsPerPage) {
			// slotNum 尚未达到数量限制, 就在文件后面写入新的记录
			memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, lastSlotNum),
				pData, fileHandle->rmFileHdr.recordSize);
			setBit(rmPageHdr->slotBitMap, lastSlotNum, true);
			*((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, lastSlotNum)) = RM_NO_MORE_FREE_SLOT;
			rid->pageNum = lastPageNum;
			rid->slotNum = lastSlotNum;

			rmPageHdr->slotCount++;

			if ((rc = MarkDirty(&lastPageHandle)))
				return rc;
		} else {
			//  2.3.1 通过 AllocatePage 向缓冲区申请新的页面
			if ((rc = AllocatePage(&(fileHandle->pfFileHandle), &newPageHandle)) ||
				(rc = GetData(&newPageHandle, &data)))
				return rc;

			rmPageHdr = (RM_PageHdr*)data;
			records = data + fileHandle->rmFileHdr.slotsOffset;

			//  2.3.2 初始化 PageHdr
			rmPageHdr->firstFreeSlot = RM_NO_MORE_FREE_SLOT;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			rmPageHdr->slotCount = 0;

			//  2.3.3 写入，更新 bitmap, 更新slotCount，并设置 该页 为脏
			memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, 0),
				pData, fileHandle->rmFileHdr.recordSize);
			*((int*)REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, 0)) = RM_NO_MORE_FREE_SLOT;
			rid->pageNum = newPageHandle.pFrame->page.pageNum;
			rid->slotNum = 0;

			setBit(rmPageHdr->slotBitMap, 0, true);
			rmPageHdr->slotCount++;

			if ((rc = MarkDirty(&newPageHandle)) ||
				(rc = UnpinPage(&newPageHandle)))
				return rc;
		}

		if ((rc = UnpinPage(&lastPageHandle)))
			return rc;
	}

	return SUCCESS;
}

//
// 目的: 删除指定RID位置的记录
// 1. 通过 GetThisPage 获得 rid 指示的页
// 2. 通过该页的 bitMap 检查 相应的 slotNum 记录是否有效
// 3. 维护 free slot list
// 4. 更新 bitMap
// 5. 如果原来不在 free Page list 上， 将该页加入 free 链
// 
//
RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;
	char* data;
	char* records;
	int* nextFreeSlot;
	RM_PageHdr* rmPageHdr;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	if (pageNum < 2)
		return RM_INVALIDRID;

	// 1. 通过 GetThisPage 获得 rid 指示的页
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

	// 2. 通过该页的 bitMap 检查 相应的 slotNum 记录是否有效
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 3. 维护 free slot list
	nextFreeSlot = (int*)(REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, slotNum));
	*nextFreeSlot = rmPageHdr->firstFreeSlot;

	// 4. 更新 bitMap
	setBit(rmPageHdr->slotBitMap, slotNum, false);

	if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
		// 5. 如果原来不在 free Page list 上， 将该页加入 free 链
		rmPageHdr->nextFreePage = fileHandle->rmFileHdr.firstFreePage;
		fileHandle->rmFileHdr.firstFreePage = pageNum;
		
		fileHandle->isRMHdrDirty = true;
	}

	rmPageHdr->firstFreeSlot = slotNum;

	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

//
// 目的: 更新指定 RID 的内容
//
RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	RC rc;
	RM_Record search;
	PF_PageHandle pfPageHandle;
	PageNum pageNum = rec->rid.pageNum;
	SlotNum slotNum = rec->rid.slotNum;
	RID tmp;

	tmp.pageNum = pageNum;
	tmp.slotNum = slotNum;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	if (pageNum < 2)
		return RM_INVALIDRID;

	if ((rc = GetRec(fileHandle, &tmp, &search)))
		return rc;

	if (!search.bValid)
		return RM_INVALIDRID;

	memcpy(search.pData, rec->pData, fileHandle->rmFileHdr.recordSize);

	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

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
	char* data;
	// 检查 recordSize 是否合法 
	// (由于采用链表的方式所以需要在记录之前保存 nextFreeSlot 字段所以加上 sizeof(int))
	if (recordSize + sizeof(int) > PF_PAGE_SIZE || 
		recordSize < 0)
		return RM_INVALIDRECSIZE;

	// 1. 计算每一页可以放置的记录个数
	maxRecordsNum = ( ( PF_PAGE_SIZE - sizeof(RM_PageHdr) ) << 3 ) / ( (recordSize << 3) + (sizeof(int) << 3) + 1 );
	// std::cout << "PageSize: " << PF_PAGE_SIZE << " sizeof(RM_PageHdr) " << sizeof(RM_PageHdr) << std::endl;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	if ((((maxRecordsNum + 7) >> 3) +
		((recordSize + sizeof(int)) * maxRecordsNum) +
		sizeof(RM_PageHdr)) > PF_PAGE_SIZE)
		maxRecordsNum--;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	// std::cout << "maxBitMapNum: " << ((maxRecordsNum + 7) >> 3) << std::endl;
	// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
	// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile(fileName, &pfFileHandle)))
		return rc;

	// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
	if ((rc = AllocatePage(&pfFileHandle, &pfPageHandle)))
		return rc;

	assert(pfPageHandle.pFrame->page.pageNum == 1);
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmFileHdr = (RM_FileHdr*)data;

	// 5. 初始化一个 RM_FileHdr 结构，写到 pData 指向的内存区
	rmFileHdr->firstFreePage = RM_NO_MORE_FREE_PAGE;
	rmFileHdr->recordSize = recordSize;
	rmFileHdr->recordsPerPage = maxRecordsNum;
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + ((maxRecordsNum + 7) >> 3);

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
	char* data;

	if (fileHandle->bOpen)
		return RM_FHOPENNED;

	// 1. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
	// 2. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
	if ((rc = openFile(fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle))) {
		std::cout << "A" << std::endl;
		return rc;
	}

	// 3. 初始化 RM_FileHandle 成员
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	if ((rc = GetData(&pfPageHandle, &data))) {
		std::cout << "B" << std::endl;
		return rc;
	}
	fileHandle->rmFileHdr = *((RM_FileHdr*)data);

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
	char* data;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	// 判断是否发生了修改
	if (fileHandle->isRMHdrDirty) {
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

		// 需要拷贝
		memcpy(data, (char*) & (fileHandle->rmFileHdr), sizeof(RM_FileHdr));
		
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
