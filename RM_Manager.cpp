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
<<<<<<< HEAD
// ³õÊ¼»¯ RM_FileScan ½á¹¹
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//³õÊ¼»¯É¨Ãè
=======
// åˆå§‹åŒ– RM_FileScan ç»“æ„
//
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//åˆå§‹åŒ–æ‰«æ
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
{
	RC rc;

	memset(rmFileScan, 0, sizeof(RM_FileScan));

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
<<<<<<< HEAD
// Ä¿µÄ: 
=======
// ç›®çš„: 
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC CloseScan(RM_FileScan* rmFileScan)
{
	if (!rmFileScan->bOpen)
		return RM_FSCLOSED;

	rmFileScan->bOpen = false;
	return SUCCESS;
}

//
<<<<<<< HEAD
// Ä¿µÄ: RM_FileScan µü´úÆ÷
=======
// ç›®çš„: RM_FileScan è¿­ä»£å™¨
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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

	assert(rec->pData);

	if ((rc = GetLastPageNum(&(rmFileScan->pRMFileHandle->pfFileHandle), &lastPageNum)))
		return rc;

	nextPageNum = rmFileScan->pn;
	nextSlotNum = rmFileScan->sn;

	for (; nextPageNum <= lastPageNum; nextPageNum++) {
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
				std::cout << "============================== Page " << nextPageNum << " end ================================" << std::endl;
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

		std::cout << "============================== Page " << nextPageNum << " end ================================" << std::endl;
		nextSlotNum = 0;
	}

	return RM_EOF;
}

//
<<<<<<< HEAD
// Ä¿µÄ: ¸ù¾İ¸ø¶¨µÄ rid, »ñÈ¡ÏàÓ¦µÄÎÄ¼ş¼ÇÂ¼±£´æµ½ rec Ö¸ÏòµÄµØÖ·
// 1. Í¨¹ı rid »ñµÃÖ¸¶¨µÄ pageNum ºÍ slotNum
// 2. Í¨¹ı PF_FileHandle::GetThisPage() »ñµÃ pageNum Ò³
// 3. »ñµÃ¸ÃÒ³ÉÏµÄ PageHdr ĞÅÏ¢, ¶¨Î»¼ÇÂ¼Êı×éÆğÊ¼µØÖ·
// 4. ¼ì²é¸Ã slotNum ÏîÊÇ·ñÊÇ¿ÉÓÃµÄ
// 5. ÉèÖÃ·µ»ØµÄ rec
=======
// ç›®çš„: æ ¹æ®ç»™å®šçš„ rid, è·å–ç›¸åº”çš„æ–‡ä»¶è®°å½•ä¿å­˜åˆ° rec æŒ‡å‘çš„åœ°å€
// 1. é€šè¿‡ rid è·å¾—æŒ‡å®šçš„ pageNum å’Œ slotNum
// 2. é€šè¿‡ PF_FileHandle::GetThisPage() è·å¾— pageNum é¡µ
// 3. è·å¾—è¯¥é¡µä¸Šçš„ PageHdr ä¿¡æ¯, å®šä½è®°å½•æ•°ç»„èµ·å§‹åœ°å€
// 4. æ£€æŸ¥è¯¥ slotNum é¡¹æ˜¯å¦æ˜¯å¯ç”¨çš„
// 5. è®¾ç½®è¿”å›çš„ rec
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	RC rc;
	PF_PageHandle pfPageHandle;
	RM_PageHdr *rmPageHdr;
	char *data;
	char *records;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	rec->bValid = false;
<<<<<<< HEAD
	// 1. Í¨¹ı rid »ñµÃÖ¸¶¨µÄ pageNum ºÍ slotNum
=======
	// 1. é€šè¿‡ rid è·å¾—æŒ‡å®šçš„ pageNum å’Œ slotNum
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	PageNum pageNum = rid->pageNum;
	SlotNum slotNum = rid->slotNum;

	if (pageNum < 2)
		return RM_INVALIDRID;

