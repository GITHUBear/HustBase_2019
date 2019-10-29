#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>


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
//1. 检查dbpath和dbname的合法性。
//2. 判断dbpath以及dbname是否和dbInfo中保存相同。如果不同则调用CloseDb，并设置dbInfo。
//3. 向OS申请在dbpath路径创建文件夹。
//4. 创建SYSTABLES文件和SYSCOLUMNS文件。
//5. 设置dbPath和curDbName。
RC CreateDB(char *dbpath,char *dbname){

	//1. 检查dbpath和dbname的合法性。
	if (dbpath == NULL || dbname == NULL)
		return SQL_SYNTAX;

	//2. 判断dbpath以及dbname是否和dbInfo中保存相同。如果不同则调用CloseDb。
	if ( dbInfo.path.size() && dbInfo.curDbName.size())
		if(dbInfo.path.compare(dbpath) != 0 || dbInfo.curDbName.compare(dbname) != 0)
			if(CloseDB())
				return SQL_SYNTAX;

	RC rc;
	
	//3. 向OS申请在dbpath路径创建文件夹。
	std::string dbPath = dbpath;
	dbPath += "\\";
	dbPath +=dbname;
	if (CreateDirectory(dbPath.c_str(), NULL)) {
		if (SetCurrentDirectory(dbpath)){
			//4. 创建SYSTABLES文件和SYSCOLUMNS文件
			std::string sysTablePath = dbPath + "\\SYSTABLES";
			std::string sysColumnsPath = dbPath + "\\SYSCOLUMNS";
			if ((rc= RM_CreateFile((char *)sysTablePath.c_str(), SIZE_SYS_TABLE)) ||  (rc= RM_CreateFile((char *)sysColumnsPath.c_str(), SIZE_SYS_COLUMNS)))

				return rc;
			//5. 设置dbPath和curDbName。
			dbInfo.path = dbpath;
			dbInfo.curDbName = dbname;
			return SUCCESS;
		}
		return SQL_SYNTAX;
	}
	//本来返回其他错误的。发现只有这有这两种？
	return SQL_SYNTAX;
}


//
//目的：删除数据库对应的目录以及目录下的所有文件
//      默认不存在子文件夹，所以不进行递归处理
//1. 判断dbInfo.path是否为空，或者dbInfo.path下是否存在dbname。如果不存在则报错。
//2. 判断删除的是否是当前DB。如果则要关闭当前目录.
//3. 搜索得到dbname目录下的所有文件名并删除,注意要跳过.和..目录
//4. 删除dbname目录
RC DropDB(char *dbname){

	//1. 判断dbInfo.path路径下是否存在dbname。如果不存在则报错。
	std::string dbPath = dbInfo.path+"\\"+dbname;
	if (!dbInfo.path.size() || access(dbPath.c_str(),0)==-1)
		return SQL_SYNTAX;

	//2. 判断删除的是否是当前DB
	if (dbInfo.curDbName.compare(dbname)==0) {
		//关闭当前目录
		CloseDB();
	}

	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	dbPath += "\\*.*";
	hFile = FindFirstFile(dbPath.c_str(), &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return SQL_SYNTAX;

	//3. 遍历删除文件夹下的所有文件
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//跳过.和..目录
			continue;
		if(!DeleteFile(pNextInfo.cFileName))
			return SQL_SYNTAX;
	}

	//4. 删除dbname目录
	if (!RemoveDirectory(dbname))
		return SQL_SYNTAX;

	return SUCCESS;
}

