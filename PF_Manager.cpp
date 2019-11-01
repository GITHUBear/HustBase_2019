#include "stdafx.h"
#include "PF_Manager.h"
#include <iostream>

BF_Manager bf_manager;                          // [dim dew] 建立在全局区的缓冲区

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

const RC PF_CreateFile(const char *fileName)
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
	bitmap=page.pData+(int)PF_FILESUBHDR_SIZE;         // [dim dew] bitmap的大小受限导致 Paged File 的大小也是受限制的
													   // 但是下面的代码当中对申请的 pageNum 并没有进行限制
	// TODO: 在 PF_FileHdr中保存 Paged File 的限制, 并在相关代码处进行检查
	fileSubHeader=(PF_FileSubHeader *)page.pData;
	fileSubHeader->nAllocatedPages=1;                  // [dim dew] 分配的首页控制页
	bitmap[0]|=0x01;                                   // [dim dew] 标志控制页used
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
PF_FileHandle * getPF_FileHandle(void )                  // [dim dew] ??? 这也能叫get? 应该是留着传给openFile的
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
	// 分配页面留给首页
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
	// 完成首页到内存的写入
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
		if(bf_manager.frame[i].accTime<mintime){           // 真·LRU
			min=i;
			mintime=bf_manager.frame[i].accTime;
		}
	}
	if(flag==false)
		return PF_NOBUF;
	if(bf_manager.frame[min].bDirty==true){
		offset=(bf_manager.frame[min].page.pageNum)*sizeof(Page);
		if(_lseek(bf_manager.frame[min].fileDesc,offset,SEEK_SET)==offset-1)           // 为什么不是-1？？
			return PF_FILEERR;
		if(_write(bf_manager.frame[min].fileDesc,&(bf_manager.frame[min].page),sizeof(Page))!=sizeof(Page))
			return PF_FILEERR;
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

	// [?bug?] 这里不应该确保一下 assert(fileHandle->bopen) 吗
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	if (pageNum > fileHandle->pFileSubHeader->pageCount)
		return PF_INVALIDPAGENUM;
	if((fileHandle->pBitmap[pageNum/8]&(1<<(pageNum%8)))==0)
		return PF_INVALIDPAGENUM;
	pPageHandle->bOpen=true;
	// 判断一下当前缓冲区中是否存在 fd(fileName勉强也行吧) + pageNum
	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if (bf_manager.frame[i].fileDesc != fileHandle->fileDesc)
			continue;
		/*if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;*/
		if(bf_manager.frame[i].page.pageNum==pageNum){
			pPageHandle->pFrame=bf_manager.frame+i;
			pPageHandle->pFrame->pinCount++;
			pPageHandle->pFrame->accTime=clock();
			return SUCCESS;
		}
	}
	// 缓冲区内不存在，挪用缓冲区读入page
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
	// 先用之前分配但是Dispose掉的页面，增加磁盘利用率
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
	// 之前都是分配着的，申请一个缓冲区放置新的pageNum对应的页
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
	// 所以每个page前4个字节存对应的page编号有啥意义呢
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

	// 也请务必保证 fileHandle->bOpen 为真
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
		if (bf_manager.frame[i].fileDesc != fileHandle->fileDesc)
			continue;
		/*if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;*/
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
	// 保证一下 pageHandle->bopen 为真
	if (!(pageHandle->bOpen))
		return PF_PHCLOSED;

	pageHandle->pFrame->bDirty=true;
	return SUCCESS;
}

const RC UnpinPage(PF_PageHandle *pageHandle)
{
	// 保证一下 pageHandle->bopen 为真
	if (!(pageHandle->bOpen))
		return PF_PHCLOSED;

	pageHandle->pFrame->pinCount--;
	return SUCCESS;
}

const RC ForcePage(PF_FileHandle *fileHandle,PageNum pageNum)
{
	int i;

	// 也请务必保证 fileHandle->bOpen 为真
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if (bf_manager.frame[i].fileDesc != fileHandle->fileDesc)
			continue;
		/*if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;*/
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

	// 也请务必保证 fileHandle->bOpen 为真
	if (!(fileHandle->bopen))
		return PF_FHCLOSED;

	for(i=0;i<PF_BUFFER_SIZE;i++){
		if(bf_manager.allocated[i]==false)
			continue;
		if (bf_manager.frame[i].fileDesc != fileHandle->fileDesc)
			continue;
		/*if(strcmp(bf_manager.frame[i].fileName,fileHandle->fileName)!=0)
			continue;*/

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