<<<<<<< HEAD
	// 2. Í¨¹ı PF_FileHandle::GetThisPage() »ñµÃ pageNum Ò³
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. »ñµÃ¸ÃÒ³ÉÏµÄ PageHdr ĞÅÏ¢, ¶¨Î»¼ÇÂ¼Êı×éÆğÊ¼µØÖ·
=======
	// 2. é€šè¿‡ PF_FileHandle::GetThisPage() è·å¾— pageNum é¡µ
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)))
		return rc;

	// 3. è·å¾—è¯¥é¡µä¸Šçš„ PageHdr ä¿¡æ¯, å®šä½è®°å½•æ•°ç»„èµ·å§‹åœ°å€
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
	// 4. ¼ì²é¸Ã slotNum ÏîÊÇ·ñÊÇ¿ÉÓÃµÄ
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. ÉèÖÃ·µ»ØµÄ rec
=======
	// 4. æ£€æŸ¥è¯¥ slotNum é¡¹æ˜¯å¦æ˜¯å¯ç”¨çš„
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 5. è®¾ç½®è¿”å›çš„ rec
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	rec->bValid = true;
	memcpy(rec->pData, WHICH_REC(fileHandle->rmFileHdr.recordSize, records, slotNum), 
		fileHandle->rmFileHdr.recordSize);
	rec->rid = *rid;

	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

//
<<<<<<< HEAD
// Ä¿µÄ: ÏòÎÄ¼şÖĞĞ´Èë¼ÇÂ¼Ïî
// 1. Í¨¹ı RM_FileHandle µÄ RM_FileHdr ÖĞµÄ firstFreePage »ñµÃµÚÒ»¸ö¿ÉÓÃµÄ pageNum
// Èç¹û pageNum != RM_NO_MORE_FREE_PAGE
//    1.1 Í¨¹ı GetThisPage() »ñµÃ¸ÃÒ³ pageHdr µÄµØÖ·
//    1.2 Í¨¹ı pageHdr ÖĞµÄ firstFreeSlot  
//    1.3 Ğ´Èë£¬±ê¼ÇÔà£¬¸üĞÂ freeSlot Á´, ĞŞ¸Äbitmap (¼´¸üĞÂ pageHdr µÄ firstFreeSlot)
//    1.4 Èç¹û´ËÊ± Ö®Ç°¸Ã slot Î»ÖÃµÄ nextSlot ÊÇ RM_NO_MORE_FREE_SLOT
//          1.4.1 ÄÇÃ´ FileHandle µÄ RM_FileHdr µÄ firstFreePage Í¬Ñù½øĞĞĞŞ¸Ä Î¬»¤ FreePage Á´
//          1.4.2 Î¬»¤Ö®ºó ÉèÖÃ FileHdr ÎªÔà
// Èç¹û pageNum == RM_NO_MORE_FREE_PAGE
//    2.1 »ñµÃ ×îºóÒ»Ò³ µÄ pageNum£¬ GetThisPageµ½ÄÚ´æ»º³åÇø
//    2.2 »ñµÃ¸ÃÒ³ PageHdr ÖĞµÄ slotNum, Èç¹û slotNum ÉĞÎ´´ïµ½ÊıÁ¿ÏŞÖÆ£¬¾ÍÔÚÎÄ¼şºóÃæĞ´ÈëĞÂµÄ¼ÇÂ¼
//    2.3 Èç¹û slotNum ÊıÁ¿´ïµ½ÁË recordsPerPage
//          2.3.1 Í¨¹ı AllocatePage Ïò»º³åÇøÉêÇëĞÂµÄÒ³Ãæ
//          2.3.2 ³õÊ¼»¯ PageHdr
//          2.3.3 Ğ´Èë£¬¸üĞÂ bitmap, ¸üĞÂslotCount£¬²¢ÉèÖÃ ¸ÃÒ³ ÎªÔà
=======
// ç›®çš„: å‘æ–‡ä»¶ä¸­å†™å…¥è®°å½•é¡¹
// 1. é€šè¿‡ RM_FileHandle çš„ RM_FileHdr ä¸­çš„ firstFreePage è·å¾—ç¬¬ä¸€ä¸ªå¯ç”¨çš„ pageNum
// å¦‚æœ pageNum != RM_NO_MORE_FREE_PAGE
//    1.1 é€šè¿‡ GetThisPage() è·å¾—è¯¥é¡µ pageHdr çš„åœ°å€
//    1.2 é€šè¿‡ pageHdr ä¸­çš„ firstFreeSlot  
//    1.3 å†™å…¥ï¼Œæ ‡è®°è„ï¼Œæ›´æ–° freeSlot é“¾, ä¿®æ”¹bitmap (å³æ›´æ–° pageHdr çš„ firstFreeSlot)
//    1.4 å¦‚æœæ­¤æ—¶ ä¹‹å‰è¯¥ slot ä½ç½®çš„ nextSlot æ˜¯ RM_NO_MORE_FREE_SLOT
//          1.4.1 é‚£ä¹ˆ FileHandle çš„ RM_FileHdr çš„ firstFreePage åŒæ ·è¿›è¡Œä¿®æ”¹ ç»´æŠ¤ FreePage é“¾
//          1.4.2 ç»´æŠ¤ä¹‹å è®¾ç½® FileHdr ä¸ºè„
// å¦‚æœ pageNum == RM_NO_MORE_FREE_PAGE
//    2.1 è·å¾— æœ€åä¸€é¡µ çš„ pageNumï¼Œ GetThisPageåˆ°å†…å­˜ç¼“å†²åŒº
//    2.2 è·å¾—è¯¥é¡µ PageHdr ä¸­çš„ slotNum, å¦‚æœ slotNum å°šæœªè¾¾åˆ°æ•°é‡é™åˆ¶ï¼Œå°±åœ¨æ–‡ä»¶åé¢å†™å…¥æ–°çš„è®°å½•
//    2.3 å¦‚æœ slotNum æ•°é‡è¾¾åˆ°äº† recordsPerPage
//          2.3.1 é€šè¿‡ AllocatePage å‘ç¼“å†²åŒºç”³è¯·æ–°çš„é¡µé¢
//          2.3.2 åˆå§‹åŒ– PageHdr
//          2.3.3 å†™å…¥ï¼Œæ›´æ–° bitmap, æ›´æ–°slotCountï¼Œå¹¶è®¾ç½® è¯¥é¡µ ä¸ºè„
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
		//    1.1 Í¨¹ı GetThisPage() »ñµÃ¸ÃÒ³ pageHdr µÄµØÖ·
