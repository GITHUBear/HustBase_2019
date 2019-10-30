#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>

WorkSpace workSpace;

//放在SYS_Manager.h中会发生冲突
typedef struct db_info {
	std::vector< RM_FileHandle* > sysFileHandle_Vec; //保存系统文件的句柄
	std::map<std::string, RM_FileHandle*> rmFileHandle_Map; //从文件名映射到对应的记录文件句柄
	std::map<std::string, IX_IndexHandle*> ixIndexHandle_Map; //从文件名映射到对应的索引文件句柄

	int MAXATTRS = 20;		 //最大属性数量
	char curDbName[300] = ""; //存放当前DB名称
	char path[300] = "C:\C++\HBASE\DB";		 //存放所有DB公共的上级目录
}DB_INFO;
DB_INFO dbInfo;

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
//目的：在路径 dbPath 下创建一个名为 dbName 的空库，生成相应的系统文件。
//1. 向OS申请在dbpath路径创建文件夹。
//2. 创建SYSTABLES文件和SYSCOLUMNS文件
RC CreateDB(char *dbpath,char *dbname)
{
	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	if (strcmp(dbpath,dbInfo.path))//如果传入的参数和默认的参数不一致则报错
		return SQL_SYNTAX;

	char createPath[40];
	RC rc;
	memset(createPath, 0, 40);
	strcat(createPath, dbpath);
	strcat(createPath, "\\");
	strcat(createPath, dbname);
	
	//1. 向OS申请在dbpath路径创建文件夹。
	if (CreateDirectory(createPath, NULL)) {
		if (SetCurrentDirectory(createPath)){
			//2. 创建SYSTABLES文件和SYSCOLUMNS文件
			//SYSTABLES存放记录:tablename atrrcount 最多25个字节
			//SYSCOLUMNS存放记录：tablename attrname attrtype attrlength attroffset ix_flag indexname，最多76个字节
			if ((rc= RM_CreateFile("SYSTABLES", 25)) ||  (rc= RM_CreateFile("SYSCOLUMNS", 76)))
				return rc;
			return SUCCESS;
		}
		return SQL_SYNTAX;
	}
	//本来返回其他错误的。发现只有这有这两种？
	return SQL_SYNTAX;
}


//
//目的：删除数据库对应的目录以及目录下的所有文件
//      由于不存在子文件夹，所以不需要进行递归处理
//1. 判断删除的是否是当前DB。如果则要关闭当前目录.
//2. 否则搜索得到dbname目录下的所有文件名并删除,注意要跳过.和..目录
//3. 删除dbname目录

RC DropDB(char *dbname){

	//1. 判断删除的是否是当前DB
	if (!strcmp(dbInfo.curDbName, dbname)) {
		//关闭当前目录
		CloseDB();
	}

	char dbPath[300], fileName[300];
	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	//设置路径
	memset(dbPath, 0, 300);
	strcat(dbPath, dbInfo.path);
	strcat(dbPath, dbname);
	strcat(dbPath, "\\*.*");

	hFile = FindFirstFile((LPCTSTR)dbPath, &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return SQL_SYNTAX;

	//2. 遍历删除文件夹下的所有文件
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//跳过.和..目录
			continue;
		memset(fileName, 0, 300);
		strcat(fileName, dbInfo.path);
		strcat(fileName, dbname);
		strcat(fileName, "\\");
		strcat(fileName, pNextInfo.cFileName);
		DeleteFile(fileName);
	}

	//3. 删除dbname目录
	if (!RemoveDirectory(dbname))
		return SQL_SYNTAX;


	return SUCCESS;
}

//
//目的：改变系统的当前数据库为 dbName 对应的文件夹中的数据库
//1. 设置进程当前工作目录为dbnmae
//2. 关闭当前打开的目录。
//3. 打开SYSTABLES和SYSCOLUMNS，设置sysFileHandle_Vec
//4. 设置curDbName
RC OpenDB(char *dbname){

	RC rc;
	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;
	char dbPath[300];

	//1. 设置进程当前工作目录为dbnmae
	if (!SetCurrentDirectory(dbInfo.path) || !SetCurrentDirectory(dbname))
		return SQL_SYNTAX;

	//2.  关闭当前打开的目录。打开SYSTABLES和SYSCOLUMNS，设置sysFileHandle_Vec
	CloseDB();

	//3. 打开SYSTABLES和SYSCOLUMNS，设置sysFileHandle_Vec
	RM_FileHandle* sysTables, * sysColumns;
	sysTables = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	sysColumns = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));

	if ((rc = RM_OpenFile("SYSTABLES", sysTables)) || (rc = RM_OpenFile("SYSCOLUMNS", sysColumns)))
		return SQL_SYNTAX;
	
	dbInfo.sysFileHandle_Vec.push_back(sysTables);
	dbInfo.sysFileHandle_Vec.push_back(sysColumns);

	//4. 设置curDbName
	memset(dbInfo.curDbName, 0, 300);
	strcpy(dbInfo.curDbName, dbname);
	
	return SUCCESS;
}



