  
//
// File:        IX_Manager.cpp
// Description: Implementation for IX_Manager
// Authors:     dim dew
//

#include "stdafx.h"
#include "IX_Manager.h"
#include <cassert>
#include <iostream>

int IXComp(IX_IndexHandle *indexHandle, void *ldata, void *rdata, int llen, int rlen)
{
	switch (indexHandle->fileHeader.attrType) {
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

bool innerCmp(IX_IndexScan* indexScan, char *data)
{
	int attrLen = indexScan->pIXIndexHandle->fileHeader.attrLength;
	int cmpres = IXComp(indexScan->pIXIndexHandle, data, indexScan->value, attrLen, attrLen);

	switch (indexScan->compOp)
	{
	case EQual:
		return cmpres == 0;
	case LEqual:
		return cmpres <= 0;
	case NEqual:
		return cmpres != 0;
	case LessT:
		return cmpres < 0;
	case GEqual:
		return cmpres >= 0;
	case GreatT:
		return cmpres > 0;
	case NO_OP:
		return true;
	}
}

bool ridEqual(const RID *lrid, const RID *rrid)
{
	return (lrid->pageNum == rrid->pageNum && lrid->slotNum == rrid->slotNum);
}

RC OpenIndexScan (IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value)
{
	RC rc;

	memset(indexScan, 0, sizeof(IX_IndexScan));

	if (indexScan->bOpen)
		return IX_SCANOPENNED;

	if (!(indexHandle->bOpen))
		return IX_IHCLOSED;

	indexScan->bOpen = true;
	indexScan->compOp = compOp;
	indexScan->pIXIndexHandle = indexHandle;
	indexScan->value = value;

	if ((rc = SearchEntry(indexHandle, indexScan->value, &(indexScan->pnNext), &(indexScan->ridIx))) ||
		(rc = GetThisPage(&(indexHandle->fileHandle), indexScan->pnNext, &(indexScan->pfPageHandle))))
		return rc;

	// 因为重复键值的处理使用的是bucket Page
	// 虽然查找的是最大的小于等于目标值的位置
	// 但是只需要自增1即可移到最小的大于目标值的位置
	switch (indexScan->compOp) {
	case LEqual:
	case NEqual:
	case LessT:
	case NO_OP:
	{
		indexScan->pnNext = indexHandle->fileHeader.first_leaf;
		indexScan->ridIx = 0;
		break;
	}
	case GreatT:
	{
		char* data;
		if (rc = GetData(&(indexScan->pfPageHandle), &data))
			return rc;
		IX_NodePageHeader* nodeHdr = (IX_NodePageHeader*)data;

		indexScan->ridIx++;

		if (indexScan->ridIx >= nodeHdr->keynum) {
			indexScan->pnNext = nodeHdr->sibling;
			indexScan->ridIx = 0;

			if ((rc = UnpinPage(&(indexScan->pfPageHandle))) ||
				(rc = GetThisPage(&(indexHandle->fileHandle), indexScan->pnNext, &(indexScan->pfPageHandle))))
				return rc;
		}
		break;
	}
	default:
		break;
	}

	return SUCCESS;
}

RC IX_GetNextEntry (IX_IndexScan *indexScan,RID * rid)
{
	// 检查当前位置是否满足
	RC rc;
	char* data;

	if (rc = GetData(&(indexScan->pfPageHandle), &data))
		return rc;


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
// 鐩殑: 鍒涘缓涓�涓� Index File锛�
//       璇� Index File 鍩轰簬 闀垮害涓� attrlength 绫诲瀷涓� attrType 鐨� key
// 瀹炵幇闈炲父绫讳技 RM_Manager 涓殑 CreateFile
// 1. 璁＄畻鍑轰竴涓� Page 涓兘澶熷瓨鏀剧殑 key entry 缁撴瀯鐨勬暟閲忥紝鎴栬�呰鍒濆鍖� B+鏍� 鐨� order
// 2. 璋冪敤 PF_Manager::CreateFile() 灏� Paged File 鐨勭浉鍏虫帶鍒朵俊鎭繘琛屽垵濮嬪寲
// 3. 璋冪敤 PF_Manager::OpenFile() 鎵撳紑璇ユ枃浠� 鑾峰彇 PF_FileHandle
// 4. 閫氳繃璇� PF_FileHandle 鐨� AllocatePage 鏂规硶鐢宠鍐呭瓨缂撳啿鍖� 鎷垮埌绗� 1 椤电殑 pData 鎸囬拡
// 5. 鍒濆鍖栦竴涓� IX_FileHdr 缁撴瀯锛屽啓鍒� pData 鎸囧悜鐨勫唴瀛樺尯
// 6. 閫氳繃璇� PF_FileHandle 鐨� AllocatePage 鏂规硶鐢宠鍐呭瓨缂撳啿鍖� 鎷垮埌绗� 2 椤电殑 pData 鎸囬拡
// 7. 鍒濆鍖栦竴涓� IX_NodeHdr 缁撴瀯锛屽啓鍒� pData 鎸囧悜鐨勫唴瀛樺尯, 鍒濆鍖栦竴涓┖鐨� Root 鑺傜偣
// 6. 鏍囪 绗� 1锛�2 椤� 涓鸿剰
// 7. PF_Manager::CloseFile() 灏嗚皟鐢� ForceAllPage
//
RC CreateIndex (const char* fileName, AttrType attrType, int attrLength)
{
	std::cout << "Bucket Entry Size: " << sizeof(IX_BucketEntry) << std::endl;
	std::cout << "Node Entry Size: " << sizeof(IX_NodeEntry) << std::endl;
	int maxKeysNum, maxBucketEntryNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfHdrPage, pfFirstRootPage;
	char* fileHdrData, *nodeHdrData;
	IX_FileHeader* ixFileHdr;
	IX_NodePageHeader* ixNodeHdr;

	// B+鏍� 鏈�灏� order 闇�瑕佷负 3锛� split鎯呭喌涓嬭嚦灏戣鍦ㄤ竴涓� node 涓瓨涓� 3鏉� key璁板綍
	maxKeysNum = (PF_PAGE_SIZE - sizeof(IX_NodePageHeader)) 
					/ (sizeof(IX_NodeEntry) + attrLength);
	maxBucketEntryNum = (PF_PAGE_SIZE - sizeof(IX_BucketPageHeader)) / sizeof(IX_BucketEntry);

	if (maxKeysNum < 3)
		return IX_INVALIDKEYSIZE;

	// 2. 璋冪敤 PF_Manager::CreateFile() 灏� Paged File 鐨勭浉鍏虫帶鍒朵俊鎭繘琛屽垵濮嬪寲
	// 3. 璋冪敤 PF_Manager::OpenFile() 鎵撳紑璇ユ枃浠� 鑾峰彇 PF_FileHandle
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
	ixFileHdr->nodeKeyListOffset = ixFileHdr->nodeEntryListOffset + sizeof(IX_NodeEntry) * maxKeysNum;
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
// 鐩殑: 鎵撳紑宸茬粡鍒涘缓鐨� Index File锛屽苟鍒濆鍖栦竴涓� IX_IndexHandle
// 1. 妫�鏌ュ悎娉曟��
// 2. 璋冪敤 PF_Manager::OpenFile锛岃幏寰椾竴涓� PF_FileHandle
// 3. 閫氳繃 PF_FileHandle 鏉ヨ幏鍙�1鍙烽〉闈笂鐨� RM_FileHdr 淇℃伅
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
// 鐩殑: 鍏抽棴鏂囦欢锛屽皢淇敼鐨勬暟鎹埛鍐欏埌纾佺洏
// 妫�鏌� indexHandle->isHdrDirty 鏄惁涓虹湡
// 濡傛灉涓虹湡锛岄渶瑕丟etThisPage鑾峰緱鏂囦欢绗�1椤碉紝灏唅ndexHandle涓淮鎶ょ殑IX_FileHeader
// 淇濆瓨鍒扮浉搴旂殑缂撳啿鍖�
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

//
// 鐩殑: 鍒濆鍖栦竴涓� Bucket Page锛屽皢鍒濆鍖栧ソ鐨� Bucket Page 鐨� pageNum 杩斿洖
//
RC CreateBucket(IX_IndexHandle* indexHandle, PageNum* pageNum)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;
	IX_BucketEntry *slotRid;
	IX_BucketPageHeader* bucketPageHdr;

	if ((rc = AllocatePage(&(indexHandle->fileHandle), &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	bucketPageHdr = (IX_BucketPageHeader*)data;
	slotRid = (IX_BucketEntry*)(data + indexHandle->fileHeader.bucketEntryListOffset);

	for (int i = 0; i < indexHandle->fileHeader.entrysPerBucket - 1; i++)
		slotRid[i].nextFreeSlot = i + 1;
	slotRid[indexHandle->fileHeader.entrysPerBucket - 1].nextFreeSlot = IX_NO_MORE_BUCKET_SLOT;

	bucketPageHdr->slotNum = 0;
	bucketPageHdr->nextBucket = IX_NO_MORE_BUCKET_PAGE;
	bucketPageHdr->firstFreeSlot = 0;
	bucketPageHdr->firstValidSlot = IX_NO_MORE_BUCKET_SLOT;

	*pageNum = pfPageHandle.pFrame->page.pageNum;

	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

// 
// 鐩殑: 鍚戠粰瀹氱殑 bucket Page(s) 鍐欏叆 RID
// 
RC InsertRIDIntoBucket(IX_IndexHandle* indexHandle, PageNum bucketPageNum, RID rid)
{
	RC rc;
	IX_BucketPageHeader* bucketPageHdr;
	PF_PageHandle bucketPage;
	IX_BucketEntry* entry;
	PageNum lastPageNum = bucketPageNum;
	char* data;
	
	while (bucketPageNum != IX_NO_MORE_BUCKET_PAGE)
	{
		lastPageNum = bucketPageNum;

		if ((rc = GetThisPage(&(indexHandle->fileHandle), bucketPageNum, &bucketPage)) ||
			(rc = GetData(&bucketPage, &data)))
			return rc;

		bucketPageHdr = (IX_BucketPageHeader*)data;
		entry = (IX_BucketEntry*)(data + indexHandle->fileHeader.bucketEntryListOffset);

		if (bucketPageHdr->firstFreeSlot != IX_NO_MORE_BUCKET_SLOT) {
			// 瀛樺湪 free slot
			// 鍐欏叆 RID
			int free_idx = bucketPageHdr->firstFreeSlot;
			int next_free_idx = entry[free_idx].nextFreeSlot;
			entry[free_idx].nextFreeSlot = bucketPageHdr->firstValidSlot;
			entry[free_idx].rid = rid;
			bucketPageHdr->firstValidSlot = free_idx;
			bucketPageHdr->firstFreeSlot = next_free_idx;

			bucketPageHdr->slotNum++;

			// 鏍囪鑴�
			if ((rc = MarkDirty(&bucketPage)) ||
				(rc = UnpinPage(&bucketPage)))
				return rc;

			return SUCCESS;
		}

		bucketPageNum = bucketPageHdr->nextBucket;
	}

	// 閬嶅巻瀹屼簡鍏ㄩ儴鐨� Bucket Page 閮芥病鏈夊瘜瑁曠┖闂翠簡
	// 鍒涘缓涓�涓柊鐨�
	PageNum newPage;
	PF_PageHandle newPageHandle, lastPageHandle;
	char* newData, *lastData;
	IX_BucketPageHeader* newHdr, *lastPageHdr;
	IX_BucketEntry* newEntry;

	if ((rc = CreateBucket(indexHandle, &newPage)) ||
		(rc = GetThisPage(&(indexHandle->fileHandle), newPage, &newPageHandle)) ||
		(rc = GetData(&newPageHandle, &newData)))
		return rc;

	newHdr = (IX_BucketPageHeader*)newData;
	newEntry = (IX_BucketEntry*)(newData + indexHandle->fileHeader.bucketEntryListOffset);

	int free_idx = newHdr->firstFreeSlot;
	int next_free_idx = newEntry[free_idx].nextFreeSlot;
	newEntry[free_idx].nextFreeSlot = newHdr->firstValidSlot;
	newEntry[free_idx].rid = rid;
	newHdr->firstValidSlot = free_idx;
	newHdr->firstFreeSlot = next_free_idx;

	newHdr->slotNum++;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), lastPageNum, &lastPageHandle)) ||
		(rc = GetData(&lastPageHandle, &lastData)))
		return rc;

	lastPageHdr = (IX_BucketPageHeader*)lastData;

	lastPageHdr->nextBucket = newPage;

	if ((rc = MarkDirty(&newPageHandle)) ||
		(rc = MarkDirty(&lastPageHandle)) ||
		(rc = UnpinPage(&newPageHandle)) ||
		(rc = UnpinPage(&lastPageHandle)))
		return rc;

	return SUCCESS;
}

//
// 鐩殑: 鍦� Bucket Page(s) 涓煡鎵惧尮閰嶇殑 rid锛屽苟鍒犻櫎鐩稿簲鐨� rid
//
RC DeleteRIDFromBucket(IX_IndexHandle* indexHandle, PageNum bucketPageNum, const RID* rid, PageNum nodePage, RID *nodeRid)
{
	RC rc;
	PF_PageHandle bucketPageHandle, prePageHandle;
	char* data, * preData;
	IX_BucketPageHeader* bucketPageHdr, * preBucketHdr;
	IX_BucketEntry* entry;
	
	PageNum prePageNum = nodePage;

	while (bucketPageNum != IX_NO_MORE_BUCKET_PAGE) 
	{
		if ((rc = GetThisPage(&(indexHandle->fileHandle), bucketPageNum, &bucketPageHandle)) ||
			(rc = GetData(&bucketPageHandle, &data)))
			return rc;

		bucketPageHdr = (IX_BucketPageHeader*)data;
		entry = (IX_BucketEntry*)(data + indexHandle->fileHeader.bucketEntryListOffset);

		SlotNum validSlot = bucketPageHdr->firstValidSlot;
		SlotNum preValidSlot = IX_NO_MORE_BUCKET_SLOT;

		while (validSlot != IX_NO_MORE_BUCKET_SLOT) 
		{
			SlotNum next_valid = entry[validSlot].nextFreeSlot;

			if (ridEqual(&(entry[validSlot].rid), rid)) {
				if (preValidSlot == IX_NO_MORE_BUCKET_SLOT) {
					bucketPageHdr->firstValidSlot = next_valid;
				} else {
					entry[preValidSlot].nextFreeSlot = next_valid;
				}

				entry[validSlot].nextFreeSlot = bucketPageHdr->firstFreeSlot;
				bucketPageHdr->firstFreeSlot = validSlot;
				bucketPageHdr->slotNum--;

				if (!(bucketPageHdr->slotNum)) {
					// 褰撳墠 bucket 鍒犵┖浜�
					if (prePageNum == nodePage) {
						// 鍓嶄竴涓� Page 鏄� Node Page
						nodeRid->pageNum = bucketPageHdr->nextBucket;
					} else {
						// 鍓嶄竴涓� Page 鏄� Bucket Page
						if ((rc = GetThisPage(&(indexHandle->fileHandle), prePageNum, &prePageHandle)) ||
							(rc = GetData(&prePageHandle, &preData)))
							return rc;

						IX_BucketPageHeader* preBucketPageHdr = (IX_BucketPageHeader*)preData;

						preBucketPageHdr->nextBucket = bucketPageHdr->nextBucket;

						if ((rc = MarkDirty(&prePageHandle)) ||
							(rc = UnpinPage(&prePageHandle)))
							return rc;
					}

					if ((rc = MarkDirty(&bucketPageHandle)) ||
						(rc = UnpinPage(&bucketPageHandle)))
						return rc;

					// dispose 鎺夎繖涓�椤电┖璁板綍鐨� bucket page
					if (rc = DisposePage(&(indexHandle->fileHandle), bucketPageNum))
						return rc;

					return SUCCESS;
				}

				if ((rc = MarkDirty(&bucketPageHandle)) ||
					(rc = UnpinPage(&bucketPageHandle)))
					return rc;

				return SUCCESS;
			}

			preValidSlot = validSlot;
			validSlot = entry[validSlot].nextFreeSlot;
		}

		if (rc = UnpinPage(&bucketPageHandle))
			return rc;

		prePageNum = bucketPageNum;
		bucketPageNum = bucketPageHdr->nextBucket;
	}

	// 閬嶅巻瀹屼簡涔熸病鏈夋壘鍒�
	return IX_DELETE_NO_RID;
}

//
// 鐩殑: 鍦� Node 鏂囦欢椤典腑 鏌ユ壘鎸囧畾 pData
//       鎵惧埌鏈�澶х殑灏忎簬绛変簬 pData 鐨勭储寮曚綅缃繚瀛樺湪 idx 涓�
//       濡傛灉瀛樺湪绛変簬 杩斿洖 true 鍚﹀垯杩斿洖 false
// pData: 鐢ㄦ埛鎸囧畾鐨勬暟鎹寚閽�
// key: Node 鏂囦欢鐨勫叧閿瓧鍖洪鍦板潃鎸囬拡
// s: 鎼滅储鑼冨洿璧峰绱㈠紩
// e: 鎼滅储鑼冨洿缁堟绱㈠紩
// 涓婅堪鑼冨洿涓哄寘鍚�
// 
bool findKeyInNode(IX_IndexHandle* indexHandle, void* pData, void* key, int s, int e, int* idx)
{
	int mid, cmpres;
	int attrLen = indexHandle->fileHeader.attrLength;

	while (s < e) {
		mid = ((s + e) >> 1) + 1;
		cmpres = IXComp(indexHandle, (char*)key + attrLen * mid, pData, attrLen, attrLen);

		if (cmpres < 0) {
			s = mid;
		} else if (cmpres > 0) {
			e = mid - 1;
		} else {
			*idx = mid;
			return true;
		}
	}

	if (s > e || 
		(cmpres = IXComp(indexHandle, (char*)key + attrLen * s, pData, attrLen, attrLen)) > 0) {
		*idx = s - 1;
		return false;
	} else {
		*idx = s;
		return cmpres == 0;
	}
}

// 
// shift 涓嶈礋璐ｈ幏寰楅〉闈紝骞惰缃〉闈㈣剰
// entry: 椤甸潰鎸囬拡鍖洪鍦板潃
// key:   椤甸潰鍏抽敭瀛楀尯棣栧湴鍧�
// idx:   浠巌dx寮�濮嬪埌缁撴潫浣嶇疆 len - 1 杩涜绉诲姩
// dir:   true 琛ㄧず 鍚戝悗绉诲姩; false 鍙嶄箣
//
void shift (IX_IndexHandle* indexHandle, char *entry, char *key, int idx, int len, bool dir) 
{
	int attrLen = indexHandle->fileHeader.attrLength;

	if (dir) {
		for (int i = len - 1; i >= idx; i--) {
			memcpy(key + attrLen * (i + 1), key + attrLen * i, attrLen);
			memcpy(entry + sizeof(IX_NodeEntry) * (i + 1), 
				entry + sizeof(IX_NodeEntry) * i, sizeof(IX_NodeEntry));
		}
	} else {
		for (int i = idx; i < len; i++) {
			memcpy(key + attrLen * (i - 1), key + attrLen * i, attrLen);
			memcpy(entry + sizeof(IX_NodeEntry) * (i - 1),
				entry + sizeof(IX_NodeEntry) * i, sizeof(IX_NodeEntry));
		}
	}
}

//
// splitChild 涓嶈礋璐� parent 椤甸潰鐨勯噴鏀撅紝浣嗘槸璐熻矗瀛愯妭鐐归〉闈㈠拰鏂伴〉闈㈢殑閲婃斁
// 
RC splitChild(IX_IndexHandle* indexHandle, PF_PageHandle* parent, int idx, PageNum child)
{
	RC rc;
	char* data, * par_entry, * par_key;
	char* child_entry, * child_key;
	char* new_entry, * new_key;
	PF_PageHandle childPageHandle, newPageHandle;
	PageNum newPageNum;
	IX_NodePageHeader* parentHdr, * childHdr, * newPageHdr;
	int childKeyNum;
	int attrLen = indexHandle->fileHeader.attrLength;

	if (rc = GetData(parent, &data))
		return rc;

	parentHdr = (IX_NodePageHeader*)data;
	par_entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	par_key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), child, &childPageHandle)) ||
		(rc = GetData(&childPageHandle, &data)))
		return rc;

	childHdr = (IX_NodePageHeader*)data;
	childKeyNum = childHdr->keynum;
	child_entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	child_key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if ((rc = AllocatePage(&(indexHandle->fileHandle), &newPageHandle)) ||
		(rc = GetData(&newPageHandle, &data)))
		return rc;

	newPageHdr = (IX_NodePageHeader*)data;
	newPageNum = newPageHandle.pFrame->page.pageNum;
	new_entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	new_key = data + indexHandle->fileHeader.nodeKeyListOffset;

	IX_NodeEntry* midChildEntry = (IX_NodeEntry*)(child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2));

	if (childHdr->is_leaf) {
		// 瀛愯妭鐐规槸涓�涓彾瀛愯妭鐐�
		// 灏嗗彾瀛愯妭鐐逛粠 childKeyNum / 2 寮�濮嬬殑鎵�鏈変俊鎭嫹璐濆埌鏂拌妭鐐逛綅缃�
		memcpy(new_entry, child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2),
			sizeof(IX_NodeEntry) * (childKeyNum - (childKeyNum / 2)));
		memcpy(new_key, child_key + attrLen * (childKeyNum / 2),
			attrLen * (childKeyNum - (childKeyNum / 2)));

		newPageHdr->firstChild = IX_NULL_CHILD;
		newPageHdr->is_leaf = true;
		newPageHdr->keynum = childKeyNum - (childKeyNum / 2);
		newPageHdr->sibling = childHdr->sibling;
	} else {
		// 瀛愯妭鐐规槸鍐呴儴鑺傜偣
		// 灏嗗彾瀛愯妭鐐逛粠 childKeyNum / 2 + 1 寮�濮嬬殑鎵�鏈変俊鎭嫹璐濆埌鏂拌妭鐐逛綅缃�
		memcpy(new_entry, child_entry + sizeof(IX_NodeEntry) * (childKeyNum / 2 + 1), 
			sizeof(IX_NodeEntry) * (childKeyNum - (childKeyNum / 2) - 1));
		memcpy(new_key, child_key + attrLen * (childKeyNum / 2 + 1),
			attrLen * (childKeyNum - (childKeyNum / 2) - 1));

		newPageHdr->firstChild = midChildEntry->rid.pageNum;
		newPageHdr->is_leaf = false;
		newPageHdr->keynum = childKeyNum - (childKeyNum / 2) - 1;
		newPageHdr->sibling = childHdr->sibling;
	}

	childHdr->keynum = childKeyNum / 2;
	childHdr->sibling = newPageNum;

	// 绉诲姩鐖惰妭鐐规暟鎹�
	shift(indexHandle, par_entry, par_key, idx + 1, parentHdr->keynum, true);
	// 鎶婂瀛愯妭鐐圭殑涓棿鏁版嵁鍐欏叆鍒扮埗鑺傜偣
	IX_NodeEntry* parEntry = (IX_NodeEntry*)(par_entry + sizeof(IX_NodeEntry) * (idx + 1));
	parEntry->tag = OCCUPIED;
	parEntry->rid.pageNum = newPageNum;
	parEntry->rid.slotNum = IX_USELESS_SLOTNUM;
	
	memcpy(par_key + attrLen * (idx + 1), child_key + attrLen * (childKeyNum / 2), attrLen);

	parentHdr->keynum++;

	if ((rc = MarkDirty(&childPageHandle)) ||
		(rc = MarkDirty(&newPageHandle)) ||
		(rc = MarkDirty(parent)) ||
		(rc = UnpinPage(&childPageHandle)) ||
		(rc = UnpinPage(&newPageHandle)))
		return rc;

	return SUCCESS;
}

