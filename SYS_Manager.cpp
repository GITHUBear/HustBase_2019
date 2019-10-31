#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>
#include <set>
#include <algorithm>

WorkSpace workSpace;

void ExecuteAndMessage(char * sql,CEditArea* editArea){//根据执行的语句类型在界面上显示执行结果。此函数需修改
	std::string s_sql = sql;
	if(s_sql.find("select") == 0){
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//将查询结果处理一下，整理成下面这种形式
		//调用editArea->ShowSelResult(col_num,row_num,fields,rows);
		int col_num = 5;
		int row_num = 3;
		char ** fields = new char *[5];
		for(int i = 0;i<col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],0,20);
			fields[i][0] = 'f';
			fields[i][1] = i+'0';
		}
		char *** rows = new char**[row_num];
		for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];
			for(int j = 0;j<col_num;j++){
				rows[i][j] = new char[20];
				memset(rows[i][j],0,20);
				rows[i][j][0] = 'r';
				rows[i][j][1] = i + '0';
				rows[i][j][2] = '+';
				rows[i][j][3] = j + '0';
			}
		}
		editArea->ShowSelResult(col_num,row_num,fields,rows);
		for(int i = 0;i<5;i++){
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	RC rc = execute(sql);
	int row_num = 0;
	char**messages;
	switch(rc){
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "操作成功";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "有语法错误";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "功能未实现";
		editArea->ShowMessage(row_num,messages);
	delete[] messages;
		break;
	}
}

RC execute(char * sql){
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
  	rc = parse(sql, sql_str);//只有两种返回结果SUCCESS和SQL_SYNTAX
	
	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////判断SQL语句为select语句

			//break;

			case 2:
			//判断SQL语句为insert语句

			case 3:	
			//判断SQL语句为update语句
			break;

			case 4:					
			//判断SQL语句为delete语句
			break;

			case 5:
			//判断SQL语句为createTable语句
			break;

			case 6:	
			//判断SQL语句为dropTable语句
			break;

			case 7:
			//判断SQL语句为createIndex语句
			break;
	
			case 8:	
			//判断SQL语句为dropIndex语句
			break;
			
			case 9:
			//判断为help语句，可以给出帮助提示
			break;
		
			case 10: 
			//判断为exit语句，可以由此进行退出操作
			break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//弹出警告框，sql语句词法解析错误信息
		return rc;
	}
}


//
// 目的：在路径 dbPath 下创建一个名为 dbName 的空库，生成相应的系统文件。
// 1. 向OS申请在dbpath路径创建文件夹。
// 2. 创建SYSTABLES文件和SYSCOLUMNS文件
// 
RC CreateDB(char *dbpath,char *dbname)
{
	RC rc;
	//1. 检查dbpath和dbname的合法性。
	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	if (workSpace.curWorkPath.size() && workSpace.curDBName.size())
		if (workSpace.curWorkPath.compare(dbpath) != 0 || 
			workSpace.curDBName.compare(dbname) != 0)
			if (rc = CloseDB())
				return rc;

	//3. 向OS申请在dbpath路径创建文件夹。
	std::string dbPath = dbpath;
	dbPath += "\\";
	dbPath += dbname;
	if (CreateDirectory(dbPath.c_str(), NULL)) {
		if (SetCurrentDirectory(dbpath)) {
			//4. 创建SYSTABLES文件和SYSCOLUMNS文件
			std::string sysTablePath = dbPath + TABLE_META_NAME;
			std::string sysColumnsPath = dbPath + COLUMN_META_NAME;
			if ((rc = RM_CreateFile((char*)sysTablePath.c_str(), TABLE_ENTRY_SIZE)) ||
				(rc = RM_CreateFile((char*)sysColumnsPath.c_str(), COL_ENTRY_SIZE)))
				return rc;

			//5. 设置dbPath和curDbName。
			workSpace.curWorkPath = dbpath;
			workSpace.curDBName = dbname;
			return SUCCESS;
		}
		return OSFAIL;
	}

	return OSFAIL;
}