//
//目的:关闭当前数据库。关闭当前数据库中打开的所有文件
//1. 检查当前是否打开了DB。即检查保存的curDbName是否为空。
//2. 调用保存句柄中对应的Close函数。
//	2.1 关闭RM文件
//	2.2 关闭IX文件
//	2.3 关闭SYS文件
//3. 切换到上级目录.
RC CloseDB(){
	//1. 检查当前是否打开了DB。即检查保存的curDbName是否为空。
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. 调用保存句柄中对应的Close函数。

	//	2.1 关闭RM文件
	std::map<std::string, RM_FileHandle*>::iterator rmIter;
	RM_FileHandle* rmFileHandle;
	for (rmIter = dbInfo.rmFileHandle_Map.begin(); rmIter != dbInfo.rmFileHandle_Map.end(); rmIter++) {
		rmFileHandle = rmIter->second;
		RM_CloseFile(rmFileHandle);
		free(rmFileHandle);
	}
	dbInfo.rmFileHandle_Map.clear();


	//	2.2 关闭IX文件
	std::map<std::string, IX_IndexHandle*>::iterator ixIter;
	IX_IndexHandle* ixIndexHandle;
	for (ixIter = dbInfo.ixIndexHandle_Map.begin(); ixIter != dbInfo.ixIndexHandle_Map.end(); ixIter++) {
		ixIndexHandle = ixIter->second;
		CloseIndex(ixIndexHandle);
		free(ixIndexHandle);
	}
	dbInfo.ixIndexHandle_Map.clear();
	
	//	2.3 关闭SYS文件
	RM_CloseFile(dbInfo.sysFileHandle_Vec[0]);
	RM_CloseFile(dbInfo.sysFileHandle_Vec[1]);
	free(dbInfo.sysFileHandle_Vec[0]);
	free(dbInfo.sysFileHandle_Vec[1]);
	dbInfo.sysFileHandle_Vec.clear();


	//3. 切换到上级目录.
	if (!SetCurrentDirectory(dbInfo.path))
		return SQL_SYNTAX;

	return SUCCESS;
}

bool CanButtonClick(){//需要重新实现
	//如果当前有数据库已经打开
	return true;
	//如果当前没有数据库打开
	//return false;
}