//
// InsertEntry 鐨勫府鍔╁嚱鏁�
//
RC InsertEntryIntoTree(IX_IndexHandle* indexHandle, PageNum node, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle nodePage;
	char* data, *entry, *key;
	IX_NodePageHeader *nodePageHdr;
	int keyNum;
	int attrLen = indexHandle->fileHeader.attrLength;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage)) ||
		(rc = GetData(&nodePage, &data)))
		return rc;

	nodePageHdr = (IX_NodePageHeader*)data;
	keyNum = nodePageHdr->keynum;
	entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if (nodePageHdr->is_leaf) {
		// 褰撳墠鑺傜偣鏄彾瀛愯妭鐐�
		// 1. 鏌ユ壘鍙跺瓙鑺傜偣涓槸鍚﹀瓨鍦ㄧ浉鍚宬ey鐨剆lot
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		if (findRes) {
			std::cout << "鐩稿悓 key 鎻掑叆" << std::endl;
			// 瀛樺湪璇ey
			// 鑾峰彇瀵瑰簲绱㈠紩浣嶇疆 entry 鍖虹殑淇℃伅
			IX_NodeEntry* pIdxEntry = (IX_NodeEntry*)(entry + sizeof(IX_NodeEntry) * idx);
			// 璇诲彇璇ラ」鐨勬爣蹇椾綅 char tag
			char tag = pIdxEntry->tag;
			RID* thisRid = &(pIdxEntry->rid);
			PageNum bucketPageNum;
			PF_PageHandle bucketPageHandle;

			assert(tag != 0);

			if (tag == OCCUPIED) {
				// 灏氭湭鏍囪涓洪噸澶嶉敭鍊�, 闇�瑕佸垱寤轰竴涓� Bucket File
				// 鑾峰彇鏂板垎閰嶇殑 Bucket Page 鐨� pageHandle,骞舵彃鍏ヨ褰�
				if ((rc = CreateBucket(indexHandle, &bucketPageNum)) || 
					(rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *thisRid)) || 
					(rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *rid)))
					return rc;

				pIdxEntry->tag = DUP;
				thisRid->pageNum = bucketPageNum;
				thisRid->slotNum = IX_USELESS_SLOTNUM;

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return SUCCESS;

			} else if (tag == DUP) {
				// 宸茬粡鏍囪涓洪噸澶嶉敭鍊硷紝鑾峰彇瀵瑰簲鐨� Bucket File锛屾彃鍏ヨ褰�
				bucketPageNum = thisRid->pageNum;
				if (rc = InsertRIDIntoBucket(indexHandle, bucketPageNum, *rid))
					return rc;

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return SUCCESS;
			}
		} else {
			// 涓嶅瓨鍦�
			// idx 涓繚瀛樹簡 <= pData 鐨勭储寮曚綅缃�
			// 闇�瑕佹妸 idx + 1 涔嬪悗鐨� 鎸囬拡鍖� 鍜� 鍏抽敭瀛楀尯 鐨勫唴瀹瑰悜鍚庣Щ鍔�
			// 鍐嶆妸鏁版嵁 RID 鍜� 鍏抽敭瀛� 鍐欏埌 idx + 1 浣嶇疆
			shift(indexHandle, entry, key, idx + 1, keyNum, true);

			IX_NodeEntry* pIdxEntry = (IX_NodeEntry*)(entry + sizeof(IX_NodeEntry) * (idx + 1));

			pIdxEntry->tag = OCCUPIED;
			pIdxEntry->rid = *rid;

			memcpy(key + attrLen * (idx + 1), pData, attrLen);

			int parentKeyNum = ++(nodePageHdr->keynum);

			if ((rc = MarkDirty(&nodePage)) ||
				(rc = UnpinPage(&nodePage)))
				return rc;

			return (parentKeyNum >= indexHandle->fileHeader.order) ? IX_CHILD_NODE_OVERFLOW : SUCCESS;
		}
	} else {
		// 褰撳墠鑺傜偣鏄唴閮ㄨ妭鐐�
		// 鎵惧埌 灏忎簬绛変簬 杈撳叆鍏抽敭瀛楃殑鏈�澶х储寮曚綅缃紝閫掑綊鎵ц InsertEntryIntoTree
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		IX_NodeEntry* nodeEntry = (IX_NodeEntry*)entry;

		PageNum child;
		if (idx == -1)
			child = nodePageHdr->firstChild;
		else
			child = nodeEntry[idx].rid.pageNum;

		// 鍦ㄦ繁鍏� B+鏍� 杩涜閫掑綊涔嬪墠锛岄噴鏀炬帀褰撳墠 page
		if (rc = UnpinPage(&nodePage))
			return rc;

		rc = InsertEntryIntoTree(indexHandle, child, pData, rid);

		if (rc == IX_CHILD_NODE_OVERFLOW) {
			// 瀛愯妭鐐瑰彂鐢熶簡 overflow
			// Get 褰撳墠鑺傜偣鐨� Page
			if (rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage))
				return rc;
			// split 瀛愯妭鐐�
			if (rc = splitChild(indexHandle, &nodePage, idx, child))
				return rc;
			
			if (rc = GetData(&nodePage, &data))
				return rc;

			int parentKeyNum = ((IX_NodePageHeader*)data)->keynum;

			if (rc = UnpinPage(&nodePage))
				return rc;

			return (parentKeyNum >= indexHandle->fileHeader.order) ? IX_CHILD_NODE_OVERFLOW : SUCCESS;
		}

		return rc;
	}

	return SUCCESS;
}

