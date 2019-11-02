//
// File:        RM_Manager.h
// Description: A interface for RM File
// Authors:     dim dew
//

#ifndef RM_MANAGER_H_H
#define RM_MANAGER_H_H

#include "PF_Manager.h"
#include "str.h"

#define WHICH_REC(recordSize, records, x) ((records) + ((recordSize) + sizeof(int)) * x + sizeof(int))
#define REC_NEXT_SLOT(recordSize, records, x) ((records) + ((recordSize) + sizeof(int)) * x)

const int RM_NO_MORE_FREE_PAGE = -1;
const int RM_NO_MORE_FREE_SLOT = -2;

typedef int SlotNum;

typedef struct {	
	PageNum pageNum;	       // 记录所在页的页号
	SlotNum slotNum;		   // 记录的插槽号
} RID;

typedef struct{
	bool bValid;		       // False表示还未被读入记录
	RID  rid; 		           // 记录的标识符 
	char *pData; 		       // 记录所存储的数据 
} RM_Record;


typedef struct
{
	int bLhsIsAttr,bRhsIsAttr;//左、右是属性（1）还是值（0）
	AttrType attrType;
	int LattrLength,RattrLength;
	int LattrOffset,RattrOffset;
	CompOp compOp;
	void *Lvalue,*Rvalue;
} Con;

typedef struct {
	int nextFreePage;          // 与 RM_FileHdr 建立一个 freePage 链
	int firstFreeSlot;         // 第 1 个 free slot
	int slotCount;             // 已经分配的 slot 个数 包括删除之后的空 slot
	char slotBitMap[0];        // 标记 slot 是否使用的 bitMap，此处仅做为地址用 (代码中实际分配空间)
} RM_PageHdr;

typedef struct {
	int recordSize;             // 一条记录的大小
	int recordsPerPage;			// 每一页中可以存放的record数量
	int firstFreePage;          // 第一个可以放下记录的page序号 (>= 2)
	int slotsOffset;            // 每个页面中记录槽相对于 PF_Page.pData 的偏移
	// 为了减少 FileHdr 的刷写这里不维护文件中记录的个数
} RM_FileHdr;

typedef struct{                 // 文件句柄
	bool bOpen;
	PF_FileHandle pfFileHandle; // 保存 PF_FileHandle
	RM_FileHdr rmFileHdr;       
	bool isRMHdrDirty;          // 第 1 页是否脏
} RM_FileHandle;

typedef struct{
	bool  bOpen;		//扫描是否打开 
	RM_FileHandle  *pRMFileHandle;		//扫描的记录文件句柄
	int  conNum;		//扫描涉及的条件数量 
	Con  *conditions;	//扫描涉及的条件数组指针
    PF_PageHandle  PageHandle; //处理中的页面句柄
	PageNum  pn; 	//扫描即将处理的页面号
	SlotNum  sn;		//扫描即将处理的插槽号
}RM_FileScan;



RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec);

RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions);

RC CloseScan(RM_FileScan *rmFileScan);

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec);

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid);

RC InsertRec (RM_FileHandle *fileHandle, char *pData, RID *rid); 

RC GetRec (RM_FileHandle *fileHandle, RID *rid, RM_Record *rec); 

RC RM_CloseFile (RM_FileHandle *fileHandle);

RC RM_OpenFile (char *fileName, RM_FileHandle *fileHandle);

RC RM_CreateFile (char *fileName, int recordSize);



#endif