//
//目的:创建一个名为 relName 的表。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查是否已经存在relName表。如果已经存在，则返回错误。
//3. 检查atrrCount是否大于 MAXATTRS。
//4. 向SYSTABLE和SYSCOLUMNS表中传入元信息
//5. 计算记录的大小。创建对应的RM文件
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) {
	//参数 attrCount 表示关系中属性的数量（取值为1 到 MAXATTRS 之间） 。 
	//参数 attributes 是一个长度为 attrCount 的数组。 对于新关系中第 i 个属性，
	//attributes 数组中的第 i 个元素包含名称、 类型和属性的长度（见 AttrInfo 结构定义）

	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

	//2. 检查是否已经存在relName表
	//因为没有直接的扫描函数，所以只能通过RM的Scan函数来完成扫描
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[1];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 21; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[0], 1, conditions); 

	rmRecord.bValid = false;
	rc = GetNextRec(&rmFileScan,&rmRecord); //这里GetNextRec应该返回RM_EOF，因为无法识别RM_EOF，所以没有保存.但是两处SUCCESS的定义是一致的

	if (rc==SUCCESS || rmRecord.bValid ) {//已经存在relName表，判断bValid只是为了确保正确
		return SQL_SYNTAX;
	}
	CloseScan(&rmFileScan);

	//3. 检查atrrCount是否大于 MAXATTRS。
	if (attrCount > dbInfo.MAXATTRS)
		return SQL_SYNTAX;

	//4. 向SYSTABLE和SYSCOLUMNS表中传入元信息
	int rmRecordSize=0;//计算记录大小
	char sysData[76];
	int* sysData_attrcount;
	RID rid;
	//表名（tablename） 占 21 个字节， 即表名为最大长度为 20 的字符串。
	memset(sysData, 0, 76);
	strncpy(sysData, relName, 20);
	//属性的数量（attrcount） 为 int 类型， 占 4 个字节。
	sysData_attrcount = (int*)(sysData + 21);
	*sysData_attrcount = attrCount;
	
	if (rc = InsertRec(dbInfo.sysFileHandle_Vec[0], sysData, &rid))//插入失败
		//这里直接返回没有做错误处理其实是有问题的，但是为了降低复杂度暂时这样实现了
		return SQL_SYNTAX;

	//向SYSCOLUMNS表插入数据。
	//SYSCOLUMNS:tablename attrname attrtype attrlength attroffset ix_flag indexname
	for (int i = 0; i < attrCount; i++) {
		strncpy(sysData + 21, attributes[i].attrName, 20);//表名（tablename） 与属性名（attrname） 各占 21 个字节
		//attrtype int类型		
		sysData_attrcount = (int*)(sysData + 21+21);// 属性的类型（attrtype） 为 int 类型， 占 4 个字节
		*sysData_attrcount = attributes[i].attrType;
		//atrrlength int类型
		sysData_attrcount = (int*)(sysData + 21 + 21+4);// 属性的长度（attrlength） 为 int 类型，各占 4 个字节
		*sysData_attrcount = attributes[i].attrLength;
		rmRecordSize+= attributes[i].attrLength;
		//atrroffset int类型
		sysData_attrcount = (int*)(sysData + 21 + 21 + 4 + 4);// 记录中的偏移量（atrroffset） 为 int 类型，各占 4 个字节
		*sysData_attrcount = i;
		//ix_flag bool类型
		sysData_attrcount = (int*)(sysData + 21 + 21 + 4 + 4 + 4);//该属性列上是否存在索引的标识(ix_flag)占 1 个字节
		*sysData_attrcount = 0;									//没有主键概率，初始都没有索引
		//indexname string类型
		sysData_attrcount += 1;
		memset(sysData_attrcount, 0, 21);					//索引的名称(indexname)占 21 个字节

		if (rc = InsertRec(dbInfo.sysFileHandle_Vec[1], sysData, &rid))//插入失败
			return SQL_SYNTAX;
	}

	//5. 计算记录的大小。创建对应的RM文件
	//根据约定，文件名为了relName.rm。
	char fileName[24];//21+".rm"
	memset(fileName, 0, 24);
	strcat(fileName, relName);
	strcat(fileName, ".rm");
	RM_CreateFile(fileName,rmRecordSize);
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
RC DropTable(char* relName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

	//2. 判断是否存在表relName
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[2];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 21; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[0], 1, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	//	 2.1. 如果已经存在则报错。
	if (rc != SUCCESS || !rmRecord.bValid) {//已经存在relName表，判断bValid只是为了确保正确
		return SQL_SYNTAX;
	}
	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. 否则从SYSTABLES中删除relName对应的一项记录
	DeleteRec(dbInfo.sysFileHandle_Vec[0],&rmRecord.rid);

	//3. 删除rm文件。
	//	 3.1.在dbInfo的rmFileHandle_Map中查找relName对应的rm文件是否被打开
	std::map<std::string, RM_FileHandle*>::iterator rmIter;
	RM_FileHandle* rmFileHandle;
	std::string rmFileName="";
	rmFileName += relName;
	rmFileName += ".rm";
	rmIter = dbInfo.rmFileHandle_Map.find(rmFileName);
	if (rmIter != dbInfo.rmFileHandle_Map.end()) {
		//如果已经打开则调用close函数，并从Map中删除。
		if(RM_CloseFile(rmIter->second))
			return SQL_SYNTAX;
		free(rmIter->second);
		dbInfo.rmFileHandle_Map.erase(rmIter);
	}
	//	 3.2.删除relName对应的rm文件
	DeleteFile(rmFileName.c_str());

	//4. 删除ix文件

	//	 4.1.读取SYSCOLUMNS。得到relName的atrrname。
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 1, conditions)) {//在SYSCOLUMNS文件中查找relName表的属性
		return SQL_SYNTAX;
	}

	std::map<std::string, IX_IndexHandle*>::iterator ixIter;
	IX_IndexHandle* ixIndexHandle;
	std::string ixFileName = "";
	while (!GetNextRec(&rmFileScan, &rmRecord)) { //无法区分RM_EOF和其他错误，所以无法做错误处理
		//	 4.2.判断是否创建了索引。如果创建了索引：
		if ((*(rmRecord.pData + 21 + 21 + 4 + 4 + 4)) == (char)1) {
			//		 4.2.1 在dbInfo的ixIndexHandle_Map中查找relName_atrrname对应的ix文件是否被打开
			ixFileName += rmRecord.pData + 21 + 21 + 4 + 4 + 4 + 1;
			ixIter = dbInfo.ixIndexHandle_Map.find(ixFileName);
			if (ixIter != dbInfo.ixIndexHandle_Map.end()) {
				//如果已经打开则调用close函数，并从Map中删除
				if (CloseIndex(ixIter->second))
					return SQL_SYNTAX;
				free(ixIter->second);
				dbInfo.ixIndexHandle_Map.erase(ixIter);
			}
			//		 4.2.2 删除relName_atrrname对应的ix文件
			DeleteFile(ixFileName.c_str());
		}
		//	 4.3.删除SYSCOLUMNS中atrrname对应的记录。
		DeleteRec(dbInfo.sysFileHandle_Vec[1],&rmRecord.rid);
	}
	
	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	return SUCCESS;
}