//
// mergeChild 涓嶈礋璐� parent 椤甸潰鐨勯噴鏀撅紝浣嗘槸璐熻矗瀛愯妭鐐归〉闈㈤噴鏀�
// 
RC mergeChild(IX_IndexHandle* indexHandle, PF_PageHandle* parent, 
	          int lidx, int ridx, 
	          PageNum lchild, PageNum rchild)
{
	RC rc;
	char* parentKey, * leftKey, * rightKey;
	PF_PageHandle leftNode, rightNode;
	char* parentData, * leftData, * rightData;
	IX_NodeEntry* parentEntry, * leftEntry, * rightEntry;
	IX_NodePageHeader* parentHdr, * leftHdr, * rightHdr;
	int attrLen = indexHandle->fileHeader.attrLength;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), lchild, &leftNode)) ||
		(rc = GetData(&leftNode, &leftData)) ||
		(rc = GetThisPage(&(indexHandle->fileHandle), rchild, &rightNode)) ||
		(rc = GetData(&rightNode, &rightData)) ||
		(rc = GetData(parent, &parentData)))
		return rc;

	parentHdr = (IX_NodePageHeader*)parentData;
	parentKey = parentData + indexHandle->fileHeader.nodeKeyListOffset;
	parentEntry = (IX_NodeEntry*)(parentData + indexHandle->fileHeader.nodeEntryListOffset);

	leftHdr = (IX_NodePageHeader*)leftData;
	leftKey = leftData + indexHandle->fileHeader.nodeKeyListOffset;
	leftEntry = (IX_NodeEntry*)(leftData + indexHandle->fileHeader.nodeEntryListOffset);

	rightHdr = (IX_NodePageHeader*)rightData;
	rightKey = rightData + indexHandle->fileHeader.nodeKeyListOffset;
	rightEntry = (IX_NodeEntry*)(rightData + indexHandle->fileHeader.nodeEntryListOffset);

	if (!(leftHdr->is_leaf)) {
		// 瀛愯妭鐐规槸鍐呴儴鑺傜偣
		leftEntry[leftHdr->keynum].rid.pageNum = rightHdr->firstChild;
		leftEntry[leftHdr->keynum].rid.slotNum = IX_USELESS_SLOTNUM;
		leftEntry[leftHdr->keynum].tag = OCCUPIED;

		memcpy(leftKey + attrLen * (leftHdr->keynum), parentKey + attrLen * ridx, attrLen);

		leftHdr->keynum++;
	}

	// 鎶� rchild 鐨� 鏁版嵁鍖� 鍜� 鎸囬拡鍖� 鎷疯礉鍒� lchild
	for (int i = 0; i < rightHdr->keynum; i++) {
		leftEntry[leftHdr->keynum].rid = rightEntry[i].rid;
		leftEntry[leftHdr->keynum].tag = rightEntry[i].tag;

		memcpy(leftKey + attrLen * (leftHdr->keynum), rightKey + attrLen * i, attrLen);

		leftHdr->keynum++;
	}

	// 缁存姢 sibling 閾�
	leftHdr->sibling = rightHdr->sibling;

	// 鐖惰妭鐐瑰乏绉�
	shift(indexHandle, (char*)parentEntry, parentKey, ridx + 1, parentHdr->keynum, false);
	parentHdr->keynum--;

	// dispose 鎺� rchild 鑺傜偣
	if ((rc = UnpinPage(&rightNode)) ||
		(rc = DisposePage(&(indexHandle->fileHandle), rchild)))
		return rc;

	if ((rc = MarkDirty(parent)) ||
		(rc = MarkDirty(&leftNode)) ||
		(rc = UnpinPage(&leftNode)))
		return rc;

	return SUCCESS;
}