//
//目的：改变系统的当前数据库为 dbName 对应的文件夹中的数据库
//1. 如果当前打开了db，则关闭当前打开的目录。
//2. 打开SYSTABLES和SYSCOLUMNS,设置dbInfo
//3. 设置curDbName
RC OpenDB(char *dbname){

	RC rc;

	//1.  关闭当前打开的目录。打开SYSTABLES和SYSCOLUMNS，设置sysFileHandle_Vec
	if (dbInfo.curDbName.size() && CloseDB())
		return SQL_SYNTAX;

	//2. 打开SYSTABLES和SYSCOLUMNS,设置dbInfo
	RM_FileHandle* sysTables, * sysColumns;
	sysTables = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	sysColumns = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));

	std::string sysTablePath = dbname;
	sysTablePath+="\\SYSTABLES";
	std::string sysColumnsPath = dbname;
	sysColumnsPath+="\\SYSCOLUMNS";
	if ((rc = RM_OpenFile((char *)sysTablePath.c_str(), sysTables)) || (rc = RM_OpenFile((char*)sysColumnsPath.c_str(), sysColumns)))
		return SQL_SYNTAX;
	
	dbInfo.sysTables=sysTables;
	dbInfo.sysColumns=sysColumns;

	//3. 设置curDbName
	dbInfo.curDbName = dbname;
	
	return SUCCESS;
}



//
//目的:关闭当前数据库。关闭当前数据库中打开的所有文件
//1. 检查当前是否打开了DB。如果curDbName为空则报错，否则设置curDbName为空。
//2. 调用保存的SYS文件句柄close函数，关闭SYS文件。
RC CloseDB(){
	//1. 检查当前是否打开了DB。如果curDbName为空则报错，否则设置curDbName为空。
	if (!dbInfo.curDbName.size())
		return SQL_SYNTAX;
	dbInfo.curDbName.clear();

	//2. 调用保存的SYS文件句柄close函数，关闭SYS文件。
	RM_CloseFile(dbInfo.sysTables);
	RM_CloseFile(dbInfo.sysColumns);
	free(dbInfo.sysTables);
	free(dbInfo.sysColumns);
	
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
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. 检查是否已经存在relName表
	//因为没有直接的扫描函数，所以只能通过RM的Scan函数来完成扫描
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[1];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	OpenScan(&rmFileScan, dbInfo.sysTables, 1, conditions); 

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
	char sysData[SIZE_SYS_COLUMNS];
	int* sysData_attrcount;
	RID rid;
	//表名（tablename） 占 21 个字节， 即表名为最大长度为 20 的字符串。
	memset(sysData, 0, SIZE_SYS_COLUMNS);
	strncpy(sysData, relName, SIZE_TABLE_NAME);
	//属性的数量（attrcount） 为 int 类型， 占 4 个字节。
	sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME);
	*sysData_attrcount = attrCount;
	
	if (rc = InsertRec(dbInfo.sysTables, sysData, &rid))//插入失败
		//这里直接返回没有做错误处理其实是有问题的，但是为了降低复杂度暂时这样实现了
		return SQL_SYNTAX;

	//向SYSCOLUMNS表插入数据。
	//SYSCOLUMNS:tablename attrname attrtype attrlength attroffset ix_flag indexname
	for (int i = 0; i < attrCount; i++) {
		strncpy(sysData + SIZE_TABLE_NAME, attributes[i].attrName, SIZE_ATTR_NAME-1);//表名（tablename） 与属性名（attrname） 各占 21 个字节
		//attrtype int类型		
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);// 属性的类型（attrtype） 为 int 类型， 占 4 个字节
		*sysData_attrcount = attributes[i].attrType;
		//atrrlength int类型
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);// 属性的长度（attrlength） 为 int 类型，各占 4 个字节
		*sysData_attrcount = attributes[i].attrLength;
		rmRecordSize += attributes[i].attrLength;
		//atrroffset int类型
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH);// 记录中的偏移量（atrroffset） 为 int 类型，各占 4 个字节
		*sysData_attrcount = i;
		//ix_flag bool类型
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET);//该属性列上是否存在索引的标识(ix_flag)占 1 个字节
		*sysData_attrcount = 0;									//没有主键概念，初始都没有索引
		//indexname string类型
		sysData_attrcount = (int*)(sysData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG);
		memset(sysData_attrcount, 0, SIZE_INDEX_NAME);					//索引的名称(indexname)占 21 个字节

		if (rc = InsertRec(dbInfo.sysColumns, sysData, &rid))//插入失败
			return SQL_SYNTAX;
	}

	//5. 计算记录的大小。创建对应的RM文件
	//根据约定，文件名为了relName.rm。
	std::string filePath = dbInfo.curDbName + "\\" + relName + ".rm";
	if(RM_CreateFile((char *)filePath.c_str(),rmRecordSize))
		return SQL_SYNTAX;

	return SUCCESS;
}

