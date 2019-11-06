#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"


RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//≥ı ºªØ…®√Ë
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

RC RM_CreateFile (char *fileName, int recordSize)
{
	return SUCCESS;
}
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	return SUCCESS;
}

RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	return SUCCESS;
}