//
// 鐩殑: DeleteEntry 鐨勫府鍔╁嚱鏁�
//
RC DeleteEntryFromTree (IX_IndexHandle* indexHandle, PageNum node, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle nodePage;
	char* data, * entry, * key;
	IX_NodePageHeader* nodePageHdr;
	int keyNum;
	int attrLen = indexHandle->fileHeader.attrLength;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage)) ||
		(rc = GetData(&nodePage, &data)))
		return rc;

	nodePageHdr = (IX_NodePageHeader*)data;
	keyNum = nodePageHdr->keynum;
	entry = data + indexHandle->fileHeader.nodeEntryListOffset;
	key = data + indexHandle->fileHeader.nodeKeyListOffset;

	if (nodePageHdr->is_leaf) {
		// 褰撳墠鑺傜偣鏄彾瀛愯妭鐐�
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		if (findRes) {
			// key 鍊煎湪 B+ 鏍戜腑瀛樺湪
			// 鑾峰彇瀵瑰簲绱㈠紩浣嶇疆 entry 鍖虹殑淇℃伅
			IX_NodeEntry* pIdxEntry = (IX_NodeEntry*)(entry + sizeof(IX_NodeEntry) * idx);
			// 璇诲彇璇ラ」鐨勬爣蹇椾綅 char tag
			char tag = pIdxEntry->tag;
			RID* thisRid = &(pIdxEntry->rid);

			assert(tag != 0);

			if (tag == OCCUPIED) {
				// 涓嶅瓨鍦� Bucket Page
				// 鍒ゆ柇 RID 鏄惁鍖归厤
				if (!ridEqual(thisRid, rid))
					return IX_DELETE_NO_RID;
				// 鍒犳帀 idx 浣嶇疆鐨� 鎸囬拡 鍜� 鍏抽敭瀛�
				shift(indexHandle, entry, key, idx + 1, keyNum, false);

				int curKeyNum = (--(nodePageHdr->keynum));

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return (curKeyNum < (indexHandle->fileHeader.order - 1) / 2) ? IX_CHILD_NODE_UNDERFLOW : SUCCESS;

			} else if (tag == DUP) {
				// 瀛樺湪 Bucket Page
				if (rc = DeleteRIDFromBucket(indexHandle, thisRid->pageNum, rid, node, thisRid))
					return rc;

				if (thisRid->pageNum == IX_NO_MORE_BUCKET_PAGE) {
					// 鍙戠幇 Bucket 宸茬粡琚垹瀹�
					// 鍒犳帀 idx 浣嶇疆鐨� 鎸囬拡 鍜� 鍏抽敭瀛�
					shift(indexHandle, entry, key, idx + 1, keyNum, false);

					int curKeyNum = (--(nodePageHdr->keynum));

					if ((rc = MarkDirty(&nodePage)) ||
						(rc = UnpinPage(&nodePage)))
						return rc;

					return (curKeyNum < (indexHandle->fileHeader.order - 1) / 2) ? IX_CHILD_NODE_UNDERFLOW : SUCCESS;
				}

				if ((rc = MarkDirty(&nodePage)) ||
					(rc = UnpinPage(&nodePage)))
					return rc;

				return SUCCESS;
			}
		}

		// key 鍦� B+ 鏍� 涓笉瀛樺湪锛岄偅涔堢洿鎺ヨ繑鍥為敊璇�
		return IX_DELETE_NO_KEY;
	} else {
		// 褰撳墠鑺傜偣鏄唴閮ㄨ妭鐐�
		// 鎵惧埌 灏忎簬绛変簬 杈撳叆鍏抽敭瀛楃殑鏈�澶х储寮曚綅缃紝閫掑綊鎵ц DeleteEntryFromTree
		int idx;
		bool findRes = findKeyInNode(indexHandle, pData, key, 0, keyNum - 1, &idx);
		IX_NodeEntry* nodeEntry = (IX_NodeEntry*)entry;

		PageNum child, siblingChild;
		PF_PageHandle siblingChildPageHandle, childPageHandle;
		char* siblingData, * siblingKey, *childData, *childKey, *nodeKey;
		IX_NodePageHeader* siblingHdr, *childHdr;
		IX_NodeEntry* siblingEntry, *childEntry;

		if (idx == -1)
			child = nodePageHdr->firstChild;
		else
			child = nodeEntry[idx].rid.pageNum;

		// 鍦ㄦ繁鍏� B+鏍� 杩涜閫掑綊涔嬪墠锛岄噴鏀炬帀褰撳墠 page
		if (rc = UnpinPage(&nodePage))
			return rc;

		rc = DeleteEntryFromTree(indexHandle, child, pData, rid);

		if (rc == IX_CHILD_NODE_UNDERFLOW) {
			// 瀛愯妭鐐瑰彂鐢熶簡 underflow
			// Get 褰撳墠鑺傜偣鐨� Page
			if ((rc = GetThisPage(&(indexHandle->fileHandle), node, &nodePage)) ||
				(rc = GetData(&nodePage, &data)))
				return rc;

			nodePageHdr = (IX_NodePageHeader*)data;
			nodeEntry = (IX_NodeEntry*)(data + indexHandle->fileHeader.nodeEntryListOffset);
			nodeKey = data + indexHandle->fileHeader.nodeKeyListOffset;

			// 瀛愯妭鐐瑰湪鐖惰妭鐐圭殑 idx 澶�
			if (idx >= 0) {
				// 瀛愯妭鐐瑰瓨鍦ㄥ乏鍏勫紵 idx - 1
				siblingChild = (idx - 1 == -1) ? (nodePageHdr->firstChild) : (nodeEntry[idx - 1].rid.pageNum);

				if ((rc = GetThisPage(&(indexHandle->fileHandle), siblingChild, &siblingChildPageHandle)) ||
					(rc = GetData(&siblingChildPageHandle, &siblingData)) ||
					(rc = GetThisPage(&(indexHandle->fileHandle), child, &childPageHandle)) ||
					(rc = GetData(&childPageHandle, &childData)))
					return rc;

				siblingHdr = (IX_NodePageHeader*)siblingData;
				siblingEntry = (IX_NodeEntry*)(siblingData + indexHandle->fileHeader.nodeEntryListOffset);
				siblingKey = siblingData + indexHandle->fileHeader.nodeKeyListOffset;

				childHdr = (IX_NodePageHeader*)childData;
				childEntry = (IX_NodeEntry*)(childData + indexHandle->fileHeader.nodeEntryListOffset);
				childKey = childData + indexHandle->fileHeader.nodeKeyListOffset;

				if (siblingHdr->keynum > (indexHandle->fileHeader.order - 1) / 2) {
					// 宸﹀厔寮熸湁瀵屼綑鑺傜偣
					// 灏� child 鑺傜偣鏁版嵁浠� 绱㈠紩 0 寮�濮� 鍚戝悗绉诲姩
					shift(indexHandle, (char*)childEntry, childKey, 0, childHdr->keynum, true);
					childHdr->keynum++;

					if (childHdr->is_leaf) {
						// 瀛愯妭鐐规槸鍙跺瓙鑺傜偣
						// 鎶� siblingChild 鐨勬渶鍚庝竴涓� 鏁版嵁妲� 杞Щ鍒� 瀛愯妭鐐圭殑绗� 0 涓綅缃�
						int siblingLast = siblingHdr->keynum - 1;
						childEntry[0].rid = siblingEntry[siblingLast].rid;
						childEntry[0].tag = siblingEntry[siblingLast].tag;

						memcpy(childKey, siblingKey + attrLen * siblingLast, attrLen);

						siblingHdr->keynum--;
						// 鎶婃柊鐨勪腑闂� key 鍊煎啓鍏ュ埌鐖惰妭鐐� idx 浣嶇疆
						memcpy(nodeKey + attrLen * idx, childKey, attrLen);
					} else {
						// 瀛愯妭鐐规槸鍐呴儴鑺傜偣
						int siblingLast = siblingHdr->keynum - 1;
						memcpy(childKey, nodeKey + attrLen * idx, attrLen);
						childEntry[0].rid.pageNum = childHdr->firstChild;
						childEntry[0].rid.slotNum = IX_USELESS_SLOTNUM;
						childEntry[0].tag = OCCUPIED;

						childHdr->firstChild = siblingEntry[siblingLast].rid.pageNum;

						memcpy(nodeKey + attrLen * idx, siblingKey + attrLen * siblingLast, attrLen);

						siblingHdr->keynum--;
					}

					if ((rc = MarkDirty(&nodePage)) || 
						(rc = MarkDirty(&siblingChildPageHandle)) ||
						(rc = MarkDirty(&childPageHandle)) ||
						(rc = UnpinPage(&nodePage)) ||
						(rc = UnpinPage(&siblingChildPageHandle)) ||
						(rc = UnpinPage(&childPageHandle)))
						return rc;

					return SUCCESS;
				}

				if ((rc = UnpinPage(&siblingChildPageHandle)) ||
					(rc = UnpinPage(&childPageHandle)))
					return rc;
			} 
			
			if (idx + 1 < nodePageHdr->keynum) {
				// 瀛愯妭鐐瑰瓨鍦ㄥ彸鍏勫紵 idx + 1
				siblingChild = nodeEntry[idx + 1].rid.pageNum;

				if ((rc = GetThisPage(&(indexHandle->fileHandle), siblingChild, &siblingChildPageHandle)) ||
					(rc = GetData(&siblingChildPageHandle, &siblingData)) ||
					(rc = GetThisPage(&(indexHandle->fileHandle), child, &childPageHandle)) ||
					(rc = GetData(&childPageHandle, &childData)))
					return rc;

				siblingHdr = (IX_NodePageHeader*)siblingData;
				siblingEntry = (IX_NodeEntry*)(siblingData + indexHandle->fileHeader.nodeEntryListOffset);
				siblingKey = siblingData + indexHandle->fileHeader.nodeKeyListOffset;

				childHdr = (IX_NodePageHeader*)childData;
				childEntry = (IX_NodeEntry*)(childData + indexHandle->fileHeader.nodeEntryListOffset);
				childKey = childData + indexHandle->fileHeader.nodeKeyListOffset;

				if (siblingHdr->keynum > (indexHandle->fileHeader.order - 1) / 2) {
					// 鍙冲厔寮熸湁瀵屼綑鑺傜偣
					if (childHdr->is_leaf) {
						// 褰撳墠鑺傜偣鏄彾瀛愯妭鐐�
						// 灏� 鍙宠妭鐐� 鐨勭0涓暟鎹啓鍏ュ埌瀛愯妭鐐圭殑鏈�鍚�
						int childLast = childHdr->keynum;
						childEntry[childLast].rid = siblingEntry[0].rid;
						childEntry[childLast].tag = siblingEntry[0].tag;

						memcpy(childKey + attrLen * childLast, siblingKey, attrLen);

						shift(indexHandle, (char*)siblingEntry, siblingKey, 1, siblingHdr->keynum, false);

						memcpy(nodeKey + attrLen * (idx + 1), siblingKey, attrLen);

						childHdr->keynum++;
						siblingHdr->keynum--;
					} else {
						// 褰撳墠鑺傜偣鏄唴閮ㄨ妭鐐�
						int childLast = childHdr->keynum;
						memcpy(childKey + attrLen * childLast, nodeKey + attrLen * (idx + 1), attrLen);
						childEntry[childLast].rid.pageNum = siblingHdr->firstChild;
						childEntry[childLast].rid.slotNum = IX_USELESS_SLOTNUM;
						childEntry[childLast].tag = OCCUPIED;

						memcpy(nodeKey + attrLen * (idx + 1), siblingKey, attrLen);

						siblingHdr->firstChild = siblingEntry[0].rid.pageNum;

						shift(indexHandle, (char*)siblingEntry, siblingKey, 1, siblingHdr->keynum, false);

						childHdr->keynum++;
						siblingHdr->keynum--;
					}

					if ((rc = MarkDirty(&nodePage)) ||
						(rc = MarkDirty(&siblingChildPageHandle)) ||
						(rc = MarkDirty(&childPageHandle)) ||
						(rc = UnpinPage(&nodePage)) ||
						(rc = UnpinPage(&siblingChildPageHandle)) ||
						(rc = UnpinPage(&childPageHandle)))
						return rc;

					return SUCCESS;
				}

				if ((rc = UnpinPage(&siblingChildPageHandle)) ||
					(rc = UnpinPage(&childPageHandle)))
					return rc;
			}
			
			// 涓よ竟閮戒笉鑳芥彁渚�1涓暟鎹Ы
			if (idx >= 0) {
				// 宸﹀厔寮熷瓨鍦�
				siblingChild = (idx - 1 == -1) ? (nodePageHdr->firstChild) : (nodeEntry[idx - 1].rid.pageNum);
				if (rc = mergeChild(indexHandle, &nodePage,
					idx - 1, idx,
					siblingChild, child))
					return rc;

				int curKeyNum = nodePageHdr->keynum;

				if (rc = UnpinPage(&nodePage))
					return rc;

				return (curKeyNum < (indexHandle->fileHeader.order - 1) / 2) ? IX_CHILD_NODE_UNDERFLOW : SUCCESS;
			}

			if (idx + 1 < nodePageHdr->keynum) {
				// 鍙冲厔寮熷瓨鍦�
				siblingChild = nodeEntry[idx + 1].rid.pageNum;
				if (rc = mergeChild(indexHandle, &nodePage,
					idx, idx + 1,
					child, siblingChild))
					return rc;

				int curKeyNum = nodePageHdr->keynum;

				if (rc = UnpinPage(&nodePage))
					return rc;

				return (curKeyNum < (indexHandle->fileHeader.order - 1) / 2) ? IX_CHILD_NODE_UNDERFLOW : SUCCESS;
			}
		}
		return rc;
	}

	return SUCCESS;
}