=======
		//    1.1 é€šè¿‡ GetThisPage() è·å¾—è¯¥é¡µ pageHdr çš„åœ°å€
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), firstFreePage, &pfPageHandle)) || 
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

<<<<<<< HEAD
		//    1.2 Í¨¹ı pageHdr ÖĞµÄ firstFreeSlot  
=======
		//    1.2 é€šè¿‡ pageHdr ä¸­çš„ firstFreeSlot  
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		rmPageHdr = (RM_PageHdr*)data;
		firstFreeSlot = rmPageHdr->firstFreeSlot;

		records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
		//    1.3 Ğ´Èë£¬±ê¼ÇÔà£¬¸üĞÂ freeSlot Á´ (¼´¸üĞÂ pageHdr µÄ firstFreeSlot)
=======
		//    1.3 å†™å…¥ï¼Œæ ‡è®°è„ï¼Œæ›´æ–° freeSlot é“¾ (å³æ›´æ–° pageHdr çš„ firstFreeSlot)
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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

<<<<<<< HEAD
		//    1.4 Èç¹û´ËÊ± Ö®Ç°¸Ã slot Î»ÖÃµÄ nextSlot ÊÇ RM_NO_MORE_FREE_SLOT
=======
		//    1.4 å¦‚æœæ­¤æ—¶ ä¹‹å‰è¯¥ slot ä½ç½®çš„ nextSlot æ˜¯ RM_NO_MORE_FREE_SLOT
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
			fileHandle->rmFileHdr.firstFreePage = rmPageHdr->nextFreePage;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			fileHandle->isRMHdrDirty = true;
		}
	} else {
<<<<<<< HEAD
		// Èç¹û pageNum == RM_NO_MORE_FREE_PAGE
=======
		// å¦‚æœ pageNum == RM_NO_MORE_FREE_PAGE
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		if ((rc = GetLastPageNum(&(fileHandle->pfFileHandle), &lastPageNum)) ||
			(rc = GetThisPage(&(fileHandle->pfFileHandle), lastPageNum, &lastPageHandle)) ||
			(rc = GetData(&lastPageHandle, &data))) {
			return rc;
		}

<<<<<<< HEAD
		// 2.2 »ñµÃ¸ÃÒ³ PageHdr ÖĞµÄ slotNum, Èç¹û slotNum ÉĞÎ´´ïµ½ÊıÁ¿ÏŞÖÆ£¬¾ÍÔÚÎÄ¼şºóÃæĞ´ÈëĞÂµÄ¼ÇÂ¼
=======
		// 2.2 è·å¾—è¯¥é¡µ PageHdr ä¸­çš„ slotNum, å¦‚æœ slotNum å°šæœªè¾¾åˆ°æ•°é‡é™åˆ¶ï¼Œå°±åœ¨æ–‡ä»¶åé¢å†™å…¥æ–°çš„è®°å½•
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		rmPageHdr = (RM_PageHdr*)data;
		lastSlotNum = rmPageHdr->slotCount;
		records = data + fileHandle->rmFileHdr.slotsOffset;

		if (lastPageNum >= 2 && lastSlotNum < fileHandle->rmFileHdr.recordsPerPage) {
<<<<<<< HEAD
			// slotNum ÉĞÎ´´ïµ½ÊıÁ¿ÏŞÖÆ, ¾ÍÔÚÎÄ¼şºóÃæĞ´ÈëĞÂµÄ¼ÇÂ¼
=======
			// slotNum å°šæœªè¾¾åˆ°æ•°é‡é™åˆ¶, å°±åœ¨æ–‡ä»¶åé¢å†™å…¥æ–°çš„è®°å½•
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
			//  2.3.1 Í¨¹ı AllocatePage Ïò»º³åÇøÉêÇëĞÂµÄÒ³Ãæ
=======
			//  2.3.1 é€šè¿‡ AllocatePage å‘ç¼“å†²åŒºç”³è¯·æ–°çš„é¡µé¢
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
			if ((rc = AllocatePage(&(fileHandle->pfFileHandle), &newPageHandle)) ||
				(rc = GetData(&newPageHandle, &data)))
				return rc;

			rmPageHdr = (RM_PageHdr*)data;
			records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
			//  2.3.2 ³õÊ¼»¯ PageHdr
=======
			//  2.3.2 åˆå§‹åŒ– PageHdr
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
			rmPageHdr->firstFreeSlot = RM_NO_MORE_FREE_SLOT;
			rmPageHdr->nextFreePage = RM_NO_MORE_FREE_PAGE;
			rmPageHdr->slotCount = 0;

<<<<<<< HEAD
			//  2.3.3 Ğ´Èë£¬¸üĞÂ bitmap, ¸üĞÂslotCount£¬²¢ÉèÖÃ ¸ÃÒ³ ÎªÔà
=======
			//  2.3.3 å†™å…¥ï¼Œæ›´æ–° bitmap, æ›´æ–°slotCountï¼Œå¹¶è®¾ç½® è¯¥é¡µ ä¸ºè„
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
// Ä¿µÄ: É¾³ıÖ¸¶¨RIDÎ»ÖÃµÄ¼ÇÂ¼
// 1. Í¨¹ı GetThisPage »ñµÃ rid Ö¸Ê¾µÄÒ³
// 2. Í¨¹ı¸ÃÒ³µÄ bitMap ¼ì²é ÏàÓ¦µÄ slotNum ¼ÇÂ¼ÊÇ·ñÓĞĞ§
// 3. Î¬»¤ free slot list
// 4. ¸üĞÂ bitMap
// 5. Èç¹ûÔ­À´²»ÔÚ free Page list ÉÏ£¬ ½«¸ÃÒ³¼ÓÈë free Á´
=======
// ç›®çš„: åˆ é™¤æŒ‡å®šRIDä½ç½®çš„è®°å½•
// 1. é€šè¿‡ GetThisPage è·å¾— rid æŒ‡ç¤ºçš„é¡µ
// 2. é€šè¿‡è¯¥é¡µçš„ bitMap æ£€æŸ¥ ç›¸åº”çš„ slotNum è®°å½•æ˜¯å¦æœ‰æ•ˆ
// 3. ç»´æŠ¤ free slot list
// 4. æ›´æ–° bitMap
// 5. å¦‚æœåŸæ¥ä¸åœ¨ free Page list ä¸Šï¼Œ å°†è¯¥é¡µåŠ å…¥ free é“¾
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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

<<<<<<< HEAD
	// 1. Í¨¹ı GetThisPage »ñµÃ rid Ö¸Ê¾µÄÒ³
=======
	// 1. é€šè¿‡ GetThisPage è·å¾— rid æŒ‡ç¤ºçš„é¡µ
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

<<<<<<< HEAD
	// 2. Í¨¹ı¸ÃÒ³µÄ bitMap ¼ì²é ÏàÓ¦µÄ slotNum ¼ÇÂ¼ÊÇ·ñÓĞĞ§
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 3. Î¬»¤ free slot list
	nextFreeSlot = (int*)(REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, slotNum));
	*nextFreeSlot = rmPageHdr->firstFreeSlot;

	// 4. ¸üĞÂ bitMap
	setBit(rmPageHdr->slotBitMap, slotNum, false);

	if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
		// 5. Èç¹ûÔ­À´²»ÔÚ free Page list ÉÏ£¬ ½«¸ÃÒ³¼ÓÈë free Á´
=======
	// 2. é€šè¿‡è¯¥é¡µçš„ bitMap æ£€æŸ¥ ç›¸åº”çš„ slotNum è®°å½•æ˜¯å¦æœ‰æ•ˆ
	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	// 3. ç»´æŠ¤ free slot list
	nextFreeSlot = (int*)(REC_NEXT_SLOT(fileHandle->rmFileHdr.recordSize, records, slotNum));
	*nextFreeSlot = rmPageHdr->firstFreeSlot;

	// 4. æ›´æ–° bitMap
	setBit(rmPageHdr->slotBitMap, slotNum, false);

	if (rmPageHdr->firstFreeSlot == RM_NO_MORE_FREE_SLOT) {
		// 5. å¦‚æœåŸæ¥ä¸åœ¨ free Page list ä¸Šï¼Œ å°†è¯¥é¡µåŠ å…¥ free é“¾
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
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
<<<<<<< HEAD
// Ä¿µÄ: ¸üĞÂÖ¸¶¨ RID µÄÄÚÈİ
=======
// ç›®çš„: æ›´æ–°æŒ‡å®š RID çš„å†…å®¹
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	PageNum pageNum = rec->rid.pageNum;
	SlotNum slotNum = rec->rid.slotNum;
	RID tmp;
	char* data;
	char* records;
	RM_PageHdr* rmPageHdr;

	tmp.pageNum = pageNum;
	tmp.slotNum = slotNum;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

	if (pageNum < 2)
		return RM_INVALIDRID;

	if ((rc = GetThisPage(&(fileHandle->pfFileHandle), pageNum, &pfPageHandle)) ||
		(rc = GetData(&pfPageHandle, &data)))
		return rc;

	rmPageHdr = (RM_PageHdr*)data;
	records = data + fileHandle->rmFileHdr.slotsOffset;

	if (!getBit(rmPageHdr->slotBitMap, slotNum))
		return RM_INVALIDRID;

	memcpy(WHICH_REC(fileHandle->rmFileHdr.recordSize, records, slotNum), rec->pData,
		fileHandle->rmFileHdr.recordSize);

	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)))
		return rc;

	return SUCCESS;
}

