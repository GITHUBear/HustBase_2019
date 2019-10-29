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

#define TABLE_META_NAME "SYSTABLES"
#define COLUMN_META_NAME "SYSCOLUMNS"
#define RM_FILE_SUFFIX ".rm"
#define IX_FILE_SUFFIX ".ix"
//SYSTABLE和SYSCOLUMNS中记录每一项的长度
#define SIZE_TABLE_NAME 21
#define SIZE_ATTR_COUNT 4
#define SIZE_ATTR_NAME 21
#define SIZE_ATTR_TYPE 4
#define SIZE_ATTR_LENGTH 4
#define SIZE_ATTR_OFFSET 4
#define SIZE_IX_FLAG 1
#define SIZE_INDEX_NAME 21
#define SIZE_SYS_TABLE 25
#define SIZE_SYS_COLUMNS 76
#define MAX_CON_LEN 100 //用来声明局部数组

typedef struct db_info {
	RM_FileHandle* sysTables;
	RM_FileHandle* sysColumns;
	int MAXATTRS=20;		 //最大属性数量
	std::string curDbName; //存放当前DB名称
	std::string path;	 //调用CreateDB传入的路径

}DB_INFO;
DB_INFO dbInfo;

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
// SYSTABLES 元数据表操作
RC TableMetaInsert(char* relName, int attrCount); 
RC TableMetaDelete(char* relName);
RC TableMetaSearch(char* relName, RM_Record& rmRecord);
RC TableMetaShow();

// SYSCOLUMNS 元数据表操作
RC ColumnSearchAttr(char* relName, char* attrName);      
RC ToData(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName, char* pData);
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName);     
RC ColumnMetaDelete(char* relName);                                  
RC ColumnMetaUpdate(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName);      
RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute);
RC ColumnMetaShow();                                                 // 打印出 column 元数据表
// 检查 relName 有效，同上
// 获取的信息保存在 attribute 中
RC MetaGet(char* relName, int attrCount, AttrInfo* attributes);      // 通过 table 元数据 和 column 元数据
                                                                     // 获得一个表的全部属性信息, 便于进行类型检查
                                                                     // CreateTable 的 逆操作

#endif