// 
// 鐩殑: 鍚� index file 涓彃鍏ユ柊鐨� key - rid 瀵�
//
RC InsertEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle newRootHandle;
	char* data;
	IX_NodePageHeader* newRootHdr;

	rc = InsertEntryIntoTree(indexHandle, indexHandle->fileHeader.rootPage, pData, rid);

	if (rc == IX_CHILD_NODE_OVERFLOW) {
		// 鏍硅妭鐐规弧浜�
		// 鍒涘缓涓�涓柊鐨� page 浣滀负鏂扮殑鏍硅妭鐐�
		if ((rc = AllocatePage(&(indexHandle->fileHandle), &newRootHandle)) || 
			(rc = GetData(&newRootHandle, &data)))
			return rc;

		newRootHdr = (IX_NodePageHeader*)data;
		newRootHdr->firstChild = indexHandle->fileHeader.rootPage;
		newRootHdr->is_leaf = false;
		newRootHdr->keynum = 0;
		newRootHdr->sibling = IX_NO_MORE_NEXT_LEAF;

		if (rc = splitChild(indexHandle, &newRootHandle, -1, newRootHdr->firstChild))
			return rc;

		indexHandle->fileHeader.rootPage = newRootHandle.pFrame->page.pageNum;
		indexHandle->isHdrDirty = true;

		if (rc = UnpinPage(&newRootHandle))
			return rc;

		return SUCCESS;
	}

	return rc;
}