//
// 目的：删除数据库对应的目录以及目录下的所有文件
//       由于不存在子文件夹，所以不需要进行递归处理
// 1. 判断删除的是否是当前DB。如果则要关闭当前目录.
// 2. 否则搜索得到dbname目录下的所有文件名并删除,注意要跳过.和..目录
// 3. 删除dbname目录
// 
RC DropDB(char *dbname)
{
	//1. 判断dbInfo.path路径下是否存在dbname。如果不存在则报错。
	std::string dbPath = workSpace.curWorkPath + "\\" + dbname;
	if (!workSpace.curWorkPath.size() || access(dbPath.c_str(), 0) == -1)
		return DB_NOT_EXIST;

	//2. 判断删除的是否是当前DB
	if (workSpace.curDBName.compare(dbname) == 0) {
		//关闭当前目录
		RC rc;
		if (rc = CloseDB())
			return rc;
	}

	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	dbPath += "\\*.*";
	hFile = FindFirstFile(dbPath.c_str(), &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return OSFAIL;

	//3. 遍历删除文件夹下的所有文件
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//跳过.和..目录
			continue;
		if (!DeleteFile(pNextInfo.cFileName))
			return OSFAIL;
	}

	//4. 删除dbname目录
	if (!RemoveDirectory(dbname))
		return OSFAIL;

	return SUCCESS;
}

//
// 目的：改变系统的当前数据库为 dbName 对应的文件夹中的数据库
// 1. 设置进程当前工作目录为dbnmae
// 2. 关闭当前打开的目录。
// 3. 打开SYSTABLES和SYSCOLUMNS，设置sysFileHandle_Vec
// 4. 设置curDbName
// 
RC OpenDB(char *dbname)
{
	RC rc;

	//1.  关闭当前打开的目录。打开SYSTABLES和SYSCOLUMNS，设置sysFileHandle_Vec
	if (workSpace.curDBName.size() && (rc = CloseDB()))
		return rc;

	std::string sysTablePath = dbname;
	sysTablePath += "\\";
	sysTablePath += TABLE_META_NAME;
	std::string sysColumnsPath = dbname;
	sysColumnsPath += "\\";
	sysColumnsPath += COLUMN_META_NAME;
	if ((rc = RM_OpenFile((char*)sysTablePath.c_str(), &(workSpace.sysTable))) ||
		(rc = RM_OpenFile((char*)sysColumnsPath.c_str(), &(workSpace.sysColumn))))
		return rc;

	//3. 设置curDbName
	workSpace.curDBName = dbname;

	return SUCCESS;
}

//
// 目的:关闭当前数据库。关闭当前数据库中打开的所有文件
// 1. 检查当前是否打开了DB。即检查保存的curDbName是否为空。
// 2. 调用保存句柄中对应的Close函数。
//	 2.1 关闭RM文件
//	 2.2 关闭IX文件
//	 2.3 关闭SYS文件
// 3. 切换到上级目录.
//
RC CloseDB()
{
	//1. 检查当前是否打开了DB。如果curDbName为空则报错，否则设置curDbName为空。
	if (!workSpace.curDBName.size())
		return NO_DB_OPENED;
	workSpace.curDBName.clear();

	//2. 调用保存的SYS文件句柄close函数，关闭SYS文件。
	RM_CloseFile(&(workSpace.sysTable));
	RM_CloseFile(&(workSpace.sysColumn));

	return SUCCESS;
}

bool CanButtonClick()
{
	// 如果当前有数据库已经打开
	// return true;
	// 如果当前没有数据库打开
	// return false;
	return (workSpace.curDBName.size() != 0);
}


bool attrVaild(int attrCount, AttrInfo* attributes)
{
	std::set<std::string> dupJudger;
	std::set<std::string>::iterator iter;

	for (int i = 0; i < attrCount; i++) {
		if (strlen(attributes[i].attrName) >= ATTRNAME)
			return false;

		std::string attrName(attributes[i].attrName);
		iter = dupJudger.find(attrName);

		if (iter != dupJudger.end())
			return false;

		dupJudger.insert(attrName);
	}

	return true;
}

