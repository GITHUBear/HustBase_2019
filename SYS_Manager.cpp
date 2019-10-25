#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>

//放在SYS_Manager.h中会发生冲突
typedef struct db_info {
	std::vector< RM_FileHandle* > sysFileHandle_Vec; //保存系统文件的句柄
	std::vector<RM_FileHandle*> rmFileHandle_Vec; //保存记录文件句柄
	std::vector<IX_IndexHandle*> ixIndexHandle_Vec; //保存索引文件的句柄

	int MAXATTRS=20;		 //最大属性数量
	char curDbName[300]=""; //存放当前DB名称
	char path[300]= "C:\C++\HBASE\DB";		 //存放所有DB公共的上级目录
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
RC CreateDB(char *dbpath,char *dbname){

	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	//TODO:
	//memset(dbInfo.curDbName,0,300);
	//memset(dbInfo.path,0,300);
	//strcpy(dbInfo.path,"C:\C++\HBASE\DB"); //设置公共路径信息

	char createPath[40];
	RC rc;
	memset(createPath, 0, 40);
	strcat(createPath, dbpath);
	strcat(createPath, "\\");
	strcat(createPath, dbname);
	
	if (CreateDirectory(createPath, NULL)) {
		if (SetCurrentDirectory(createPath)){
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
	int size = dbInfo.rmFileHandle_Vec.size();
	RM_FileHandle* rmFileHandle;
	for (int i = 0; i < size; i++) {
		rmFileHandle = dbInfo.rmFileHandle_Vec[i];
		RM_CloseFile(rmFileHandle);
		free(rmFileHandle);
	}
	dbInfo.rmFileHandle_Vec.clear();


	//	2.2 关闭IX文件
	size = dbInfo.ixIndexHandle_Vec.size();
	IX_IndexHandle* ixIndexHandle;
	for (int i = 0; i < size; i++) {
		ixIndexHandle = dbInfo.ixIndexHandle_Vec[i];
		CloseIndex(ixIndexHandle);
		free(ixIndexHandle);
	}
	dbInfo.ixIndexHandle_Vec.clear();
	
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
//1. 检查是否已经存在relName表。如果已经存在，则返回错误。
//2. 检查atrrCount是否大于 MAXATTRS。
//3. 向SYSTABLE和SYSCOLUMNS表中传入元信息
//4. 计算记录的大小。创建对应的RM文件
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) {
	//参数 attrCount 表示关系中属性的数量（取值为1 到 MAXATTRS 之间） 。 
	//参数 attributes 是一个长度为 attrCount 的数组。 对于新关系中第 i 个属性，
	//attributes 数组中的第 i 个元素包含名称、 类型和属性的长度（见 AttrInfo 结构定义）

	//1. 检查是否已经存在relName表
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

	//2. 检查atrrCount是否大于 MAXATTRS。
	if (attrCount > dbInfo.MAXATTRS)
		return SQL_SYNTAX;

	//3. 向SYSTABLE和SYSCOLUMNS表中传入元信息
	int rmRecordSize=0;//计算记录大小
	char sysData[25];
	int* sysData_attrcount;
	RID rid;
	//表名（tablename） 占 21 个字节， 即表名为最大长度为 20 的字符串。
	memset(sysData, 0, 25);
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

	//4. 计算记录的大小。创建对应的RM文件
	//根据约定，文件名为了relName.rm。
	char fileName[21];
	memset(fileName, 0, 21);
	strcat(fileName, relName);
	strcat(fileName, ".rm");
	RM_CreateFile(fileName,rmRecordSize);
}

//
//目的：销毁名为 relName 的表以及在该表上建立的所有索引。
//1. 判断是否存在表relName
//2. 在dbInfo的vector中查找relName对应的rm文件和ix文件是否已经打开.如果已经打开则调用close函数，并从vector中删除。
//3. 在当前目录下查找relName对应的rm文件和ix文件。（利用系统提供的正则匹配查找可以不用查看SYSCOLUMNS信息？）
RC DropTable(char* relName) {
	//1. 判断是否存在表relName
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[2];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = 21; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = 4; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	rmRecord.bValid = false;
	OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[0], 1, conditions);

	rc = GetNextRec(&rmFileScan, &rmRecord); //这里GetNextRec应该返回RM_EOF，因为无法识别RM_EOF，所以没有保存.但是两处SUCCESS的定义是一致的

	if (rc != SUCCESS || !rmRecord.bValid) {//已经存在relName表，判断bValid只是为了确保正确
		return SQL_SYNTAX;
	}
	CloseScan(&rmFileScan);

	//2. 在dbInfo的vector中查找relName对应的rm文件和ix文件是否已经打开.
	//查找rm文件句柄
	RM_FileHandle *rmFileHandle;
	for (int i = 0; i < dbInfo.rmFileHandle_Vec.size(); i++) {
		rmFileHandle = dbInfo.rmFileHandle_Vec[i];
		//如果已经打开则调用close函数，并从vector中删除。
		if (strcmp(relName, rmFileHandle->pfFileHandle.fileName) == 0) {
			RM_CloseFile(rmFileHandle);
			free(rmFileHandle);
			dbInfo.rmFileHandle_Vec.erase(dbInfo.rmFileHandle_Vec.begin()+i);
			i--;
		}
	}
	//查找ix文件句柄
	char str_one[1];
	str_one[0] = 1;//为了方便查出ix_flag为1的项
	conditions[1].attrType = chars; conditions[1].bLhsIsAttr = 1; conditions[1].bRhsIsAttr = 0;
	conditions[1].compOp = EQual; conditions[1].LattrLength = 1; conditions[1].LattrOffset = 21 + 21 + 4 + 4 + 4;
	conditions[1].Lvalue = NULL; conditions[1].RattrLength = 1; conditions[1].RattrOffset = 0;
	conditions[1].Rvalue = str_one;
	OpenScan(&rmFileScan, dbInfo.sysFileHandle_Vec[1], 2, conditions);//在SYSCOLUMNS文件中查找建立了的索引项。
	
}