//
//目的：销毁名为 relName 的表以及在该表上建立的所有索引。
//1. 检查当前是否已经打开了一个数据库，如果没有则报错。
//2. 判断是否存在表relName.
//	 2.1. 如果已经存在则报错。
//	 3.2. 否则从SYSTABLES中删除relName对应的一项记录
//3. 删除relName对应的rm文件
//4. 删除ix文件
//	 4.1.读取SYSCOLUMNS。得到relName的atrrname。
//	 4.2.判断是否创建了索引。如果创建了索引：删除relName_atrrname对应的ix文件
//	 4.3.删除SYSCOLUMNS中atrrname对应的记录。
RC DropTable(char* relName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. 判断是否存在表relName
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[2];
	RC rc;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysTables, 1, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	//	 2.1. 如果已经存在则报错。
	if (rc != SUCCESS || !rmRecord.bValid) {//已经存在relName表，判断bValid只是为了确保正确
		return SQL_SYNTAX;
	}
	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. 否则从SYSTABLES中删除relName对应的一项记录
	if(DeleteRec(dbInfo.sysTables,&rmRecord.rid))
		return SQL_SYNTAX;

	//3. 删除relName对应的rm文件
	std::string rmFileName="";
	rmFileName += relName;
	rmFileName += ".rm";

	if(!DeleteFile((LPCSTR)rmFileName.c_str()))
		return SQL_SYNTAX;


	//4. 删除ix文件

	//	 4.1.读取SYSCOLUMNS。得到relName的atrrname。
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 1, conditions)) {//在SYSCOLUMNS文件中查找relName表的属性
		return SQL_SYNTAX;
	}

	std::string ixFileName ;
	while (!GetNextRec(&rmFileScan, &rmRecord)) { //无法区分RM_EOF和其他错误，所以无法做错误处理

		//	 4.2.判断是否创建了索引。如果创建了索引：删除relName_atrrname对应的ix文件
		if ((*(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET)) == (char)1) {
			ixFileName=dbInfo.curDbName+"\\";
			ixFileName += (char *)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG);
			if(!DeleteFile(ixFileName.c_str()))
				return SQL_SYNTAX;

		}
		//	 4.3.删除SYSCOLUMNS中atrrname对应的记录。
		if(DeleteRec(dbInfo.sysColumns,&rmRecord.rid))
			return SQL_SYNTAX;
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
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	char ixFileName[SIZE_TABLE_NAME+SIZE_ATTR_NAME+3];//relname+"_"+attrName+".ix"-1
	memset(ixFileName, 0, SIZE_TABLE_NAME+SIZE_ATTR_NAME+3);
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
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;

	conditions[1].attrType = chars; conditions[1].bLhsIsAttr = 1; conditions[1].bRhsIsAttr = 0;
	conditions[1].compOp = EQual; conditions[1].LattrLength = SIZE_ATTR_NAME; conditions[1].LattrOffset = SIZE_TABLE_NAME;
	conditions[1].Lvalue = NULL; conditions[1].RattrLength = SIZE_ATTR_NAME; conditions[1].RattrOffset = 0;
	conditions[1].Rvalue = attrName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 2, conditions))
		return SQL_SYNTAX;

	 GetNextRec(&rmFileScan, &rmRecord);//没有办法对错误进行处理

	char* ix_flag = rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;//无法在Scan的Conditions中加入ix_flag的判断。即使用chars也无法比较。
	if (rmRecord.bValid && (*ix_flag)==(char)1) {//已经创建了对应的索引
		return SQL_SYNTAX;
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. 创建对应的索引。
	//	 3.1. 读取relName,atrrName在SYSCOLUMNS中的记录项，得到atrrType
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 2, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	if (rc != SUCCESS || !rmRecord.bValid)
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 3.2. 创建IX文件
	int attrType = *((int*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);
	int attrLength = *((int*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);
	char* ix_Flag = (char*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	if (*ix_Flag != 0)//检查ix_flag等于0
		return SQL_SYNTAX;
	//RC CreateIndex(const char * fileName,AttrType attrType,int attrLength);
	if(CreateIndex(ixFileName,(AttrType)attrType,attrLength))
		return SQL_SYNTAX;

	//	 3.3. 更新SYS_COLUMNS
	*ix_Flag =(char) 1;
	if(	UpdateRec(dbInfo.sysColumns,&rmRecord))
		return SQL_SYNTAX;
	
	return SUCCESS;
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

	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. 从SYSCOLUMNS中检查索引是否存在。（这里利用了我们约定的indexname具有唯一性来进行查找）
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[3];

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_INDEX_NAME; 
	conditions[0].LattrOffset = SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_INDEX_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = indexName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 1, conditions))
		return SQL_SYNTAX;

	GetNextRec(&rmFileScan, &rmRecord);//没有办法对错误进行处理

	//	 2.1 如果不存在则报错。
	char* ix_flag = rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;//无法在Scan的Conditions中加入ix_flag的判断。即使用chars也无法比较。
	if (!rmRecord.bValid || (*ix_flag) == (char)0)//没有创建索引。
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2.否则修改SYSCOLUMNS中对应记录项ix_flag为0.
	char* ix_Flag = rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	*ix_Flag =(char)0;
	if (UpdateRec(dbInfo.sysColumns, &rmRecord))
		return SQL_SYNTAX;
	
	//3. 删除ix文件
	std::string ixFilePath = dbInfo.curDbName + "\\" + indexName;
	if(!DeleteFile((LPCSTR)ixFilePath.c_str()))
		return SQL_SYNTAX;

	return SUCCESS;
}

//
//目的：该函数用来在 relName 表中插入具有指定属性值的新元组，
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查传入参数的合法性
//	 2.1. 扫描SYSTABLE
//		 2.1.1 如果传入的relName表不存在就报错。
//		 2.1.2 如果存在则保存attrcount。如果attrcount和传入的nValues不一致则报错。
//	 2.2. 扫描SYSCOLUMNS文件。
//		 2.2.1 检查传入values中属性值的类型是否和SYSCOLUMNS记录的相同。
//		 2.2.2 对每一个保存建立了索引的attroffset和indexName
//3. 利用relName打开RM文件，利用保存的indexName打开所有索引文件。对values的每一条记录
//	 3.1. 调用InsertRec方法，向rm插入记录
//	 3.2. 用保存的attroffse和ix文件句柄，向索引文件插入记录
//4. 关闭文件句柄
RC Insert(char* relName, int nValues, Value* values) {
	//nValues 为属性值个数， values 为对应的属性值数组。 

	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. 检查传入参数的合法性
	//	 2.1. 扫描SYSTABLE
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[3];
	RC rc;
	int attrCount=0;

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_TABLE_NAME; conditions[0].LattrOffset = 0;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_TABLE_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = relName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysTables, 1, conditions))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);

	//		 2.1.1 如果传入的relName表不存在就报错。
	if (rc!=SUCCESS || !rmRecord.bValid )
		return SQL_SYNTAX;
	//		 2.1.2 如果存在则保存attrcount。如果attrcount和传入的nValues不一致则报错。
	attrCount = *((int *)rmRecord.pData+SIZE_TABLE_NAME);
	if (attrCount != nValues)
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. 扫描SYSCOLUMNS文件。
	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 1, conditions))
		return SQL_SYNTAX;

	int site_Value = 0;
	std::vector<int> attrOffsets;
	std::vector<std::string> indexNames;
	std::string str_indexName;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	while (rc == SUCCESS && rmRecord.bValid){
		//		 2.2.1 检查传入values中属性值的类型是否和SYSCOLUMNS记录的相同。
		if (values[site_Value].type != (AttrType)(*((int*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME)))
			return SQL_SYNTAX;
		//		 2.2.2 保存建立了索引的attroffset和indexName
		if (*((char*)rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET) == (char)1) {
			attrOffsets.push_back(*((int*)rmRecord.pData  + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH));
			str_indexName = dbInfo.curDbName;
			str_indexName+=(char *)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_ATTR_OFFSET);
			indexNames.push_back(str_indexName);
		}
		site_Value++;
		rc = GetNextRec(&rmFileScan, &rmRecord);
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. 利用relName打开RM文件，利用保存的indexName打开所有索引文件。对values的每一条记录
	std::string rmFileName="";
	int numIndexs = indexNames.size();
	RM_FileHandle rmFileHandle;
	std::vector<IX_IndexHandle> ixIndexHandles;
	IX_IndexHandle curIndexHanle;

	//打开RM文件
	rmFileName = dbInfo.curDbName + "\\" + relName + ".rm";
	if (RM_OpenFile((char *)rmFileName.c_str(), &rmFileHandle))
		return SQL_SYNTAX;

	//打开ix文件
	for (int i = 0; i < numIndexs; i++) {
		if (OpenIndex(indexNames[i].c_str(), &curIndexHanle))
			return SQL_SYNTAX;
		ixIndexHandles.push_back(curIndexHanle);
	}

	site_Value = 0;
	std::vector<RID> rmInsertRids;
	RID rid;
	char* insertData;
	while (site_Value < nValues) {
		//	 3.1. 调用InsertRec方法，向rm插入记录
		if (InsertRec(&rmFileHandle, (char*)(values[site_Value].data), &rid))
			return SQL_SYNTAX;
		//	 3.2. 用保存的attroffset和ix文件句柄，向索引文件插入记录
		insertData = (char*)values[site_Value].data;
		for (int i = 0; i < numIndexs; i++){			
			curIndexHanle = ixIndexHandles[i];
			if (InsertEntry(&curIndexHanle, (void*)(insertData + attrOffsets[i]), &rid))
				return SQL_SYNTAX;
		}
	}

	//4. 关闭文件句柄
	if (RM_CloseFile(&rmFileHandle))
		return SQL_SYNTAX;
	for (int i = 0; i < numIndexs; i++) {
		curIndexHanle = ixIndexHandles[i];
		if (CloseIndex(&curIndexHanle))
			return SQL_SYNTAX;
	}

	return SUCCESS;
}


