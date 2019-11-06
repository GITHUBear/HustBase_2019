#ifndef __QUERY_MANAGER_H_
#define __QUERY_MANAGER_H_
#include "str.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"

typedef struct Records{
	int nRecords;
	RM_Record* records[MAX_NUM];
}QU_Records;

typedef struct SelResult {
	int col_num;
	int row_num;
	AttrType type[20];	//结果集各字段的数据类型
	int length[20];		//结果集各字段值的长度
	char fields[20][20];//最多二十个字段名，而且每个字段的长度不超过20
	char** res[100];	//最多一百条记录
	SelResult* next_res;
}SelResult;

void Init_Result(SelResult* res);
void Destory_Result(SelResult* res);

RC Query(char* sql, SelResult* res);

RC Select(int nSelAttrs, RelAttr** selAttrs, int nRelations, char** relations, int nConditions, Condition* conditions, SelResult* res);
int GetValueLength(const Value* v);
const char* GetFullColumnName(RelAttr* relAttr);
RC GetRecordByTableName(char* tableName, QU_Records *records);
RC GetRecordValue(RM_Record* rmRecord, const char* attrName, const std::vector<AttrEntry>& attributes, Value* value);
RC check_cond(const CompOp& op, const Value& lhsValue, const Value& rhsValue, bool& res);
RC check_cmp(const CompOp& op, const int& cmp, bool& res);
#endif