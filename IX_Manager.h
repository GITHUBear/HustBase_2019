//
// File:       IX_Manager.h
// Desciption: An interface defination of Index File Manager
// Authors:    dim dew
//

#ifndef IX_MANAGER_H_H
#define IX_MANAGER_H_H

#include "RM_Manager.h"
#include "PF_Manager.h"

const int IX_NO_MORE_BUCKET_SLOT = -1;  
const int IX_USELESS_SLOTNUM = -2;      // 内部节点中只需要用 pagenum，以及 DUPLICATE 的叶子节点
const int IX_NULL_CHILD = -3;
const int IX_NO_MORE_NEXT_LEAF = -4;
const int IX_NO_MORE_BUCKET_PAGE = -5;

const char OCCUPIED = 'o';
const char DUP = 'd';

typedef struct{
	int attrLength;
	// int keyLength;
	AttrType attrType;
	PageNum rootPage;
	PageNum first_leaf;
	int order;                    // 一个Node中能够包含的key的数量范围为 [ (order - 1) / 2, order - 1 ]

	// 一些 offset 值的保存，便于在文件中进行定位
	int nodeEntryListOffset;
	int bucketEntryListOffset;

	int nodeKeyListOffset;
	int entrysPerBucket;
} IX_FileHeader;

typedef struct{
	bool bOpen;
	bool isHdrDirty;
	PF_FileHandle fileHandle;
	IX_FileHeader fileHeader;
} IX_IndexHandle;

typedef struct {
	int is_leaf;                   // 节点类型
	int keynum;                    // 目前含有的 keys 的数量
	// PageNum parent;             // 可以有但是没有必要                
	PageNum sibling;               // 叶子节点的兄弟节点
	PageNum firstChild;            // 内部节点的第一个子节点
} IX_NodePageHeader; 

typedef struct {
	SlotNum slotNum;
	SlotNum firstValidSlot;
	SlotNum firstFreeSlot;
	PageNum nextBucket;
} IX_BucketPageHeader;

typedef struct {
	SlotNum nextFreeSlot;
	RID rid;
} IX_BucketEntry;

typedef struct {
	char tag;
	RID rid;
} IX_NodeEntry;

typedef struct{
	bool bOpen;		                               /*扫描是否打开 */
	IX_IndexHandle *pIXIndexHandle;	               //指向索引文件操作的指针
	CompOp compOp;                                 /* 用于比较的操作符*/
	char *value;		                           /* 与属性行比较的值 */
    PF_PageHandle pfPageHandle;   // 固定在缓冲区页面所对应的页面操作列表
	PageNum pnNext; 	                           //下一个将要被读入的页面号
	int ridIx;
} IX_IndexScan;

typedef struct Tree_Node{
	int  keyNum;		//节点中包含的关键字（属性值）个数
	char  **keys;		//节点中包含的关键字（属性值）数组
	Tree_Node  *parent;	//父节点
	Tree_Node  *sibling;	//右边的兄弟节点
	Tree_Node  *firstChild;	//最左边的孩子节点
} Tree_Node; //节点数据结构

typedef struct{
	AttrType  attrType;	//B+树对应属性的数据类型
	int  attrLength;	//B+树对应属性值的长度
	int  order;			//B+树的序数
	Tree_Node  *root;	//B+树的根节点
} Tree;

RC CreateIndex(const char * fileName,AttrType attrType,int attrLength);
RC OpenIndex(const char *fileName,IX_IndexHandle *indexHandle);
RC CloseIndex(IX_IndexHandle *indexHandle);

RC InsertEntry(IX_IndexHandle *indexHandle,void *pData,const RID * rid);
RC DeleteEntry(IX_IndexHandle *indexHandle,void *pData,const RID * rid);
RC SearchEntry(IX_IndexHandle* indexHandle, void* pData, PageNum* pageNum, int* idx);
RC OpenIndexScan(IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value);
RC IX_GetNextEntry(IX_IndexScan *indexScan,RID * rid);
RC CloseIndexScan(IX_IndexScan *indexScan);
// RC GetIndexTree(char *fileName, Tree *index);

RC CreateBucket(IX_IndexHandle *indexHandle, PageNum *pageNum);
RC InsertRIDIntoBucket(IX_IndexHandle* indexHandle, PageNum bucketPageNum, RID rid);
RC DeleteRIDFromBucket(IX_IndexHandle* indexHandle, PageNum bucketPageNum, const RID* rid, PageNum nodePage, RID* nodeRid);

RC splitChild(IX_IndexHandle* indexHandle, PF_PageHandle* parent, int idx, PageNum child);
RC mergeChild(IX_IndexHandle* indexHandle, PF_PageHandle* parent, int lidx, int ridx, PageNum lchild, PageNum rchild);

#endif