//
//目的：该函数用来删除 relName 表中所有满足指定条件的元组以及该元组对应的索
//引项。 如果没有指定条件， 则此方法删除 relName 关系中所有元组。 如果包
//含多个条件， 则这些条件之间为与关系。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查传入参数的合法性
//	 2.1. 扫描SYSTABLE.如果传入的relName表不存在就报错。
//	 2.2. 扫描SYSCOLUMNS文件。对每一个保存建立了索引的indexName
//3. 利用relName打开RM文件，利用保存的indexName打开所有索引文件。
//4. 将输入的nConditions和conditions转换成RM_FileScan的参数。
//5. 用rm句柄、nConditions和conditions创建rmFileScan。对筛选处的每一项记录
//	 3.1. 调用rm句柄的DeleteRec方法进行删除。
//	 3.2. 用保存ix文件句柄，删除ix记录。
//6. 关闭文件句柄
RC Delete(char* relName, int nConditions, Condition* conditions) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName[0] == 0)
		return SQL_SYNTAX;

	//2. 检查传入参数的合法性
	//	 2.1. 扫描SYSTABLE.如果传入的relName表不存在就报错。
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con cons[3];
	RC rc;
	int attrCount = 0;

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysTables, 1, cons))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	if (rc != SUCCESS || !rmRecord.bValid)//不存在relName表
		return SQL_SYNTAX;

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//	 2.2. 扫描SYSCOLUMNS文件。对每一个保存建立了索引的indexName
	int site_Value = 0;
	std::vector<std::string> indexNames;
	std::string str_indexName;

	char str_one[1];
	str_one[0]=(char)1;
	cons[1].attrType = chars; cons[1].bLhsIsAttr = 1; cons[1].bRhsIsAttr = 0;
	cons[1].compOp = EQual; cons[1].LattrLength = 1; 
	cons[1].LattrOffset = SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	cons[1].Lvalue = NULL; cons[1].RattrLength = 1; cons[1].RattrOffset = 0;
	cons[1].Rvalue = str_one;

	rmRecord.bValid = false;
	if (OpenScan(&rmFileScan, dbInfo.sysColumns, 2, cons))
		return SQL_SYNTAX;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	while (rc == SUCCESS && rmRecord.bValid) {
		//保存建立了索引的indexName
		str_indexName = dbInfo.curDbName; 
		str_indexName += (char *)(rmRecord.pData  + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_ATTR_OFFSET + SIZE_IX_FLAG);
		indexNames.push_back(str_indexName);
		rc = GetNextRec(&rmFileScan, &rmRecord);
	}

	if (CloseScan(&rmFileScan))
		return SQL_SYNTAX;

	//3. 利用relName打开RM文件，利用保存的indexName打开所有索引文件。
	std::string rmFileName=dbInfo.curDbName+"\\"+relName+".rm";//relName+".rm";
	int numIndexs = indexNames.size();
	RM_FileHandle rmFileHandle;
	std::vector<IX_IndexHandle> ixIndexHandles;
	IX_IndexHandle curIndexHanle;

	//打开RM文件
	if (RM_OpenFile((char *)rmFileName.c_str(), &rmFileHandle))
		return SQL_SYNTAX;

	//打开ix文件
	for (int i = 0; i < numIndexs; i++) {
		if (OpenIndex(indexNames[i].c_str(), &curIndexHanle))
			return SQL_SYNTAX;
		ixIndexHandles.push_back(curIndexHanle);
	}

	//4. 将输入的nConditions和conditions转换成RM_FileScan的参数。
	

	return SUCCESS;
}