//
// 目的:创建一个名为 relName 的表。
// 1. 检查当前是否打开了一个数据库。如果没有则报错。
// 2. 检查是否已经存在relName表。如果已经存在，则返回错误。
// 3. 检查atrrCount是否大于 MAXATTRS。
// 4. 向SYSTABLE和SYSCOLUMNS表中传入元信息
// 5. 计算记录的大小。创建对应的RM文件
// 
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) 
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[TABLE_ENTRY_SIZE];
	memset(rmRecord.pData, 0, TABLE_ENTRY_SIZE);

	int tableRecLength = 0;
	// 1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (!workSpace.curDBName.size()) {
		delete[] rmRecord.pData;
		return NO_DB_OPENED;
	}

	// 检查 relName 是否合法
	if (strlen(relName) >= TABLENAME ||
		!strcmp(relName, TABLE_META_NAME) ||
		!strcmp(relName, COLUMN_META_NAME)) {
		delete[] rmRecord.pData;
		return SQL_SYNTAX;
	}

	if ((rc = TableMetaSearch(relName, &rmRecord)) != TABLE_NOT_EXIST) {
		delete[] rmRecord.pData;
		return SQL_SYNTAX;
	}

	// 检查 attr 是否合法
	if (!attrVaild(attrCount, attributes)) {
		delete[] rmRecord.pData;
		return SQL_SYNTAX;
	}

	// 向 column 元数据 插入，并计算 该表的记录长度
	for (int i = 0; i < attrCount; i++) {
		if (rc = ColumnMetaInsert(relName, attributes[i].attrName, attributes[i].attrType,
			attributes[i].attrLength, tableRecLength, false, NULL)) {
			delete[] rmRecord.pData;
			return rc;
		}
		tableRecLength += attributes[i].attrLength;
	}

	// 向 table 元数据 插入
	if (rc = TableMetaInsert(relName, attrCount)) {
		delete[] rmRecord.pData;
		return rc;
	}

	// 创建 RM 文件
	std::string rmPath = workSpace.curDBName + "\\";
	rmPath += relName;
	rmPath += REC_FILE_SUFFIX;

	if (rc = RM_CreateFile((char*)(rmPath.c_str()), tableRecLength)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//目的：销毁名为 relName 的表以及在该表上建立的所有索引。
//1. 检查当前是否已经打开了一个数据库，如果没有则报错。
//2. 判断是否存在表relName.
//	 2.1. 如果已经存在则报错。
//	 3.2. 否则从SYSTABLES中删除relName对应的一项记录
//3. 删除rm文件。
//	 3.1.在dbInfo的rmFileHandle_Map中查找relName对应的rm文件是否被打开，如果已经打开则调用close函数，并从Map中删除。
//	 3.2.删除relName对应的rm文件
//4. 删除ix文件
//	 4.1.读取SYSCOLUMNS。得到relName的atrrname。
//	 4.2.判断是否创建了索引。如果创建了索引：
//		 4.2.1 在dbInfo的ixIndexHandle_Map中查找relName_atrrname对应的ix文件是否被打开，如果已经打开则调用close函数，并从Map中删除。
//		 4.2.2 删除relName_atrrname对应的ix文件
//	 4.3.删除SYSCOLUMNS中atrrname对应的记录。
RC DropTable(char* relName) 
{
	RC rc;

	// 1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (!workSpace.curDBName.size()) {
		return NO_DB_OPENED;
	}

	if (strlen(relName) >= TABLENAME ||
		!strcmp(relName, TABLE_META_NAME) ||
		!strcmp(relName, COLUMN_META_NAME)) {
		return SQL_SYNTAX;
	}

	if ((rc = ColumnMetaDelete(relName)) ||
		(rc = TableMetaDelete(relName)))
		return rc;

	return SUCCESS;
}