// 
// ç›®çš„: è·å–æ–‡ä»¶å¥æŸ„ fd, è®¾ç½® RM æ§åˆ¶é¡µ (ç¬¬ 1 é¡µ)
// 1. æ ¹æ® recordSize è®¡ç®—å‡ºæ¯ä¸€é¡µå¯ä»¥æ”¾ç½®çš„è®°å½•ä¸ªæ•°
// 2. è°ƒç”¨ PF_Manager::CreateFile() å°† Paged File çš„ç›¸å…³æ§åˆ¶ä¿¡æ¯è¿›è¡Œåˆå§‹åŒ–
// 3. è°ƒç”¨ PF_Manager::OpenFile() æ‰“å¼€è¯¥æ–‡ä»¶ è·å– PF_FileHandle
// 4. é€šè¿‡è¯¥ PF_FileHandle çš„ AllocatePage æ–¹æ³•ç”³è¯·å†…å­˜ç¼“å†²åŒº æ‹¿åˆ°ç¬¬ 1 é¡µçš„ pData æŒ‡é’ˆ
// 5. åˆå§‹åŒ–ä¸€ä¸ª RM_FileHdr ç»“æ„ï¼Œå†™åˆ° pData æŒ‡å‘çš„å†…å­˜åŒº
// 6. æ ‡è®° ç¬¬ 1 é¡µ ä¸ºè„
// 7. PF_Manager::CloseFile() å°†è°ƒç”¨ ForceAllPage
//
RC RM_CreateFile (char *fileName, int recordSize)
{
	int maxRecordsNum;
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	RM_FileHdr* rmFileHdr;
	char* data;
<<<<<<< HEAD
	// ¼ì²é recordSize ÊÇ·ñºÏ·¨ 
	// (ÓÉÓÚ²ÉÓÃÁ´±íµÄ·½Ê½ËùÒÔĞèÒªÔÚ¼ÇÂ¼Ö®Ç°±£´æ nextFreeSlot ×Ö¶ÎËùÒÔ¼ÓÉÏ sizeof(int))
=======
	// æ£€æŸ¥ recordSize æ˜¯å¦åˆæ³• 
	// (ç”±äºé‡‡ç”¨é“¾è¡¨çš„æ–¹å¼æ‰€ä»¥éœ€è¦åœ¨è®°å½•ä¹‹å‰ä¿å­˜ nextFreeSlot å­—æ®µæ‰€ä»¥åŠ ä¸Š sizeof(int))
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if (recordSize + sizeof(int) > PF_PAGE_SIZE || 
		recordSize < 0)
		return RM_INVALIDRECSIZE;

<<<<<<< HEAD
	// 1. ¼ÆËãÃ¿Ò»Ò³¿ÉÒÔ·ÅÖÃµÄ¼ÇÂ¼¸öÊı
=======
	// 1. è®¡ç®—æ¯ä¸€é¡µå¯ä»¥æ”¾ç½®çš„è®°å½•ä¸ªæ•°
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	maxRecordsNum = ( ( PF_PAGE_SIZE - sizeof(RM_PageHdr) ) << 3 ) / ( (recordSize << 3) + (sizeof(int) << 3) + 1 );
	// std::cout << "PageSize: " << PF_PAGE_SIZE << " sizeof(RM_PageHdr) " << sizeof(RM_PageHdr) << std::endl;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	if ((((maxRecordsNum + 7) >> 3) +
		((recordSize + sizeof(int)) * maxRecordsNum) +
		sizeof(RM_PageHdr)) > PF_PAGE_SIZE)
		maxRecordsNum--;
	// std::cout << "maxRecordsNum: " << maxRecordsNum << std::endl;
	// std::cout << "maxBitMapNum: " << ((maxRecordsNum + 7) >> 3) << std::endl;