//目的：向SYSTABLES插入一条 relName/attrCount 记录
//1. 检查relName是否已经存在，如果已经存在，则返回错误TABLE_EXIST
//2. 向SYSTABLES插入记录
RC TableMetaInsert(char* relName, int attrCount) {
	RC rc;
	RID rid;
	RM_Record rmRecord;
	if (TableMetaSearch(relName, &rmRecord) == SUCCESS)
		return TABLE_EXIST;

	char insertData[SIZE_TABLE_NAME + SIZE_ATTR_COUNT];
	int* insertAttrCount;
	
	memset(insertData, 0, sizeof(insertData));
	strcpy(insertData,relName);
	insertAttrCount = (int *)(insertData + SIZE_TABLE_NAME);
	*insertAttrCount = attrCount;

	if (rc = InsertRec(dbInfo.sysTables, insertData, &rid))
		return rc;

	return SUCCESS;
}

//
// 目的：在SYSTABLE表中删除relName匹配的那一项
// relName: 表名称
// 1. 判断 relName 是否存在, 不存在返回 TABLE_NOT_EXIST
// 2. 如果存在则从SYSTABLE中删除relName匹配的记录，
RC TableMetaDelete(char* relName){
	RC rc;
	RM_Record rmRecord;

	// 1. 判断 relName 是否存在, 不存在返回 TABLE_NOT_EXIST
	if ((rc = TableMetaSearch(relName, rmRecord)))
		return rc;

	// 2. 如果存在则从SYSTABLE中删除relName匹配的记录，
	if ((rc = DeleteRec(dbInfo.sysTables, &rmRecord.rid)))
		return rc;

	return SUCCESS;
}