//该函数在关系 relName 的属性 attrName 上创建名为 indexName 的索引。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查是否已经存在对应的索引。如果已经存在则报错。
//3. 创建对应的索引。
//	 3.1. 读取relName,atrrName在SYSCOLUMNS中的记录项，得到attrType和attrLength
//	 3.2. 创建IX文件
//	 3.3. 更新SYS_COLUMNS
RC CreateIndex(char* indexName, char* relName, char* attrName) 
{
	if (strlen(indexName) >= INDEXNAME)
		return SQL_SYNTAX;

	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);

	// 1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (!workSpace.curDBName.size()) {
		delete[] rmRecord.pData;
		return NO_DB_OPENED;
	}

	// 判断表是否存在
	RM_Record tableRec;
	tableRec.pData = new char[TABLE_ENTRY_SIZE];
	memset(tableRec.pData, 0, TABLE_ENTRY_SIZE);
	if (rc = TableMetaSearch(relName, &tableRec)) {
		delete[] tableRec.pData;
		delete[] rmRecord.pData;
		return rc;
	}
	delete[] tableRec.pData;

	// 判断属性是否存在
	if (rc = ColumnSearchAttr(relName, attrName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	if (*(rmRecord.pData + IXFLAG_OFF) == (char)1) {
		delete[] rmRecord.pData;
		return INDEX_EXIST;
	}

	// 先创建 IX 文件，再更改


	char* pData;
	char ix_c = 1;
	pData = rmRecord.pData;
	*(pData + IXFLAG_OFF) = ix_c;
	memcpy(pData + INDEXNAME_OFF, indexName, strlen(indexName));

	if (rc = UpdateRec(&(workSpace.sysColumn), &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//目的：该函数用来删除名为 indexName 的索引。 
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 从SYSCOLUMNS中检查索引是否存在。（这里利用了我们约定的indexname具有唯一性来进行查找）
//	 2.1 如果不存在则报错。
//	 2.2.否则修改SYSCOLUMNS中对应记录项ix_flag为0.
//3. 删除ix文件
RC DropIndex(char* indexName) 
{
	// 1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (!workSpace.curDBName.size()) {
		return NO_DB_OPENED;
	}

	return SUCCESS;
}

RC Insert(char* relName, int nValues, Value* values)
{
	// 1. 尽量检查 relName 以及 values 的字符串是否长度符合标准
	// 2. bug: attrCount = *((int *)rmRecord.pData+ATTR_COUNT_OFF);
	return SUCCESS;
}

// 
// 向 table meta 插入记录
// 
RC TableMetaInsert(char* relName, int attrCount)
{
	RC rc;
	RID rid;

	char* factory = new char[TABLE_ENTRY_SIZE];
	memset(factory, 0, sizeof(TABLE_ENTRY_SIZE));

	// 装配 table meta 需要的数据
	memcpy(factory, relName, strlen(relName));
	memcpy(factory + ATTRCOUNT_OFF, &attrCount, sizeof(int));

	if (rc = InsertRec(&(workSpace.sysTable), factory, &rid)) {
		delete[] factory;
		return rc;
	}

	delete[] factory;
	return SUCCESS;
}

//
// 查找 table meta 中是否存在指定 relName 的记录
// 
RC TableMetaSearch(char* relName, RM_Record* rmRecord)
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[TABLE_ENTRY_SIZE];
	memset(rec.pData, 0, TABLE_ENTRY_SIZE);
	Con cond;

	if (strlen(relName) >= TABLENAME) {
		delete[] rec.pData;
		return TABLE_NAME_ILLEGAL;
	}

	cond.attrType = chars; cond.bLhsIsAttr = 1; cond.bRhsIsAttr = 0;
	cond.compOp = EQual; cond.LattrLength = TABLENAME; cond.LattrOffset = TABLENAME_OFF;
	cond.Lvalue = NULL; cond.RattrLength = TABLENAME; cond.RattrOffset = 0;
	cond.Rvalue = relName;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysTable), 1, &cond)) {
		delete[] rec.pData;
		return rc;
	}

	if ((rc = GetNextRec(&rmFileScan, &rec)) == RM_EOF) {
		delete[] rec.pData;
		return TABLE_NOT_EXIST;
	}
	if (rc != SUCCESS) {
		delete[] rec.pData;
		return rc;
	}

	memcpy(rmRecord->pData, rec.pData, TABLE_ENTRY_SIZE);
	rmRecord->rid = rec.rid;
	rmRecord->bValid = rec.bValid;

	if ((rc = GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		delete[] rec.pData;
		return FAIL;             // 来到这个分支可能存在严重逻辑错误
	}

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rec.pData;
		return rc;
	}

	delete[] rec.pData;
	return SUCCESS;
}

