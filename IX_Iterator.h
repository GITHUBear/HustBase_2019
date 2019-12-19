#pragma once
#include "SYS_Manager.h"
#include <string>

class IX_Iterator
{
public:
	/*	Construct a indexscan iterator with given ix filename
	 *	set rec_size the size of table record
	 *	set is_success 1 if construction meets no error
	 *	otherwise set is_success 0 and return
	 */
	IX_Iterator(char* tableName, char* columnName, CompOp op, char* value, int& rec_size, int& is_success)
	{
		is_success = 0;
		if (tableName == NULL || columnName == NULL) {
			return;
		}
		RC rc;
		AttrEntry attribute;
		char indexName[SIZE_TABLE_NAME];
		if ((rc = ColumnMetaGet(tableName, columnName, &attribute))) {
			if (attribute.ix_flag == 0) {
				return;
			}
		}
		memcpy(indexName, attribute.indexName.c_str(), SIZE_TABLE_NAME);

		if ((rc = GetRecordSize(tableName, &rec_size))) {
			return;
		}

		indexHandle = new IX_IndexHandle;
		if ((rc = OpenIndex(indexName, indexHandle))) {
			return;
		}
		indexScan = new IX_IndexScan;
		if ((rc = OpenIndexScan(indexScan, indexHandle, op, value))) {
			CloseIndex(indexHandle);
			return;
		}
		rmFileHandle = new RM_FileHandle;
		if ((rc = RM_OpenFile(tableName, rmFileHandle))) {
			CloseIndexScan(indexScan);
			CloseIndex(indexHandle);
			return;
		}
		rmFileName = tableName;
		is_success = 1;
	}
	IX_Iterator() : indexHandle(NULL), indexScan(NULL), rmFileHandle(NULL) {}

	IX_Iterator(const IX_Iterator& old) {
		this->indexHandle = old.indexHandle;
		this->indexScan = old.indexScan;
		this->rmFileHandle = old.rmFileHandle;
		this->rmFileName = old.rmFileName;
	}

	void Close()
	{
		if (rmFileHandle) {
			RM_CloseFile(rmFileHandle);
		}
		if (indexScan) {
			CloseIndexScan(indexScan);
		}
		if (indexHandle) {
			CloseIndex(indexHandle);
		}

	}

	//get Next Record through IX file
	//return SUCCESS if get a valid entry
	//return IX_EOF if no more entry
	//return non_zero RC if met other error
	RC Next(RM_Record* rm_Record) {
		RC rc;
		RID rid;
		if ((rc = IX_GetNextEntry(indexScan, &rid)) ||
			(rc = GetRec(rmFileHandle, &rid, rm_Record))) {
			return rc;
		}
		return SUCCESS;
	}

private:
	IX_IndexHandle* indexHandle;
	IX_IndexScan* indexScan;
	RM_FileHandle* rmFileHandle;
	std::string rmFileName;
};