//该函数在关系 relName 的属性 attrName 上创建名为 indexName 的索引。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查是否已经存在对应的索引。如果已经存在则报错。
//3. 创建对应的索引。
//	 3.1. 读取relName,atrrName在SYSCOLUMNS中的记录项，得到attrType和attrLength
//	 3.2. 创建IX文件
//	 3.3. 更新SYS_COLUMNS
RC CreateIndex(char* indexName, char* relName, char* attrName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (strcmp(dbInfo.curDbName, "") == 0)
		return SQL_SYNTAX;

	char ixFileName[45];//relname+"_"+attrName+".ix"-1
	memset(ixFileName, 0, 42);
	strcat(ixFileName,relName);
	strcat(ixFileName, "_");
	strcat(ixFileName, attrName);
	strcat(ixFileName, ".ix");
	
	if (strcmp(indexName, ixFileName)) {//检查indexName和约定的是否一致。
		return SQL_SYNTAX;
	}

	//2. 检查是否已经存在对应的索引。如果已经存在则报错。
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[3];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 21; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;

	conditions[1].attrType = chars; conditions[1].bLhsIsAttr = 1; conditions[1].bRhsIsAttr = 0;
	conditions[1].compOp = EQual; conditions[1].LattrLength = 21; conditions[1].LattrOffset = 21;
	conditions[1].Lvalue = NULL; conditions[1].RattrLength = 21; conditions[1].RattrOffset = 0;
	conditions[1].Rvalue = attrName;

	char str_one[1];//因为ix_flag是一个字节的标识位，所以用chars的方式来比较。
	str_one[0]=1;
	conditions[2].attrType = chars; conditions[2].bLhsIsAttr = 1; conditions[2].bRhsIsAttr = 0;
	conditions[2].compOp = EQual; conditions[2].LattrLength = 1; conditions[2].LattrOffset = + 21 + 21 + 4 + 4 + 4+1;
	conditions[2].Lvalue = NULL; conditions[2].RattrLength = 1; conditions[2].RattrOffset = 0;
	conditions[2].Rvalue = str_one;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 3, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	if (rc == SUCCESS || rmRecord.bValid) {//已经创建了对应的索引
		return SQL_SYNTAX;
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. 创建对应的索引。
	//	 3.1. 读取relName,atrrName在SYSCOLUMNS中的记录项，得到atrrType
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 2, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	if (rc != SUCCESS || !rmRecord.bValid)
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 3.2. 创建IX文件
	int attrType = *((int*)rmRecord.pData + 21 + 21);
	int attrLength = *((int*)rmRecord.pData + 21 + 21 + 4);
	char* ix_Flag = (char*)rmRecord.pData + 21 + 21 + 4 + 4 + 4;
	if (ix_Flag != 0)//检查ix_flag等于0
		return SQL_SYNTAX;
	//RC CreateIndex(const char * fileName,AttrType attrType,int attrLength);
	if(CreateIndex(ixFileName,(AttrType)attrType,attrLength))
		return SQL_SYNTAX;

	//	 3.3. 更新SYS_COLUMNS
	*ix_Flag = 1;
	RM_FileHandle* rmFileHandle;
	char rmFileName[24];//relName+".rm"
	memset(rmFileName, 0, 24);
	strcat(rmFileName, relName);
	strcat(rmFileName, ".rm");
	if(	RM_OpenFile(rmFileName,rmFileHandle)||
		UpdateRec(rmFileHandle,&rmRecord)||
		RM_CloseFile(rmFileHandle))
		return SQL_SYNTAX;
	
}

//
//目的：该函数用来删除名为 indexName 的索引。 
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 从SYSCOLUMNS中检查索引是否存在。（这里利用了我们约定的indexname具有唯一性来进行查找）
//	 2.1 如果不存在则报错。
//	 2.2.否则修改SYSCOLUMNS中对应记录项ix_flag为0.
//3. 删除ix文件
RC DropIndex(char* indexName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
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
		return TABLE_NOT_EXIST;
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