// 
// 在 table meta 里面删除指定 relName 的 记录
// 
RC TableMetaDelete(char* relName)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[TABLE_ENTRY_SIZE];
	memset(rmRecord.pData, 0, TABLE_ENTRY_SIZE);

	if (rc = TableMetaSearch(relName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	// 删除表的 rm 文件
	char tableName[21];
	memcpy(tableName, rmRecord.pData + TABLENAME_OFF, TABLENAME);
	std::string tablePath = workSpace.curDBName;
	tablePath += "\\";
	tablePath += tableName;
	tablePath += REC_FILE_SUFFIX;

	std::cout << "delete table record file: " << tablePath << std::endl;

	if (!DeleteFile(tablePath.c_str())) {
		delete[] rmRecord.pData;
		return OSFAIL;
	}

	if (rc = DeleteRec(&(workSpace.sysTable), &(rmRecord.rid))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC TableMetaShow()
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[TABLE_ENTRY_SIZE];
	memset(rec.pData, 0, TABLE_ENTRY_SIZE);
	int recnt = 0;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysTable), 0, NULL)) {
		delete[] rec.pData;
		return rc;
	}

	printf("Table Meta Table: \n");
	printf("+--------------------+----------+\n");
	printf("|     Table Name     |  AttrCnt |\n");
	printf("+--------------------+----------+\n");
	while ((rc == GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] rec.pData;
			return rc;
		}
		printf("|%-20s|%-10d|\n", rec.pData, *(int*)(rec.pData + ATTRCOUNT_OFF));
		recnt++;
	}
	printf("+--------------------+----------+\n");
	printf("Table Count: %d\n", recnt);

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rec.pData;
		return rc;
	}

	delete[] rec.pData;
	return SUCCESS;
}

// 插入一条 column 记录
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool idx, char* indexName)
{
	RC rc;
	RID rid;
	char idx_c = (idx) ? 1 : 0;
	
	char* factory = new char[COL_ENTRY_SIZE];
	memset(factory, 0, sizeof(COL_ENTRY_SIZE));

	// 装配 column meta 需要的数据
	memcpy(factory, relName, strlen(relName));
	memcpy(factory + ATTRNAME_OFF, attrName, strlen(attrName));
	memcpy(factory + ATTRTYPE_OFF, &attrType, sizeof(int));
	memcpy(factory + ATTRLENGTH_OFF, &attrLength, sizeof(int));
	memcpy(factory + ATTROFFSET_OFF, &attrOffset, sizeof(int));
	memcpy(factory + IXFLAG_OFF, &idx_c, sizeof(char));
	if (indexName)
		memcpy(factory + INDEXNAME_OFF, indexName, strlen(indexName));

	if (rc = InsertRec(&(workSpace.sysColumn), factory, &rid)) {
		delete[] factory;
		return rc;
	}

	delete[] factory;
	return SUCCESS;
}