<<<<<<< HEAD
	// 2. µ÷ÓÃ PF_Manager::CreateFile() ½« Paged File µÄÏà¹Ø¿ØÖÆĞÅÏ¢½øĞĞ³õÊ¼»¯
	// 3. µ÷ÓÃ PF_Manager::OpenFile() ´ò¿ª¸ÃÎÄ¼ş »ñÈ¡ PF_FileHandle
=======
	// 2. è°ƒç”¨ PF_Manager::CreateFile() å°† Paged File çš„ç›¸å…³æ§åˆ¶ä¿¡æ¯è¿›è¡Œåˆå§‹åŒ–
	// 3. è°ƒç”¨ PF_Manager::OpenFile() æ‰“å¼€è¯¥æ–‡ä»¶ è·å– PF_FileHandle
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = CreateFile(fileName)) ||
		(rc = openFile(fileName, &pfFileHandle)))
		return rc;

	// 4. é€šè¿‡è¯¥ PF_FileHandle çš„ AllocatePage æ–¹æ³•ç”³è¯·å†…å­˜ç¼“å†²åŒº æ‹¿åˆ°ç¬¬ 1 é¡µçš„ pData æŒ‡é’ˆ
	if ((rc = AllocatePage(&pfFileHandle, &pfPageHandle)))
		return rc;

	assert(pfPageHandle.pFrame->page.pageNum == 1);
	if ((rc = GetData(&pfPageHandle, &data)))
		return rc;
	rmFileHdr = (RM_FileHdr*)data;

	// 5. åˆå§‹åŒ–ä¸€ä¸ª RM_FileHdr ç»“æ„ï¼Œå†™åˆ° pData æŒ‡å‘çš„å†…å­˜åŒº
	rmFileHdr->firstFreePage = RM_NO_MORE_FREE_PAGE;
	rmFileHdr->recordSize = recordSize;
	rmFileHdr->recordsPerPage = maxRecordsNum;
	rmFileHdr->slotsOffset = sizeof(RM_PageHdr) + ((maxRecordsNum + 7) >> 3);

	// 6. æ ‡è®° ç¬¬ 1 é¡µ ä¸ºè„
	// 7. PF_Manager::CloseFile() å°†è°ƒç”¨ ForceAllPage
	if ((rc = MarkDirty(&pfPageHandle)) ||
		(rc = UnpinPage(&pfPageHandle)) || 
		(rc = CloseFile(&pfFileHandle)))
		return rc;

	return SUCCESS;
}

