#include "stdafx.h"
#include "PF_Manager.h"

BF_Manager bf_manager;                          // [dim dew] ½¨Á¢ÔÚÈ«¾ÖÇøµÄ»º³åÇø

const RC AllocateBlock(Frame **buf);
const RC DisposeBlock(Frame *buf);
const RC ForceAllPages(PF_FileHandle *fileHandle);

void inti()
{
	int i;
	for(i=0;i<PF_BUFFER_SIZE;i++){
		bf_manager.allocated[i]=false;
		bf_manager.frame[i].pinCount=0;
	}
}

const RC CreateFile(const char *fileName)
{
	int fd;
	char *bitmap;
	PF_FileSubHeader *fileSubHeader;
	fd=_open(fileName,_O_RDWR|_O_CREAT|_O_EXCL|_O_BINARY,_S_IREAD|_S_IWRITE);
	if (fd < 0)
		return PF_EXIST;
	_close(fd);
	fd=open(fileName,_O_RDWR);
	Page page;
	memset(&page,0,PF_PAGESIZE);
	bitmap=page.pData+(int)PF_FILESUBHDR_SIZE;         // [dim dew] bitmapµÄ´óÐ¡ÊÜÏÞµ¼ÖÂ Paged File µÄ´óÐ¡Ò²ÊÇÊÜÏÞÖÆµÄ
													   // µ«ÊÇÏÂÃæµÄ´úÂëµ±ÖÐ¶ÔÉêÇëµÄ pageNum ²¢Ã»ÓÐ½øÐÐÏÞÖÆ
	// TODO: ÔÚ PF_FileHdrÖÐ±£´æ Paged File µÄÏÞÖÆ, ²¢ÔÚÏà¹Ø´úÂë´¦½øÐÐ¼ì²é
	fileSubHeader=(PF_FileSubHeader *)page.pData;
	fileSubHeader->nAllocatedPages=1;                  // [dim dew] ·ÖÅäµÄÊ×Ò³¿ØÖÆÒ³
	bitmap[0]|=0x01;                                   // [dim dew] ±êÖ¾¿ØÖÆÒ³used
	if(_lseek(fd,0,SEEK_SET)==-1)
		return PF_FILEERR;
	if(_write(fd,(char *)&page,sizeof(Page))!=sizeof(Page)){
		_close(fd);
		return PF_FILEERR;
	}
	if(_close(fd)<0)
		return PF_FILEERR;
	return SUCCESS;
}
PF_FileHandle * getPF_FileHandle(void )                  // [dim dew] ??? ÕâÒ²ÄÜ½Ðget? Ó¦¸ÃÊÇÁô×Å´«¸øopenFileµÄ
{
	PF_FileHandle *p=(PF_FileHandle *)malloc(sizeof( PF_FileHandle));
	p->bopen=false;
	return p;
}                      

const RC openFile(char *fileName,PF_FileHandle *fileHandle)
{
	int fd;
	PF_FileHandle *pfilehandle=fileHandle;
	RC tmp;
	if((fd=_open(fileName,O_RDWR|_O_BINARY))<0)
		return PF_FILEERR;
	pfilehandle->bopen=true;
	pfilehandle->fileName=fileName;
	pfilehandle->fileDesc=fd;
	// ·ÖÅäÒ³ÃæÁô¸øÊ×Ò³
	if((tmp=AllocateBlock(&pfilehandle->pHdrFrame))!=SUCCESS){
		_close(fd);
		return tmp;
	}
	pfilehandle->pHdrFrame->bDirty=false;
	pfilehandle->pHdrFrame->pinCount=1;
	pfilehandle->pHdrFrame->accTime=clock();
	pfilehandle->pHdrFrame->fileDesc=fd;
	pfilehandle->pHdrFrame->fileName=fileName;
	if(_lseek(fd,0,SEEK_SET)==-1){
		DisposeBlock(pfilehandle->pHdrFrame);
		_close(fd);
		return PF_FILEERR;
	}
	// Íê³ÉÊ×Ò³µ½ÄÚ´æµÄÐ´Èë
	if(_read(fd,&(pfilehandle->pHdrFrame->page),sizeof(Page))!=sizeof(Page)){
		DisposeBlock(pfilehandle->pHdrFrame);
		_close(fd);
		return PF_FILEERR;
	}
	pfilehandle->pHdrPage=&(pfilehandle->pHdrFrame->page);
	pfilehandle->pBitmap=pfilehandle->pHdrPage->pData+PF_FILESUBHDR_SIZE;
	pfilehandle->pFileSubHeader=(PF_FileSubHeader *)pfilehandle->pHdrPage->pData;
	fileHandle=pfilehandle;
	return SUCCESS;
}