RC ColumnSearchAttr(char* relName, char* attrName, RM_Record* rmRecord)
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[COL_ENTRY_SIZE];
	memset(rec.pData, 0, COL_ENTRY_SIZE);
	Con cond[2];

	if (strlen(relName) >= TABLENAME ||
		strlen(attrName) >= ATTRNAME) {
		delete[] rec.pData;
		return TABLE_NAME_ILLEGAL;
	}

	cond[0].attrType = chars; cond[0].bLhsIsAttr = 1; cond[0].bRhsIsAttr = 0;
	cond[0].compOp = EQual; cond[0].LattrLength = TABLENAME; cond[0].LattrOffset = TABLENAME_OFF;
	cond[0].Lvalue = NULL; cond[0].RattrLength = TABLENAME; cond[0].RattrOffset = 0;
	cond[0].Rvalue = relName;

	cond[1].attrType = chars; cond[1].bLhsIsAttr = 1; cond[1].bRhsIsAttr = 0;
	cond[1].compOp = EQual; cond[1].LattrLength = ATTRNAME; cond[1].LattrOffset = ATTRNAME_OFF;
	cond[1].Lvalue = NULL; cond[1].RattrLength = ATTRNAME; cond[1].RattrOffset = 0;
	cond[1].Rvalue = attrName;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysColumn), 2, cond)) {
		delete[] rec.pData;
		return rc;
	}

	if ((rc = GetNextRec(&rmFileScan, &rec)) == RM_EOF) {
		delete[] rec.pData;
		return FLIED_NOT_EXIST;
	}
	if (rc != SUCCESS) {
		delete[] rec.pData;
		return rc;
	}

	memcpy(rmRecord->pData, rec.pData, COL_ENTRY_SIZE);
	rmRecord->bValid = rec.bValid;
	rmRecord->rid = rec.rid;

	if ((rc = GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		delete[] rec.pData;
		return FAIL;             // 来到这个分支可能存在严重逻辑错误
	}

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rec.pData;
		return rc;
	}

	delete[] rec.pData;
	return SUCCESS;
}