//
// 目的：在SYSTABLES表中查找relName是否存在。
// relName: 表名称
// 返回值：
// 1. 不存在返回 TABLE_NOT_EXIST
// 2. 存在 返回 SUCCESS
// 
RC TableMetaSearch(char* relName,RM_Record* rmRecord) {
	RM_FileScan rmFileScan;
	RC rc;
	Con cons[1];

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	if ((rc = OpenScan(&rmFileScan, dbInfo.sysTables, 1, cons)) ||
		(rc = GetNextRec(&rmFileScan, rmRecord)) || (rc = CloseScan(&rmFileScan)))
		return rc;

	// 1. 不存在返回 TABLE_NOT_EXIST
	if (rc != SUCCESS || !rmRecord->bValid)
		return TABLE_NOT_EXIST;

	// 2. 存在 返回 SUCCESS
	return SUCCESS;
}

// 
// 目的：打印出SYSTABLES数据表
// RC 代表 RM Scan 过程中发生的错误 (除去 RM_EOF)
// 完成 Scan 返回 Success
// 输出格式尽量好看些，仿照 mysql 是最吼的
// 
RC TableMetaShow() {
	return SUCCESS;
}

//
// 目的： 扫描SYSCOLUMNS中relName和attrName是否存在。如果不存在则返回ATTR_NOT_EXIST
RC ColumnSearchAttr(char* relName, char* attrName, RM_Record* rmRecord) {
	RM_FileScan rmFileScan;
	RC rc;
	Con cons[2];
	
	RID rid;

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	cons[1].attrType = chars; cons[1].bLhsIsAttr = 1; cons[1].bRhsIsAttr = 0;
	cons[1].compOp = EQual; cons[1].LattrLength = SIZE_ATTR_NAME; cons[1].LattrOffset = SIZE_TABLE_NAME;
	cons[1].Lvalue = NULL; cons[1].RattrLength = SIZE_ATTR_NAME; cons[1].RattrOffset = 0;
	cons[1].Rvalue = attrName;

	rmRecord->bValid = false;

	if ((rc = OpenScan(&rmFileScan, dbInfo.sysColumns, 2, cons)) ||
		(rc = GetNextRec(&rmFileScan, rmRecord)) || (rc = CloseScan(&rmFileScan)))
		return rc;

	// 扫描SYSCOLUMNS中relName和attrName是否存在。如果不存在则返回ATTR_NOT_EXIST
	if (rc != SUCCESS || !rmRecord->bValid)
		return ATTR_NOT_EXIST;

	return SUCCESS;
}