const RC CloseFile(PF_FileHandle *fileHandle)
{
	RC tmp;
	fileHandle->pHdrFrame->pinCount--;
	if((tmp=ForceAllPages(fileHandle))!=SUCCESS)
		return tmp;
	if(_close(fileHandle->fileDesc)<0)
		return PF_FILEERR;
	return SUCCESS;
}

const RC AllocateBlock(Frame **buffer)
{
	int i,min,offset;
	bool flag;
	clock_t mintime;
	for(i=0;i<PF_BUFFER_SIZE;i++)
		if(bf_manager.allocated[i]==false){
			bf_manager.allocated[i]=true;
			*buffer=bf_manager.frame+i;
			return SUCCESS;
		}
	flag=false;
	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.frame[i].pinCount!=0)
			continue;
		if(flag==false){
			flag=true;
			min=i;
			mintime=bf_manager.frame[i].accTime;
		}
		if(bf_manager.frame[i].accTime<mintime){           // Õæ¡¤LRU
			min=i;
			mintime=bf_manager.frame[i].accTime;
		}
	}
	if(flag==false)
		return PF_NOBUF;
	if(bf_manager.frame[min].bDirty==true){
		offset=(bf_manager.frame[min].page.pageNum)*sizeof(Page);
		if(_lseek(bf_manager.frame[min].fileDesc,offset,SEEK_SET)==offset-1)           // ÎªÊ²Ã´²»ÊÇ-1£¿£¿
			return PF_FILEERR;
		if(_write(bf_manager.frame[min].fileDesc,&(bf_manager.frame[min].page),sizeof(Page))!=sizeof(Page))
			return PF_FILEERR;
			
		//bf_manager.frame[min].bDirty==flase è¿™é‡Œåº”è¯¥ä¿®æ”¹æˆfalseã€‚è™½ç„¶openFileä¼šè®¾ç½®ï¼Œä½†æ˜¯ä¸ºäº†ç¡®ä¿æ­£ç¡®è¿˜æ˜¯åŠ ä¸Šã€‚
		//æˆ–è€…è¿™é‡Œå¯ä»¥ç›´æŽ¥è°ƒç”¨DisposeBlockï¼Ÿ
	}

	*buffer=bf_manager.frame+min;
	return SUCCESS;
}

const RC DisposeBlock(Frame *buf)
{
	if(buf->pinCount!=0)
		return PF_PAGEPINNED;
	if(buf->bDirty==true){
		if(_lseek(buf->fileDesc,buf->page.pageNum*sizeof(Page),SEEK_SET)<0)
			return PF_FILEERR;
		if(_write(buf->fileDesc,&(buf->page),sizeof(Page))!=sizeof(Page))
			return PF_FILEERR;
	}
	buf->bDirty=false;
	bf_manager.allocated[buf-bf_manager.frame]=false;
	return SUCCESS;
}