RC DeleteEntry (IX_IndexHandle* indexHandle, void* pData, const RID* rid)
{
	RC rc;
	PF_PageHandle rootPage;
	char* data;
	IX_NodePageHeader* rootPageHdr;

	rc = DeleteEntryFromTree(indexHandle, indexHandle->fileHeader.rootPage, pData, rid);

	if (rc == IX_CHILD_NODE_UNDERFLOW) {
		// 鍒犻櫎鐨勬儏鍐典笅 鍗虫椂鏍硅妭鐐硅繑鍥炰簡 IX_CHILD_NODE_UNDERFLOW 涔熸棤濡�
		// 浣嗘槸闇�瑕佹鏌� 鏍硅妭鐐� 鏄惁琚垹绌�
		if ((rc = GetThisPage(&(indexHandle->fileHandle), indexHandle->fileHeader.rootPage, &rootPage)) ||
			(rc = GetData(&rootPage, &data)))
			return rc;

		rootPageHdr = (IX_NodePageHeader*)data;

		if (!(rootPageHdr->keynum)) {
			// 鏍硅妭鐐硅鍒犵┖浜�
			// 鏇存柊 index 鏂囦欢鐨勬牴鑺傜偣
			if ((rc = UnpinPage(&rootPage)) ||
				(rc = DisposePage(&(indexHandle->fileHandle), indexHandle->fileHeader.rootPage)))
				return rc;

			indexHandle->fileHeader.rootPage = rootPageHdr->firstChild;
			indexHandle->isHdrDirty = true;
			return SUCCESS;
		}

		if (rc = UnpinPage(&rootPage))
			return rc;

		return SUCCESS;
	}

	return rc;
}