RC ColumnMetaDelete(char* relName)
{
	// 检查 relName 表是否存在
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);
	RM_FileScan rmFileScan;
	Con cond;
	char idxName[21];
	char ix_c;

	if (rc = TableMetaSearch(relName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	cond.attrType = chars; cond.bLhsIsAttr = 1; cond.bRhsIsAttr = 0;
	cond.compOp = EQual; cond.LattrLength = TABLENAME; cond.LattrOffset = TABLENAME_OFF;
	cond.Lvalue = NULL; cond.RattrLength = TABLENAME; cond.RattrOffset = 0;
	cond.Rvalue = relName;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysColumn), 1, &cond)) {
		delete[] rmRecord.pData;
		return rc;
	}

	while ((rc = GetNextRec(&rmFileScan, &rmRecord)) != RM_EOF) {
		if (rc != SUCCESS) { // 发生错误
			delete[] rmRecord.pData;
			return rc;
		}

		// TODO: 删除 ix 文件
		ix_c = *(rmRecord.pData + IXFLAG_OFF);
		if (ix_c == 1) {
			memcpy(idxName, rmRecord.pData + INDEXNAME_OFF, INDEXNAME);
			std::string indexFilePath = workSpace.curDBName;
			indexFilePath += "\\";
			indexFilePath += idxName;
			indexFilePath += INDEX_FILE_SUFFIX;

			std::cout << "delete index file: " <<  indexFilePath << std::endl;

			if (!DeleteFile(indexFilePath.c_str())) {
				delete[] rmRecord.pData;
				return OSFAIL;
			}
		}

		if (rc = DeleteRec(&(workSpace.sysColumn), &(rmRecord.rid))) {
			delete[] rmRecord.pData;
			return rc;
		}
	}

	if (rc = CloseScan(&rmFileScan)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColumnMetaUpdate(char* relName, char* attrName, bool ixFlag, char* indexName)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);
	char* pData;
	char ix_c = (ixFlag) ? 1 : 0;

	if (rc = ColumnSearchAttr(relName, attrName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	pData = rmRecord.pData;
	*(pData + IXFLAG_OFF) = ix_c;
	if (indexName)
		memcpy(pData + INDEXNAME_OFF, indexName, strlen(indexName));

	if (rc = UpdateRec(&(workSpace.sysColumn), &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[COL_ENTRY_SIZE];
	memset(rmRecord.pData, 0, COL_ENTRY_SIZE);
	char* pData;
	
	assert(attrName);

	if (rc = ColumnSearchAttr(relName, attrName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	pData = rmRecord.pData;
	attribute->attrName = attrName;
	attribute->attrLength = *(int*)(pData + ATTRLENGTH_OFF);
	attribute->attrType = (AttrType)(*(int*)(pData + ATTRTYPE));

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColumnMetaShow()
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[COL_ENTRY_SIZE];
	memset(rec.pData, 0, COL_ENTRY_SIZE);
	int recnt = 0;

	if (rc = OpenScan(&rmFileScan, &(workSpace.sysTable), 0, NULL)) {
		delete[] rec.pData;
		return rc;
	}

	printf("Column Meta Table: \n");
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	printf("|     Table Name     ");
	printf("|   Attribute Name   ");
	printf("| Attr Type ");
	printf("|  AttrLen  ");
	printf("|Attr Offset");
	printf("|  idxFlag  ");
	printf("|     index Name     |\n");
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	while ((rc == GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] rec.pData;
			return rc;
		}
		printf("|%-20s|%-20s|%-10d|%-10d|%-10d|%-10d|%-20s|\n", rec.pData, rec.pData + ATTRNAME_OFF,
			                      *(int*)(rec.pData + ATTRTYPE_OFF), *(int*)(rec.pData + ATTRLENGTH_OFF), 
								  *(int*)(rec.pData + ATTROFFSET_OFF), (int)(*(rec.pData + IXFLAG_OFF)),
			                      rec.pData + INDEXNAME_OFF);
		recnt++;
	}
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	printf("Column Count: %d\n", recnt);

	delete[] rec.pData;
	return SUCCESS;
}

bool cmp(AttrEntry lattrEntry, AttrEntry rattrEntry)
{
	return lattrEntry.attrOffset < rattrEntry.attrOffset;
}

RC MetaGet(char* relName, int* attrCount, std::vector<AttrEntry>& attributes)
{
	RC rc;
	RM_Record tableRec;
	tableRec.pData = new char[TABLE_ENTRY_SIZE];
	memset(tableRec.pData, 0, TABLE_ENTRY_SIZE);

	if (rc = TableMetaSearch(relName, &tableRec)) {
		delete[] tableRec.pData;
		return rc;
	}

	*attrCount = *(int*)(tableRec.pData + ATTRCOUNT_OFF);
	delete[] tableRec.pData;

	RM_Record columnRec;
	columnRec.pData = new char[COL_ENTRY_SIZE];
	memset(columnRec.pData, 0, COL_ENTRY_SIZE);

	RM_FileScan columnScan;
	Con cond;

	cond.attrType = chars; cond.bLhsIsAttr = 1; cond.bRhsIsAttr = 0;
	cond.compOp = EQual; cond.LattrLength = TABLENAME; cond.LattrOffset = TABLENAME_OFF;
	cond.Lvalue = NULL; cond.RattrLength = TABLENAME; cond.RattrOffset = 0;
	cond.Rvalue = relName;

	if (rc = OpenScan(&columnScan, &(workSpace.sysColumn), 1, &cond)) {
		delete[] columnRec.pData;
		return rc;
	}
	
	AttrEntry attrEntry;

	while ((rc = GetNextRec(&columnScan, &columnRec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] columnRec.pData;
			return rc;
		}

		attrEntry.attrLength = *(int*)(columnRec.pData + ATTRLENGTH_OFF);
		attrEntry.attrType = (AttrType)(*(int*)(columnRec.pData + ATTRTYPE_OFF));
		attrEntry.attrOffset = *(int*)(columnRec.pData + ATTROFFSET_OFF);
		attrEntry.ix_flag = (*(columnRec.pData + IXFLAG_OFF) == (char)1);
		attrEntry.idxName = columnRec.pData + INDEXNAME_OFF;

		attributes.push_back(attrEntry);
	}

	if (rc = CloseScan(&columnScan)) {
		delete[] columnRec.pData;
		return rc;
	}

	assert(attributes.size() == *attrCount);

	sort(attributes.begin(), attributes.end(), cmp);

	delete[] columnRec.pData;
	return SUCCESS;
}