<<<<<<< HEAD
// 
<<<<<<< HEAD
// Ä¿µÄ: ´ò¿ªÎÄ¼ş£¬½¨Á¢ RM_fileHandle
// 1. µ÷ÓÃ PF_Manager::OpenFile£¬»ñµÃÒ»¸ö PF_FileHandle
// 2. Í¨¹ı PF_FileHandle À´»ñÈ¡1ºÅÒ³ÃæÉÏµÄ RM_FileHdr ĞÅÏ¢
// 3. ³õÊ¼»¯ RM_FileHandle ³ÉÔ±
=======
// ç›®çš„: æ‰“å¼€æ–‡ä»¶ï¼Œå»ºç«‹ RM_fileHandle
// 1. è°ƒç”¨ PF_Manager::OpenFileï¼Œè·å¾—ä¸€ä¸ª PF_FileHandle
// 2. é€šè¿‡ PF_FileHandle æ¥è·å–1å·é¡µé¢ä¸Šçš„ RM_FileHdr ä¿¡æ¯
// 3. åˆå§‹åŒ– RM_FileHandle æˆå‘˜
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC rc;
	PF_FileHandle pfFileHandle;
	PF_PageHandle pfPageHandle;
	char* data;

	memset(fileHandle, 0, sizeof(RM_FileHandle));

	if (fileHandle->bOpen)
		return RM_FHOPENNED;