//
// 目的：将各个数据项写入pData中，用来向SYSCOLUMNS进行插入
RC ToData(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName,char* pData) {
	int* pDataAttrType, * pDataAttrLength, * pDataAttrOffset;
	char* pDataIxFlag;
	memset(pData, 0, SIZE_SYS_COLUMNS);
	strcpy(pData, relName);
	strcpy(pData + SIZE_TABLE_NAME, attrName);
	pDataAttrType = (int*)(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);
	*pDataAttrType = attrType;
	pDataAttrLength = (int*)(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);
	*pDataAttrLength = attrLength;
	pDataAttrOffset = (int*)(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH);
	*pDataAttrOffset = attrOffset;
	pDataIxFlag = pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET;
	*pDataIxFlag = ixFlag;
	strcpy(pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE + SIZE_ATTR_LENGTH + SIZE_ATTR_OFFSET + SIZE_IX_FLAG, indexName);
	
	return SUCCESS;
}

//
// 目的：向SYSCOLUMNS中插入一条记录
// 1.检查relName表是否存在，如果不存在则返回错误。
// 2.检查输入relName的attrName项是否已经存在。如果存在返回ATTR_EXIST
// 3.向SYSCOLUMNS中插入记录值
//	 3.1. 检查relName、attrName和indexname是否超过指定范围
//	 3.2. 将输入的各项值整合
//	 3.3. 向SYSCOLUMNS表插入
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName) {
	RC rc;
	RM_Record rmRecord;
	// 1.检查relName表是否存在，如果不存在则返回错误。
	if ((rc = TableMetaSearch(relName, &rmRecord)))
		return rc;

	// 2.检查输入relName的attrName项是否已经存在。如果存在返回ATTR_EXIST
	if (  ColumnSearchAttr(relName, attrName,&rmRecord) == SUCCESS)
		return ATTR_EXIST;

	// 2.向SYSCOLUMNS中插入记录值

	//	 2.1. 检查relName和attrName是否超过指定范围
	if (strlen(relName) >= SIZE_TABLE_NAME || strlen(attrName) >= SIZE_ATTR_NAME 
		||strlen(indexName)>= SIZE_INDEX_NAME)
		return NAME_TOO_LONG;

	//	 2.2. 将输入的各项值整合
	RID rid;
	char pData[SIZE_SYS_COLUMNS];
	ToData(relName, attrName, attrType, attrLength, attrOffset, ixFlag, indexName, pData);

	//	 3.3. 向SYSCOLUMNS表插入
	if((rc=InsertRec(dbInfo.sysColumns,pData,&rid)))
		return rc;

	return SUCCESS;
}

