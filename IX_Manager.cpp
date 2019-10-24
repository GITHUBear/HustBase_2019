#include "stdafx.h"
#include "IX_Manager.h"

RC OpenIndexScan (IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value)
{
	return SUCCESS;
}

RC IX_GetNextEntry (IX_IndexScan *indexScan,RID * rid)
{
	return SUCCESS;
}

RC CloseIndexScan (IX_IndexScan *indexScan)
{
	return SUCCESS;
}

//RC GetIndexTree(char *fileName, Tree *index){
//		return SUCCESS;
//}


// 
// 目的: 创建一个 Index File，
//       该 Index File 基于 长度为 attrlength 类型为 attrType 的 key
// 实现非常类似 RM_Manager 中的 CreateFile
// 1. 计算出一个 Page 中能够存放的 key entry 结构的数量，或者说初始化 B+树 的 order
// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
// 4. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 1 页的 pData 指针
// 5. 初始化一个 IX_FileHdr 结构，写到 pData 指向的内存区
// 6. 通过该 PF_FileHandle 的 AllocatePage 方法申请内存缓冲区 拿到第 2 页的 pData 指针
// 7. 初始化一个 IX_NodeHdr 结构，写到 pData 指向的内存区, 初始化一个空的 Root 节点
// 6. 标记 第 1，2 页 为脏
// 7. PF_Manager::CloseFile() 将调用 ForceAllPage
//
RC CreateIndex (const char* fileName, AttrType attrType, int attrLength)
{
	int maxKeysNum, maxBucketEntryNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfHdrPage, pfFirstRootPage;
	IX_FileHeader* ixFileHdr;
	char* fileHdrData, *nodeHdrData;
	IX_FileHeader* ixFileHdr;
	IX_NodePageHeader* ixNodeHdr;

	// B+树 最小 order 需要为 3， split情况下至少要在一个 node 中存下 3条 key记录
	maxKeysNum = (PF_PAGE_SIZE - sizeof(IX_NodePageHeader)) 
					/ (sizeof(char) + sizeof(RID) + attrLength);
	maxBucketEntryNum = (PF_PAGE_SIZE - sizeof(IX_BucketPageHeader)) / (sizeof(RID) + sizeof(SlotNum));

	if (maxKeysNum < 3)
		return IX_INVALIDKEYSIZE;

	// 2. 调用 PF_Manager::CreateFile() 将 Paged File 的相关控制信息进行初始化
	// 3. 调用 PF_Manager::OpenFile() 打开该文件 获取 PF_FileHandle
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile((char*)fileName, &pfFileHandle)))
		return rc;

	if ((rc = AllocatePage(&pfFileHandle, &pfHdrPage)) ||
		(rc = GetData(&pfHdrPage, &fileHdrData)) || 
		(rc = AllocatePage(&pfFileHandle, &pfFirstRootPage)) ||
		(rc = GetData(&pfFirstRootPage, &nodeHdrData)))
		return rc;

	ixFileHdr = (IX_FileHeader*)fileHdrData;
	ixNodeHdr = (IX_NodePageHeader*)nodeHdrData;

	ixFileHdr->attrLength = attrLength;
	ixFileHdr->attrType = attrType;
	ixFileHdr->bucketEntryListOffset = sizeof(IX_BucketPageHeader);
	ixFileHdr->entrysPerBucket = maxBucketEntryNum;
	ixFileHdr->first_leaf = 2;
	ixFileHdr->nodeEntryListOffset = sizeof(IX_NodePageHeader);
	ixFileHdr->nodeKeyListOffset = ixFileHdr->nodeEntryListOffset + (sizeof(char) + sizeof(RID)) * maxKeysNum;
	ixFileHdr->order = maxKeysNum;
	ixFileHdr->rootPage = 2;

	ixNodeHdr->firstChild = IX_NULL_CHILD;
	ixNodeHdr->is_leaf = true;
	ixNodeHdr->keynum = 0;
	ixNodeHdr->sibling = IX_NO_MORE_NEXT_LEAF;

	if ((rc = MarkDirty(&pfHdrPage)) ||
		(rc = MarkDirty(&pfFirstRootPage)) ||
		(rc = UnpinPage(&pfHdrPage)) ||
		(rc = UnpinPage(&pfFirstRootPage)) ||
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}


//
// 目的: 打开已经创建的 Index File，并初始化一个 IX_IndexHandle
// 1. 检查合法性
// 2. 调用 PF_Manager::OpenFile，获得一个 PF_FileHandle
// 3. 通过 PF_FileHandle 来获取1号页面上的 RM_FileHdr 信息
//
RC OpenIndex (const char* fileName, IX_IndexHandle* indexHandle)
{
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	char* data;

	memset(indexHandle, 0, sizeof(IX_IndexHandle));

	if (indexHandle->bOpen)
		return IX_IHOPENNED;

	if ((rc = openFile((char*)fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle)))
		return rc;

	indexHandle->bOpen = true;
	if (rc = GetData(&pfPageHandle, &data))
		return rc;
	indexHandle->fileHeader = *(IX_FileHeader*)data;
	indexHandle->fileHandle = pfFileHandle;
	indexHandle->isHdrDirty = false;

	if (rc = UnpinPage(&pfPageHandle))
		return rc;

	return SUCCESS;
}

// 
// 目的: 关闭文件，将修改的数据刷写到磁盘
// 检查 indexHandle->isHdrDirty 是否为真
// 如果为真，需要GetThisPage获得文件第1页，将indexHandle中维护的IX_FileHeader
// 保存到相应的缓冲区
// 
RC CloseIndex (IX_IndexHandle* indexHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;

	if (!(indexHandle->bOpen))
		return IX_IHCLOSED;

	if (indexHandle->isHdrDirty) {
		if ((rc = GetThisPage(&(indexHandle->fileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

		memcpy(data, (char*) & (indexHandle->fileHeader), sizeof(IX_FileHeader));

		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

	if ((rc = CloseFile(&(indexHandle->fileHandle))))
		return rc;

	indexHandle->bOpen = false;
	indexHandle->isHdrDirty = false;

	return SUCCESS;
}

RC InsertEntryIntoTree(IX_IndexHandle* indexHandle, PageNum node, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle nodePage;
	char* data, *entry, *key;                    // 回溯回来之后不要再用data
	IX_NodePageHeader *nodePageHdr;
	int keyNum;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage)) ||
		(rc = GetData(&nodePage, &data)))
		return rc;

	nodePageHdr = (IX_NodePageHeader*)data;
	keyNum = nodePageHdr->keynum;
	entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if (nodePageHdr->is_leaf) {
		// 当前节点是叶子节点
		// 1. 查找叶子节点中是否存在相同key的slot

	} else {
		// 当前节点是内部节点

	}

	return SUCCESS;
}

// 
// 目的: 
//
RC InsertEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{

	return SUCCESS;
}

RC DeleteEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{
	return SUCCESS;
}