#ifndef SYS_MANAGER_H_H
#define SYS_MANAGER_H_H

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "str.h"
#include <string>
#include <map>
#include <cassert>
#include <vector>

#define TABLE_META_NAME "SYSTABLES.rm"
#define COLUMN_META_NAME "SYSCOLUMNS.rm"

#define INDEX_FILE_SUFFIX ".ix"
#define REC_FILE_SUFFIX ".rm"

// table 元数据表字段长度
const int TABLENAME = 21;
const int ATTRCOUNT = sizeof(int);
// table 元数据各个字段偏移
const int TABLENAME_OFF = 0;
const int ATTRCOUNT_OFF = TABLENAME_OFF + TABLENAME;
const int TABLE_ENTRY_SIZE = TABLENAME + ATTRCOUNT;

// column 元数据表字段长度
// const int TABLENAME = 21;
const int ATTRNAME = 21;
const int ATTRTYPE = sizeof(int);
const int ATTRLENGTH = sizeof(int);
const int ATTROFFSET = sizeof(int);
const int IXFLAG = sizeof(char);
const int INDEXNAME = 21;
// column 元数据表各个字段偏移
// const int TABLENAME_OFF = 0;
const int ATTRNAME_OFF = TABLENAME_OFF + TABLENAME;
const int ATTRTYPE_OFF = ATTRNAME_OFF + ATTRNAME;
const int ATTRLENGTH_OFF = ATTRTYPE_OFF + ATTRTYPE;
const int ATTROFFSET_OFF = ATTRLENGTH_OFF + ATTRLENGTH;
const int IXFLAG_OFF = ATTROFFSET_OFF + ATTROFFSET;
const int INDEXNAME_OFF = IXFLAG_OFF + IXFLAG;
const int COL_ENTRY_SIZE = TABLENAME + ATTRNAME + ATTRTYPE + ATTRLENGTH + ATTROFFSET + IXFLAG + INDEXNAME;

typedef struct {
	RM_FileHandle sysTable;
	RM_FileHandle sysColumn;

	std::string curWorkPath;        // DB 文件夹上层目录路径
	std::string curDBPath;
} WorkSpace;

void ExecuteAndMessage(char * ,CEditArea*);
bool CanButtonClick();

RC CreateDB(char *dbpath,char *dbname);
RC DropDB(char *dbname);
RC OpenDB(char *dbname);
RC CloseDB();

RC execute(char * sql);

RC CreateTable(char *relName,int attrCount,AttrInfo *attributes);
RC DropTable(char *relName);
RC CreateIndex(char *indexName,char *relName,char *attrName);
RC DropIndex(char *indexName);
RC Insert(char *relName,int nValues,Value * values);
RC Delete(char *relName,int nConditions,Condition *conditions);
RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions);

// 一些非接口方法
// table 元数据表操作
//
// relName: 表名称
// attrCount: 属性数量
// RC 由底层 RM insert 操作进行返回
// 
RC TableMetaInsert(char* relName, int attrCount);                    // 插入一条 table name 记录
//
// relName: 表名称
// 判断 relName 是否存在, 不存在返回 TABLE_NOT_EXIST
// 其次 RC 由底层 RM Scan/delete 操作进行返回
// 底层实现保证 边查边删 应该是安全的
// 
RC TableMetaDelete(char* relName);                                   // 删除一个 table name 记录
//
// relName: 表名称
// 判断 relName 是否存在, 不存在返回 TABLE_NOT_EXIST
// 存在 返回 SUCCESS
// 
RC TableMetaSearch(char* relName, RM_Record* rmRecord);                         // 判断一个 table name 是否存在
// RC 代表 RM Scan 过程中发生的错误 (除去 RM_EOF)
// 完成 Scan 返回 Success
// 输出格式尽量好看些，仿照 mysql 是最吼的
// 
RC TableMetaShow();                                                  // 打印出 table 元数据表

// column 元数据表操作
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool idx, char* indexName);      // 插入一条 column 记录
// 检查 relName 有效，同上
RC ColumnMetaDelete(char* relName);                                  // 删除同一个表中的所有属性列信息
                                                                     // RM 的实现允许边查边删
RC ColumnSearchAttr(char* relName, char* attrName, RM_Record* rmRecord);
// 检查 relName 有效，同上
RC ColumnMetaUpdate(char* relName, char* attrName,
	bool ixFlag, char* indexName);                                   // 更新 column 元数据表中的某一项记录的相关属性 
// 检查 relName 有效，同上
// 获取的信息保存在 attribute 中
RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute);// 获取某个 表 中的 某一个属性的 类型、长度
// 同上
RC ColumnMetaShow();                                                 // 打印出 column 元数据表

// 检查 relName 有效，同上
// 获取的信息保存在 attribute 中
RC MetaGet(char* relName, int attrCount, AttrInfo* attributes);      // 通过 table 元数据 和 column 元数据
                                                                     // 获得一个表的全部属性信息, 便于进行类型检查
                                                                     // CreateTable 的 逆操作

#endif