//
// 目的：删除同一个表中的所有属性列信息
// 1. 检查relName表是否存在
// 2. 遍历SYSCOLUMNS，删除relName相关的所有记录
RC ColumnMetaDelete(char* relName) {
	// 1. 检查relName表是否存在
	RC rc;
	RM_Record rmRecord;
	if ((rc = TableMetaSearch(relName, &rmRecord)))
		return rc;

	// 2. 遍历SYSCOLUMNS，删除relName相关的所有记录
	RM_FileScan rmFileScan;
	Con cons[1];
	RID rid;

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;
	rmRecord.bValid = false;

	if ((rc = OpenScan(&rmFileScan, dbInfo.sysColumns, 1, cons)))
		return rc;

	rc = GetNextRec(&rmFileScan, &rmRecord);
	while (rc == SUCCESS && rmRecord.bValid) {
		if ((rc = DeleteRec(dbInfo.sysColumns, &rmRecord.rid)) ||
			(rc = GetNextRec(&rmFileScan, &rmRecord)))
			return rc;
	}

	if((rc=CloseScan(&rmFileScan)))
		return rc;

	return SUCCESS;
}


//
// 目的：// 更新SYSCOLUMNS元数据表中的某一项记录 
// 1. 检查relName和attrName是否存在。如果不存在则报错
// 2. 更新SYSCOLUMNS
RC ColumnMetaUpdate(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName) {
	// 1. 检查relName和attrName是否存在。如果不存在则报错
	RC rc;
	RM_Record rmRecord;
	if ((rc = TableMetaSearch(relName, &rmRecord)) ||
		(rc=ColumnSearchAttr(relName,attrName,&rmRecord)))
		return rc;

	// 2. 更新SYSCOLUMNS
	RID rid;
	char pData[SIZE_SYS_COLUMNS];
	ToData(relName, attrName, attrType, attrLength, attrOffset, ixFlag, indexName, pData);
	rmRecord.pData = pData;

	if ((rc = UpdateRec(dbInfo.sysColumns,&rmRecord)))
		return rc;

	return SUCCESS;
}


//
// 目的：扫描获取relName表的attrName属性的类型、长度
// 1. 判断relName和attrName是否存在
// 2. 将信息封装进attribute
RC ColunmMetaGet(char* relName, char* attrName, AttrInfo* attribute) {
	// 1. 检查relName和attrName是否存在。如果不存在则报错
	RC rc;
	RM_Record rmRecord;
	if ((rc = TableMetaSearch(relName, &rmRecord)) ||
		(rc = ColumnSearchAttr(relName, attrName, &rmRecord)))
		return rc;

	strcpy(attrName, attribute->attrName);
	AttrType* attrType = (AttrType*)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME);
	attribute->attrType = *attrType;
	int* attrLength = (int*)(rmRecord.pData + SIZE_TABLE_NAME + SIZE_ATTR_NAME + SIZE_ATTR_TYPE);
	attribute->attrLength = *attrLength;

	return SUCCESS;
}