//æ”¹åå«Create_PF_PageHandleä¼šæ›´å¥½ã€‚è°è®©ç”¨æ˜¯cï¼Œæ²¡æœ‰æž„é€ å‡½æ•°å‘¢
PF_PageHandle* getPF_PageHandle()
{
	PF_PageHandle *p=(PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	p->bOpen=false;
	return p;
}

const RC GetThisPage(PF_FileHandle *fileHandle,PageNum pageNum,PF_PageHandle *pageHandle)
{
	int i,nread,offset;
	RC tmp;
	PF_PageHandle *pPageHandle=pageHandle;

	// [?bug?] ÕâÀï²»Ó¦¸ÃÈ·±£Ò»ÏÂ assert(fileHandle->bopen) Âð
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	if(pageNum>fileHandle->pFileSubHeader->pageCount)
		return PF_INVALIDPAGENUM;
	if((fileHandle->pBitmap[pageNum/8]&(1<<(pageNum%8)))==0)
		return PF_INVALIDPAGENUM;
	pPageHandle->bOpen=true;
	// ÅÐ¶ÏÒ»ÏÂµ±Ç°»º³åÇøÖÐÊÇ·ñ´æÔÚ fd(fileNameÃãÇ¿Ò²ÐÐ°É) + pageNum
	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;
		if(bf_manager.frame[i].page.pageNum==pageNum){
			pPageHandle->pFrame=bf_manager.frame+i;
			pPageHandle->pFrame->pinCount++;
			pPageHandle->pFrame->accTime=clock();
			return SUCCESS;
		}
	}
	// »º³åÇøÄÚ²»´æÔÚ£¬Å²ÓÃ»º³åÇø¶ÁÈëpage
	if((tmp=AllocateBlock(&(pPageHandle->pFrame)))!=SUCCESS){
		return tmp;
	}
	pPageHandle->pFrame->bDirty=false;
	pPageHandle->pFrame->fileDesc=fileHandle->fileDesc;
	pPageHandle->pFrame->fileName=fileHandle->fileName;
	pPageHandle->pFrame->pinCount=1;
	pPageHandle->pFrame->accTime=clock();
	offset=pageNum*sizeof(Page);
	if(_lseek(fileHandle->fileDesc,offset,SEEK_SET)==offset-1){
		bf_manager.allocated[pPageHandle->pFrame-bf_manager.frame]=false;
		return PF_FILEERR;
	}
	if((nread=read(fileHandle->fileDesc,&(pPageHandle->pFrame->page),sizeof(Page)))!=sizeof(Page)){
		bf_manager.allocated[pPageHandle->pFrame-bf_manager.frame]=false;
		return PF_FILEERR;
	}
	return SUCCESS;
}
	
const RC AllocatePage(PF_FileHandle *fileHandle,PF_PageHandle *pageHandle)
{
	PF_PageHandle *pPageHandle=pageHandle;
	RC tmp;
	int i,byte,bit;
	fileHandle->pHdrFrame->bDirty=true;
	// ÏÈÓÃÖ®Ç°·ÖÅäµ«ÊÇDisposeµôµÄÒ³Ãæ£¬Ôö¼Ó´ÅÅÌÀûÓÃÂÊ
	if((fileHandle->pFileSubHeader->nAllocatedPages)<=(fileHandle->pFileSubHeader->pageCount)){
		for(i=0;i<=fileHandle->pFileSubHeader->pageCount;i++){
			byte=i/8;
			bit=i%8;
			if(((fileHandle->pBitmap[byte])&(1<<bit))==0){
				(fileHandle->pFileSubHeader->nAllocatedPages)++;
				fileHandle->pBitmap[byte]|=(1<<bit);
				break;
			}
		}
		if(i<=fileHandle->pFileSubHeader->pageCount)
			return GetThisPage(fileHandle,i,pageHandle);
		
	}
	// Ö®Ç°¶¼ÊÇ·ÖÅä×ÅµÄ£¬ÉêÇëÒ»¸ö»º³åÇø·ÅÖÃÐÂµÄpageNum¶ÔÓ¦µÄÒ³
	fileHandle->pFileSubHeader->nAllocatedPages++;
	fileHandle->pFileSubHeader->pageCount++;
	byte=fileHandle->pFileSubHeader->pageCount/8;
	bit=fileHandle->pFileSubHeader->pageCount%8;
	fileHandle->pBitmap[byte]|=(1<<bit);
	if((tmp=AllocateBlock(&(pPageHandle->pFrame)))!=SUCCESS){
		return tmp;
	}
	pPageHandle->pFrame->bDirty=false;
	pPageHandle->pFrame->fileDesc=fileHandle->fileDesc;
	pPageHandle->pFrame->fileName=fileHandle->fileName;
	pPageHandle->pFrame->pinCount=1;
	pPageHandle->pFrame->accTime=clock();
	memset(&(pPageHandle->pFrame->page),0,sizeof(Page));
	// ËùÒÔÃ¿¸öpageÇ°4¸ö×Ö½Ú´æ¶ÔÓ¦µÄpage±àºÅÓÐÉ¶ÒâÒåÄØ
	pPageHandle->pFrame->page.pageNum=fileHandle->pFileSubHeader->pageCount;
	if(_lseek(fileHandle->fileDesc,0,SEEK_END)==-1){
		bf_manager.allocated[pPageHandle->pFrame-bf_manager.frame]=false;
		return PF_FILEERR;
	}
	if(_write(fileHandle->fileDesc,&(pPageHandle->pFrame->page),sizeof(Page))!=sizeof(Page)){
		bf_manager.allocated[pPageHandle->pFrame-bf_manager.frame]=false;
		return PF_FILEERR;
	}
	
	return SUCCESS;
}

