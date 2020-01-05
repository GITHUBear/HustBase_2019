#ifndef __QUERY_MANAGER_H_
#define __QUERY_MANAGER_H_
#include "str.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"
#include "IX_Iterator.h"
#include <chrono>
using namespace std::chrono;

typedef std::vector<RM_Record*> QU_Records;

typedef struct SelResult {
	int col_num;
	int row_num;
	AttrType type[20];	//结果集各字段的数据类型
	int length[20];		//结果集各字段值的长度
	char fields[20][20];//最多二十个字段名，而且每个字段的长度不超过20
	char** res[100];	//最多一百条记录
	SelResult* next_res;
}SelResult;

typedef struct SelStatus {
	bool is_index;
	int i;
	IX_Iterator index;
	SelStatus(int _i) : is_index(0), i(_i), index() {}
	SelStatus(
		char* tableName,
		char* columnName,
		CompOp op,
		char* value,
		int& is_success) : is_index(1), i(0), index(tableName, columnName, op, value, i, is_success) {}
}SelStatus;

void Init_Result(SelResult* res);
void Destory_Result(SelResult* res);

RC Query(char* sql, SelResult* res);

RC Select(int nSelAttrs, RelAttr** selAttrs, int nRelations, char** relations, int nConditions, Condition* conditions, SelResult* res);
int GetValueLength(const Value* v);
const char* GetFullColumnName(RelAttr* relAttr);
RC GetRecordByTableName(char* tableName, QU_Records &records);
RC GetRecordValue(RM_Record* rmRecord, const char* attrName, const AttrEntry& attr, Value& value);
RC GetValueForCond(bool isAttr, RelAttr attr, Value cond_value, std::map<std::pair<std::string, std::string>, AttrEntry>& tableAttrs, RM_Record* record, Value& value);
RC FindAttr(std::map<std::pair<std::string, std::string>, AttrEntry>& tableAttrs, const std::string& tableName, const std::string& columnName, AttrEntry& attr);
RC FilterRecordByCondition(std::map<std::pair<std::string, std::string>, AttrEntry>& tableAttrMap, const char* relation, std::vector<Condition> conditions, QU_Records& in, QU_Records& out);
RC check_cond(const CompOp& op, const Value& lhsValue, const Value& rhsValue, bool& res);
RC check_cmp(const CompOp& op, const int& cmp, bool& res);
void ReverseCond(Condition& cond);
bool condCmp(const Condition& x, const Condition& y);
#endif