<<<<<<< HEAD
	// 1. µ÷ÓÃ PF_Manager::OpenFile£¬»ñµÃÒ»¸ö PF_FileHandle
	// 2. Í¨¹ı PF_FileHandle À´»ñÈ¡1ºÅÒ³ÃæÉÏµÄ RM_FileHdr ĞÅÏ¢
=======
	// 1. è°ƒç”¨ PF_Manager::OpenFileï¼Œè·å¾—ä¸€ä¸ª PF_FileHandle
	// 2. é€šè¿‡ PF_FileHandle æ¥è·å–1å·é¡µé¢ä¸Šçš„ RM_FileHdr ä¿¡æ¯
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = openFile(fileName, &pfFileHandle)) ||
		(rc = GetThisPage(&pfFileHandle, 1, &pfPageHandle))) {
		return rc;
	}

<<<<<<< HEAD
	// 3. ³õÊ¼»¯ RM_FileHandle ³ÉÔ±
=======
	// 3. åˆå§‹åŒ– RM_FileHandle æˆå‘˜
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	fileHandle->bOpen = true;
	fileHandle->isRMHdrDirty = false;
	fileHandle->pfFileHandle = pfFileHandle;
	if ((rc = GetData(&pfPageHandle, &data))) {
		return rc;
	}
	fileHandle->rmFileHdr = *((RM_FileHdr*)data);