const RC GetPageNum(PF_PageHandle *pageHandle,PageNum *pageNum)
{
	if(pageHandle->bOpen==false)
		return PF_PHCLOSED;
	*pageNum=pageHandle->pFrame->page.pageNum;
	return SUCCESS;
}

const RC GetData(PF_PageHandle *pageHandle,char **pData)
{
	if(pageHandle->bOpen==false)
		return PF_PHCLOSED;
	*pData=pageHandle->pFrame->page.pData;
	return SUCCESS;
}

const RC DisposePage(PF_FileHandle *fileHandle,PageNum pageNum)
{
	int i;
	char tmp;

	// Ò²ÇëÎñ±Ø±£Ö¤ fileHandle->bOpen ÎªÕæ
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	if(pageNum>fileHandle->pFileSubHeader->pageCount)
		return PF_INVALIDPAGENUM;
	if(((fileHandle->pBitmap[pageNum/8])&(1<<(pageNum%8)))==0)
		return PF_INVALIDPAGENUM;
	fileHandle->pHdrFrame->bDirty=true;
	fileHandle->pFileSubHeader->nAllocatedPages--;
	tmp=1<<(pageNum%8);
	fileHandle->pBitmap[pageNum/8]&=~tmp;
	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;
		if(bf_manager.frame[i].page.pageNum==pageNum){
			if(bf_manager.frame[i].pinCount!=0)
				return PF_PAGEPINNED;
			bf_manager.allocated[i]=false;
			return SUCCESS;
		}
	}
	return SUCCESS;
}

const RC MarkDirty(PF_PageHandle *pageHandle)
{
	// ±£Ö¤Ò»ÏÂ pageHandle->bopen ÎªÕæ
	if (!(pageHandle->bOpen))
		return PF_PHCLOSED;

	pageHandle->pFrame->bDirty=true;
	return SUCCESS;
}

const RC UnpinPage(PF_PageHandle *pageHandle)
{
	// ±£Ö¤Ò»ÏÂ pageHandle->bopen ÎªÕæ
	if (!(pageHandle->bOpen))
		return PF_PHCLOSED;

	pageHandle->pFrame->pinCount--;
	return SUCCESS;
}

const RC ForcePage(PF_FileHandle *fileHandle,PageNum pageNum)
{
	int i;

	// Ò²ÇëÎñ±Ø±£Ö¤ fileHandle->bOpen ÎªÕæ
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;
		if(bf_manager.frame[i].page.pageNum==pageNum){
			if(bf_manager.frame[i].pinCount!=0)
				return PF_PAGEPINNED;
			if(bf_manager.frame[i].bDirty==true){
				if(_lseek(fileHandle->fileDesc,pageNum*PF_PAGESIZE,SEEK_SET)<0)
					return PF_FILEERR;
				if(_write(fileHandle->fileDesc,&(bf_manager.frame[i].page),PF_PAGESIZE)<0)
					return PF_FILEERR;
			}
			bf_manager.allocated[i]=false;
		}
	}
	return SUCCESS;
}

const RC ForceAllPages(PF_FileHandle *fileHandle)
{
	int i,offset;

	// Ò²ÇëÎñ±Ø±£Ö¤ fileHandle->bOpen ÎªÕæ
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;

		if(bf_manager.frame[i].bDirty==true){
			offset=(bf_manager.frame[i].page.pageNum)*sizeof(Page);
			if(_lseek(fileHandle->fileDesc,offset,SEEK_SET)==offset-1)
				return PF_FILEERR;
			if(_write(fileHandle->fileDesc,&(bf_manager.frame[i].page),sizeof(Page))!=sizeof(Page))
				return PF_FILEERR;
		}
		bf_manager.allocated[i]=false;
	}
	return SUCCESS;
}

const RC GetLastPageNum(PF_FileHandle* fileHandle, PageNum* pageNum)
{
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	*pageNum = fileHandle->pFileSubHeader->pageCount;
	return SUCCESS;
}