RC SearchEntryInTree(IX_IndexHandle* indexHandle, PageNum node, void* pData, PageNum* pageNum, int* idx)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data, *key;
	IX_NodePageHeader* nodePageHdr;
	IX_NodeEntry* nodeEntry;
	int findIdx;

	if ((rc = GetThisPage(&(indexHandle->fileHandle), node, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	nodePageHdr = (IX_NodePageHeader*)data;
	key = data + (indexHandle->fileHeader.nodeKeyListOffset);
	nodeEntry = (IX_NodeEntry*)(data + indexHandle->fileHeader.nodeEntryListOffset);

	if (nodePageHdr->is_leaf) {
		// 当前节点是叶子节点
		*pageNum = node;
		*idx = findIdx;
		return SUCCESS;
	} else {
		PageNum child;
		findKeyInNode(indexHandle, pData, key, 0, nodePageHdr->keynum - 1, &findIdx);

		if (findIdx == -1)
			child = nodePageHdr->firstChild;
		else
			child = nodeEntry[findIdx].rid.pageNum;

		if (rc = SearchEntryInTree(indexHandle, child, pData, pageNum, idx))
			return rc;

		return SUCCESS;
	}
}

RC SearchEntry(IX_IndexHandle* indexHandle, void* pData, PageNum* pageNum, int* idx)
{
	return SearchEntryInTree(indexHandle, indexHandle->fileHeader.rootPage, pData, pageNum, idx);
}