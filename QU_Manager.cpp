#include "StdAfx.h"
#include "QU_Manager.h"
#include <assert.h>

void Init_Result(SelResult* res){
	res->next_res = NULL;
}

void Destory_Result(SelResult* res){
	for(int i = 0;i < res->row_num; i++){
		for(int j = 0;j < res->col_num; j++){
			delete[] res->res[i][j];
		}
		delete[] res->res[i];
	}
	if(res->next_res != NULL){
		Destory_Result(res->next_res);
	}
}

// RC Query(char* sql, SelResult* res){
// 	return SUCCESS;
// }
#define CHECK(rc) if (rc != SUCCESS) {return rc;}

RC GetRecordByTableName(char* tableName, QU_Records *records) {
	RM_FileHandle fileHandle;
	RC rc = RM_OpenFile(tableName, &fileHandle);
	CHECK(rc);
	RM_FileScan fileScan;
	rc = OpenScan(&fileScan, &fileHandle, 0, NULL);
	CHECK(rc);
	records->nRecords = 0;
	while(1) {
		RM_Record* record = new RM_Record;
		int recordSize;
		rc = GetRecordSize(tableName, &recordSize);
		CHECK(rc);
		record->pData = new char[recordSize];
		memset(record->pData, 0, recordSize);
		rc = GetNextRec(&fileScan, record);
		if (rc == RM_EOF) break;
		records->records[records->nRecords++] = record;
	}
	return SUCCESS;
}

RC Select(selects* select, SelResult* res){
	if (res == NULL) {
		return INVALID_VALUES;
	}
	Init_Result(res);
	int num = 1;
	RC rc;
	std::vector<std::pair<char*, QU_Records*>> tables;
	for (int i = 0;i < select->nRelations; ++i) {
		tables.push_back(std::make_pair(select->relations[i], new QU_Records));
		rc = GetRecordByTableName(select->relations[i], tables[i].second);
		CHECK(rc);
		num *= tables[i].second->nRecords;
	}
	SelResult selResult;
	for (int i = 0;i < num; ++i) {
		int x = i;
		std::vector<RM_Record*> record;
		for (int j = 0;j < tables.size(); ++j) {
			record.push_back(tables[j].second->records[x%tables[j].second->nRecords]);
			x /= tables[j].second->nRecords;
		}
		bool checked = 1;
		for (int j = 0;j < select->nConditions; ++j) {
			Condition* cond = &select->conditions[j];
			Value lhsValue, rhsValue;
			if (cond->bLhsIsAttr) {
				for (int k = 0;k < tables.size(); ++k) {
					if (cond->lhsAttr.relName == NULL || strcmp(cond->lhsAttr.relName, tables[k].first) == 0) {
						rc = getRecordValue(tables[k].first, record[k], cond->lhsAttr.attrName, &lhsValue);
						if (rc == SUCCESS) {
							break;
						}
					}
				}
			} else {
				lhsValue = cond->lhsValue;
			}

			if (cond->bRhsIsAttr) {
				for (int k = 0;k < tables.size(); ++k) {
					if (cond->rhsAttr.relName == NULL || strcmp(cond->rhsAttr.relName, tables[k].first) == 0) {
						rc = getRecordValue(tables[k].first, record[k], cond->rhsAttr.attrName, &rhsValue);
						if (rc == SUCCESS) {
							break;
						}
					}
				}
			} else {
				rhsValue = cond->rhsValue;
			}
			if (!check_cond(cond->op, lhsValue, rhsValue)) {
				checked = 0;
				break;
			}
		}
		if (checked) {
			
		}

	}
	return SUCCESS;
}

RC getRecordValue(char* relName, RM_Record* rmRecord, const char* attrName, Value* value) {
	int attrCount;
	RC rc;
	std::vector<AttrEntry> attributes;
	if ((rc = ColumnEntryGet(relName, &attrCount, attributes))) {
		return rc;
	}
	bool found = 0;
	char* pData = rmRecord->pData;
	for (int i = 0; i < attrCount; i++) {
		if (attributes[i].columnName.compare(attrName) == 0) {
			value->type = attributes[i].attrType;
			value->data = pData + attributes[i].attrOffset;
			found = 1;
		}
	}
	if (found) {
		return SUCCESS;
	}
	return ATTR_NOT_EXIST;
}