<<<<<<< HEAD
	// 4. ÊÍ·ÅÄÚ´æ»º³åÇø
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;
=======
//Ä¿µÄ£»¶ÁÈ¡ÎÄ¼ş£¬ÉèÖÃRM_FileHandle
=======
	// 4. é‡Šæ”¾å†…å­˜ç¼“å†²åŒº
	if ((rc = UnpinPage(&pfPageHandle)))
		return rc;
=======
//ç›®çš„ï¼›è¯»å–æ–‡ä»¶ï¼Œè®¾ç½®RM_FileHandle
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	RM_FileHdr *rm_FileHdr;
    if((rc=openFile(fileName,&fileHandle->pfFileHandle)) ||
		rc=GetThisPage(&fileHandle->pfFileHandle,1,&pfPageHandle))
		return rc;
	fileHandle->bOpen=true;
	rm_FileHdr = (RM_FileHdr*)pfPageHandle.pFrame->page.pData;
	
	fileHandle->rmFileHdr = *rm_FileHdr;
>>>>>>> f0594f8d9057450446ce7c495932a73b2b65bf97

	return SUCCESS;
}


// 
<<<<<<< HEAD
// Ä¿µÄ: ¹Ø±ÕÎÄ¼ş£¬ÅĞ¶Ï RM_FileHdr ÊÇ·ñ±»ĞŞ¸Ä
// Èç¹û·¢ÉúÁËĞŞ¸Ä£¬ GetThisPageµ½ÄÚ´æ»º³åÇø£¬Ğ´Èë¸ÃÒ³, ±ê¼ÇÎªÔàÖ®ºó
// µ÷ÓÃ PF_Manager::CloseFile() ¹Ø±Õ RM ÎÄ¼ş
=======
// ç›®çš„: å…³é—­æ–‡ä»¶ï¼Œåˆ¤æ–­ RM_FileHdr æ˜¯å¦è¢«ä¿®æ”¹
// å¦‚æœå‘ç”Ÿäº†ä¿®æ”¹ï¼Œ GetThisPageåˆ°å†…å­˜ç¼“å†²åŒºï¼Œå†™å…¥è¯¥é¡µ, æ ‡è®°ä¸ºè„ä¹‹å
// è°ƒç”¨ PF_Manager::CloseFile() å…³é—­ RM æ–‡ä»¶
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
//
RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC rc;
	PF_PageHandle pfPageHandle;
	char* data;

	if (!(fileHandle->bOpen))
		return RM_FHCLOSED;

<<<<<<< HEAD
	// ÅĞ¶ÏÊÇ·ñ·¢ÉúÁËĞŞ¸Ä
=======
	// åˆ¤æ–­æ˜¯å¦å‘ç”Ÿäº†ä¿®æ”¹
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if (fileHandle->isRMHdrDirty) {
		if ((rc = GetThisPage(&(fileHandle->pfFileHandle), 1, &pfPageHandle)) ||
			(rc = GetData(&pfPageHandle, &data)))
			return rc;

<<<<<<< HEAD
		// ĞèÒª¿½±´
=======
		// éœ€è¦æ‹·è´
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
		memcpy(data, (char*) & (fileHandle->rmFileHdr), sizeof(RM_FileHdr));
		
		if ((rc = MarkDirty(&pfPageHandle)) ||
			(rc = UnpinPage(&pfPageHandle)))
			return rc;
	}

<<<<<<< HEAD
	// µ÷ÓÃ PF_Manager::CloseFile() ¹Ø±Õ RM ÎÄ¼ş
=======
	// è°ƒç”¨ PF_Manager::CloseFile() å…³é—­ RM æ–‡ä»¶
>>>>>>> 21862d3ad3607d49ccb1057ad9a98fdb4d4c4be5
	if ((rc = CloseFile(&(fileHandle->pfFileHandle))))
		return rc;

	fileHandle->bOpen = false;
	fileHandle->isRMHdrDirty = false;

	return SUCCESS;
}
