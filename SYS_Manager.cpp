#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>

DB_INFO dbInfo;

RC execute(char* sql, CEditArea* editArea) {
	RC rc;
	sqlstr* sql_str = get_sqlstr();
	rc = parse(sql, sql_str);//只有两种返回结果SUCCESS和SQL_SYNTAX
	if (rc == SUCCESS) {
		switch (sql_str->flag) {
		case 1: {
			//判断SQL语句为select语句
			SelResult res;
			Init_Result(&res);
			//将查询结果处理一下，整理成下面这种形式
			//调用editArea->ShowSelResult(col_num,row_num,fields,rows);
			if ((rc = Query(sql, &res))) {
				return rc;
			}
			int col_num = res.col_num;//列
			int row_num = 0;//行
			SelResult* cur_res = &res;
			while (cur_res) {//所有节点的记录数之和
				row_num += cur_res->row_num;
				cur_res = cur_res->next_res;
			}
			char** fields = new char* [20];//各字段名称
			for (int i = 0; i < col_num; i++) {
				fields[i] = new char[20];
				memset(fields[i], 0, 20);
				memcpy(fields[i], res.fields[i], 20);
			}
			cur_res = &res;
			char*** rows = new char** [row_num];
			for (int i = 0; i < row_num; i++) {
				rows[i] = new char* [col_num];//存放一条记录
				for (int j = 0; j < col_num; j++) {
					rows[i][j] = new char[20];//一条记录的一个字段
					memset(rows[i][j], 0, 20);
					memcpy(rows[i][j], cur_res->res[i][j], 20);
				}
				if (i == 99)
					cur_res = cur_res->next_res;//每个链表节点最多记录100条记录
			}
			editArea->ShowSelResult(col_num, row_num, fields, rows);
			for (int i = 0; i < col_num; i++) {
				delete[] fields[i];
			}
			delete[] fields;
			Destory_Result(&res);
			return SELECT_SUCCESS;
		}
		case 2:
			//判断SQL语句为insert语句
			if((rc = Insert(sql_str->sstr.ins.relName, sql_str->sstr.ins.nValues, sql_str->sstr.ins.values))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 3:
			//判断SQL语句为update语句
			if ((rc = Update(sql_str->sstr.upd.relName, sql_str->sstr.upd.attrName, &sql_str->sstr.upd.value,
				sql_str->sstr.upd.nConditions, sql_str->sstr.upd.conditions))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 4:
			//判断SQL语句为delete语句
			if ((rc = Delete(sql_str->sstr.del.relName, sql_str->sstr.del.nConditions, sql_str->sstr.del.conditions))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 5:
			//判断SQL语句为createTable语句
			if ((rc = CreateTable(sql_str->sstr.cret.relName, sql_str->sstr.cret.attrCount, sql_str->sstr.cret.attributes))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 6:
			//判断SQL语句为dropTable语句
			if ((rc = DropTable(sql_str->sstr.drt.relName))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 7:
			//判断SQL语句为createIndex语句
			if ((rc = CreateIndex(sql_str->sstr.crei.indexName, sql_str->sstr.crei.relName, sql_str->sstr.crei.attrName))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 8:
			//判断SQL语句为dropIndex语句
			if ((rc = DropIndex(sql_str->sstr.dri.indexName))) {
				return SQL_SYNTAX;
			}
			return SUCCESS;
		case 9:
			//判断为help语句，可以给出帮助提示
			return SUCCESS;
		case 10:
			//判断为exit语句，可以由此进行退出操作
			AfxGetMainWnd()->SendMessage(WM_CLOSE);
			return SUCCESS;
		}
	}
	else {
		AfxMessageBox(sql_str->sstr.errors);//弹出警告框，sql语句词法解析错误信息
		return rc;
	}
	return SUCCESS;
}

void ExecuteAndMessage(char* sql, CEditArea* editArea) {//根据执行的语句类型在界面上显示执行结果。此函数需修改
	RC rc = execute(sql, editArea);
	int row_num = 0;
	char** messages = NULL;
	switch (rc) {
	case SUCCESS:
		row_num = 1;
		messages = new char* [row_num];
		messages[0] = "操作成功";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char* [row_num];
		messages[0] = "有语法错误";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	case SELECT_SUCCESS:
		break;
	default:
		row_num = 1;
		messages = new char* [row_num];
		messages[0] = "功能未实现";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	}
}

//
//目的：在路径 dbPath 下创建一个名为 dbName 的空库，生成相应的系统文件。
//1. 检查dbpath和dbname的合法性。
//2. 判断dbpath以及dbname是否和dbInfo中保存相同。如果不同则调用CloseDb。如果相同则返回。
//3. 向OS申请在dbpath路径创建文件夹。
//4. 创建SYSTABLES文件和SYSCOLUMNS文件。
RC CreateDB(char* dbpath, char* dbname) {

	RC rc;
	//1. 检查dbpath和dbname的合法性。
	if (dbpath == NULL || dbname == NULL)
		return DB_NAME_ILLEGAL;

	//2. 判断dbpath以及dbname是否和dbInfo中保存相同。如果不同则调用CloseDb。如果相同则返回。
	std::string dbPath = dbpath;
	dbPath = dbPath + "\\" + dbname;
	if ( dbInfo.curDbName.size() && dbInfo.curDbName.compare(dbPath) != 0 ) {
		if ((rc = CloseDB()))
			return rc;
	}
	if (dbInfo.curDbName.size() && dbInfo.curDbName.compare(dbPath) == 0) {
		return SUCCESS;
	}

	//3. 向OS申请在dbpath路径创建文件夹。
	if (CreateDirectory(dbPath.c_str(), NULL)) {
		if (SetCurrentDirectory((char*)dbPath.c_str())) {
			//4. 创建SYSTABLES文件和SYSCOLUMNS文件
			std::string sysTablePath = TABLE_META_NAME;
			std::string sysColumnsPath = COLUMN_META_NAME;
			if ((rc = RM_CreateFile((char*)(sysTablePath.c_str()), SIZE_SYS_TABLE)) ||
				(rc = RM_CreateFile((char*)(sysColumnsPath.c_str()), SIZE_SYS_COLUMNS)))
				return rc;

			return SUCCESS;
		}
		return OS_FAIL;
	}

	return OS_FAIL;
}


//
//目的：删除数据库对应的目录以及目录下的所有文件
//      默认不存在子文件夹，所以不进行递归处理
//1. 判断dbname是否为空，或者dbInfo.path下是否存在dbname。如果不存在则报错。
//2. 判断删除的是否是当前DB。如果则要关闭当前目录.
//3. 搜索得到dbname目录下的所有文件名并删除,注意要跳过.和..目录
//4. 删除dbname目录
RC DropDB(char* dbname) {

	//1. 判断dbname是否为空，或者是否存在dbname。如果不存在则报错。
	std::string dbPath = dbname;
	if (dbname == NULL || access(dbPath.c_str(), 0) == -1)
		return DB_NOT_EXIST;

	//2. 判断删除的是否是当前DB
	RC rc;
	if (dbInfo.curDbName.compare(dbname) == 0) {
		//关闭当前目录
		if ((rc = CloseDB()))
			return rc;
	}

	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	dbPath += "\\*";
	hFile = FindFirstFile(dbPath.c_str(), &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return OS_FAIL;

	//3. 遍历删除文件夹下的所有文件
	std::string fileName;
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//跳过.和..目录
			continue;
		fileName = dbname;
		fileName = fileName + "\\" + pNextInfo.cFileName;
		if (!DeleteFile(fileName.c_str()))
			return OS_FAIL;
	}

	//4. 删除dbname目录
	std::string dbName =dbname;
	if (!RemoveDirectory(dbName.c_str())) {
		int i = GetLastError();
		return OS_FAIL;
	}
		

	return SUCCESS;
}

//
//目的：改变系统的当前数据库为 dbName 对应的文件夹中的数据库
//1. 检查dbname是否为空。已经能否达到。
//2. 如果当前打开了db.
//	 2.1 如果打开的目录和dbname相同，则返回。
//	 2.2 否则关闭当前打开的目录。
//3. 打开SYSTABLES和SYSCOLUMNS,设置dbInfo
//4. 设置curDbName
RC OpenDB(char* dbname) {

	RC rc;
	//1. 检查dbname是否为空。
	if (dbname == NULL)
		return SQL_SYNTAX;

	//2. 如果当前打开了db.
	if (dbInfo.curDbName.size()) {
		//	 2.1 如果打开的目录和dbname相同，则返回。
		if (dbInfo.curDbName.compare(dbname) == 0)
			return SUCCESS;
		//	 2.2 否则关闭当前打开的目录。
		if ((rc = CloseDB()))
			return rc;
	}

	//2. 打开SYSTABLES和SYSCOLUMNS,设置dbInfo
	std::string sysTablePath = TABLE_META_NAME;
	//sysTablePath = sysTablePath + "\\" + TABLE_META_NAME;
	std::string sysColumnsPath = COLUMN_META_NAME;
	//sysColumnsPath = sysColumnsPath + "\\" + COLUMN_META_NAME;

	if ((rc = RM_OpenFile((char*)sysTablePath.c_str(), &(dbInfo.sysTables))) ||
		(rc = RM_OpenFile((char*)sysColumnsPath.c_str(), &(dbInfo.sysColumns))))
		return rc;

	//3. 设置curDbName
	dbInfo.curDbName = dbname;
	if (!SetCurrentDirectory(dbname)) {
		return OS_FAIL;
	}
	
	return SUCCESS;
}



//
//目的:关闭当前数据库。关闭当前数据库中打开的所有文件
//1. 检查当前是否打开了DB。如果curDbName为空则报错，否则设置curDbName为空。
//2. 调用保存的SYS文件句柄close函数，关闭SYS文件。
RC CloseDB() {
	//1. 检查当前是否打开了DB。如果curDbName为空则报错，否则设置curDbName为空。
	if (!dbInfo.curDbName.size())
		return SQL_SYNTAX;
	dbInfo.curDbName.clear();

	//2. 调用保存的SYS文件句柄close函数，关闭SYS文件。
	RM_CloseFile(&(dbInfo.sysTables));
	RM_CloseFile(&(dbInfo.sysColumns));

	return SUCCESS;
}

bool CanButtonClick() {//需要重新实现
	//如果当前有数据库已经打开
	if (dbInfo.curDbName.size())
		return true;
	//如果当前没有数据库打开
	return false;
}

//
//目的：检查传入的attributes是否存在属性名过长或者重名。
bool attrVaild(int attrCount, AttrInfo* attributes)
{
	std::set<std::string> dupJudger;
	std::set<std::string>::iterator iter;

	for (int i = 0; i < attrCount; i++) {
		if (strlen(attributes[i].attrName) >= SIZE_ATTR_NAME)
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
//目的:创建一个名为 relName 的表。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查输入参数的合法性：
//	 2.1. 检查atrrCount是否大于 MAXATTRS。
//	 2.2. 检查relName是否大于20个字节。
//	 2.3. 检查relName是否和SYSTABLES以及SYSCOLUMNS同名。
//   2.4. 检查attributes是否合法。
//3. 检查是否已经存在relName表。如果已经存在，则返回错误。
//4. 向SYSTABLES和SYSCOLUMNS表中传入元信息,并计算relName表记录的大小。
//5. 创建对应的RM文件。并将SYSTABLE文件和SYSCOLUMNS文件刷写到disk。
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) {
	//参数 attrCount 表示关系中属性的数量（取值为1 到 MAXATTRS 之间） 。 
	//参数 attributes 是一个长度为 attrCount 的数组。 对于新关系中第 i 个属性，
	//attributes 数组中的第 i 个元素包含名称、 类型和属性的长度（见 AttrInfo 结构定义）

	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return DB_NOT_EXIST;

	//2. 检查输入参数的合法性：

	//	 2.1. 检查atrrCount是否大于 MAXATTRS。
	if (attrCount > dbInfo.MAXATTRS)
		return TOO_MANY_ATTR;
	//	 2.2. 检查relName是否大于20个字节。
	if (strlen(relName) >= SIZE_TABLE_NAME)
		return TABLE_NAME_ILLEGAL;
	//	 2.3. 检查relName是否和SYSTABLES以及SYSCOLUMNS同名。
	if (strcmp(relName, TABLE_META_NAME) == 0 ||
		strcmp(relName, COLUMN_META_NAME) == 0)
		return TABLE_NAME_ILLEGAL;
	//   2.4. 检查attributes是否合法。
	if (!attrVaild(attrCount, attributes))
		return INVALID_ATTRS;

	//3. 检查是否已经存在relName表。如果已经存在，则返回错误。
	RM_Record rmRecord;
	RC rc;
	rmRecord.pData = new char[SIZE_SYS_TABLE];
	memset(rmRecord.pData, 0, SIZE_SYS_TABLE);

	if ((rc = TableMetaSearch(relName, &rmRecord)) != TABLE_NOT_EXIST) {
		if (rc == SUCCESS) {
			delete[] rmRecord.pData;
			return TABLE_EXIST;
		}
		delete[] rmRecord.pData;
		return rc;
	}

	//4. 向SYSTABLES和SYSCOLUMNS表中传入元信息,并计算relName表记录的大小。

	//向SYSTABLES插入信息
	if ((rc = TableMetaInsert(relName, attrCount))) {
		//插入失败
		delete[] rmRecord.pData;
		return rc;
	}

	//向SYSCOLUMNS插入信息
	char indexName[1];//indexName为空
	int rmRecordSize = 0;
	indexName[0] = 0;
	for (int i = 0; i < attrCount; i++) {
		if ((rc = ColumnMetaInsert(relName, attributes[i].attrName, attributes[i].attrType,
			attributes[i].attrLength, rmRecordSize, 0, indexName))) {
			delete[] rmRecord.pData;
			return rc;
		}
		//计算记录的大小
		rmRecordSize += attributes[i].attrLength;
	}

	//5. 创建对应的RM文件。并将SYSTABLE文件和SYSCOLUMNS文件刷写到disk。
	//根据约定，文件名为了relName.rm。
	std::string filePath = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_CreateFile((char*)filePath.c_str(), rmRecordSize))||
		(rc = ForceDataPages(&dbInfo.sysTables.pfFileHandle)) ||
		(rc = ForceDataPages(&dbInfo.sysColumns.pfFileHandle)) ) {
		delete[] rmRecord.pData;
		return rc;
	}

	//

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//目的：销毁名为 relName 的表以及在该表上建立的所有索引。
//1. 检查当前是否已经打开了一个数据库，如果没有则报错。
//2. 检查relName是否和SYSTABLES以及SYSCOLUMNS同名。
//3. 从SYSTABLES中删除relName对应的记录以及relName对应的rm文件
//4. 删除SYSCOLUMNS中的记录项以及可能存在的ix文件
RC DropTable(char* relName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	RC rc;

	//2. 检查relName是否和SYSTABLES以及SYSCOLUMNS同名。
	if (strcmp(relName, TABLE_META_NAME) == 0 ||
		strcmp(relName, COLUMN_META_NAME) == 0) {
		return TABLE_NAME_ILLEGAL;
	}

	//3. 从SYSTABLES中删除relName对应的记录以及relName对应的rm文件
	if ((rc = TableMetaDelete(relName))) {
		return rc;
	}

	//4. 删除SYSCOLUMNS中的记录项以及可能存在的ix文件
	if ((rc = ColumnMetaDelete(relName))) {
		return rc;
	}

	if((rc= ForceDataPages(&(dbInfo.sysTables.pfFileHandle))) ||
		(rc= ForceDataPages(&(dbInfo.sysColumns.pfFileHandle))))
		return rc;

	return SUCCESS;
}

//该函数在关系 relName 的属性 attrName 上创建名为 indexName 的索引。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 判断输入参数的indexName长度是否合法。
//3. 检查是否已经存在对应的索引。如果已经存在则报错。
//4. 创建对应的索引。
//	 4.1. 创建IX文件
//	 4.2. 更新SYS_COLUMNS
//   4.3. 扫描RM文件，将已有记录信息插入到创建的IX文件
RC CreateIndex(char* indexName, char* relName, char* attrName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. 判断输入参数的indexName长度是否合法。
	if (strlen(indexName) >= SIZE_INDEX_NAME) {
		return INDEX_NAME_ILLEGAL;
	}
	std::string ixFileName = dbInfo.curDbName + "\\" + indexName + IX_FILE_SUFFIX;

	//3. 检查是否已经存在对应的索引。如果已经存在则报错。
	RM_Record rmRecord;
	RC rc;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	if ((rc = ColumnSearchAttr(relName, attrName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}

	char* ix_flag = rmRecord.pData + ATTR_IXFLAG_OFF;//无法在Scan的Conditions中加入ix_flag的判断。即使用chars也无法比较。
	if ((*ix_flag) == (char)1) {//已经创建了对应的索引
		delete[] rmRecord.pData;
		return INDEX_EXIST;
	}

	//4. 创建对应的索引。
	//	 4.1. 创建IX文件
	int attrType = *((int*)(rmRecord.pData + ATTR_TYPE_OFF));
	int attrLength = *((int*)(rmRecord.pData + ATTR_LENGTH_OFF));
	int attrOffset = *((int*)(rmRecord.pData + ATTR_OFFSET_OFF));
	if ((rc = CreateIndex((char*)ixFileName.c_str(), (AttrType)attrType, attrLength))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//	 4.2. 更新SYS_COLUMNS
	if ((rc = ColumnMetaUpdate(relName, attrName, 1, indexName))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//   4.3. 扫描RM文件，将已有记录信息插入到创建的IX文件
	if ((rc = CreateIxFromTable(relName, indexName, attrOffset))) {
		delete[] rmRecord.pData;
		return rc;
	}
	if((rc=ForceDataPages(&(dbInfo.sysTables.pfFileHandle))) ||
		(rc= ForceDataPages(&(dbInfo.sysColumns.pfFileHandle)))){
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
//	 2.2. 删除ix文件
//	 2.3.否则修改SYSCOLUMNS中对应记录项ix_flag为0.

RC DropIndex(char* indexName) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. 从SYSCOLUMNS中检查索引是否存在。（这里利用了我们约定的indexname具有唯一性来进行查找）
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	Con conditions[1];
	RC rc;

	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	conditions[0].attrType = chars; conditions[0].bLhsIsAttr = 1; conditions[0].bRhsIsAttr = 0;
	conditions[0].compOp = EQual; conditions[0].LattrLength = SIZE_INDEX_NAME;
	conditions[0].LattrOffset = ATTR_INDEX_NAME_OFF;
	conditions[0].Lvalue = NULL; conditions[0].RattrLength = SIZE_INDEX_NAME; conditions[0].RattrOffset = 0;
	conditions[0].Rvalue = indexName;

	if ((rc = OpenScan(&rmFileScan, &(dbInfo.sysColumns), 1, conditions))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//	 2.1 如果不存在则报错。
	rc = GetNextRec(&rmFileScan, &rmRecord);
	if (rc == RM_EOF) {
		delete[] rmRecord.pData;
		return INDEX_NOT_EXIST;
	}
	if (rc != SUCCESS) {
		delete[] rmRecord.pData;
		return rc;
	}
	char* ix_flag = rmRecord.pData + ATTR_IXFLAG_OFF;
	if ((*ix_flag) == (char)0) {//没有创建索引。
		delete[] rmRecord.pData;
		return INDEX_NOT_EXIST;
	}
	if ((rc = CloseScan(&rmFileScan))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//	2.2. 删除ix文件
	std::string ixFilePath = dbInfo.curDbName + "\\" + indexName + IX_FILE_SUFFIX;
	if (!DeleteFile((LPCSTR)ixFilePath.c_str())) {
		delete[] rmRecord.pData;
		return OS_FAIL;
	}

	//	 2.3.否则修改SYSCOLUMNS中对应记录项ix_flag为0.
	ix_flag = rmRecord.pData + ATTR_IXFLAG_OFF;
	*ix_flag = (char)0;
	if ((rc = UpdateRec(&(dbInfo.sysColumns), &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}

	if((rc= ForceDataPages(&(dbInfo.sysTables.pfFileHandle))) ||
		(rc= ForceDataPages(&(dbInfo.sysColumns.pfFileHandle)))){
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
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
//		 2.2.1 检查传入value中属性值的类型和长度是否和SYSCOLUMNS记录的相同。
//		 2.2.2 如果传入的value类型是chars，判断values.data长度是否和SYSCOLUMNS记录相同
//3. 将values的数据整合为一个数据。
//4. 利用relName打开RM文件.
//5. 对每一个保存建立了索引的属性打开对应的ix文件并保存索引。
//6. 向RM文件插入记录。同时向可能存在的索引文件插入记录。
//7. 关闭文件句柄
RC Insert(char* relName, int nValues, Value* values) {
	//nValues 为属性值个数， values 为对应的属性值数组。 
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. 检查传入参数的合法性
	//	 2.1. 扫描SYSTABLE
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	RC rc;
	int attrCount = 0;

	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	//		 2.1.1 如果传入的relName表不存在就报错。
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//		 2.1.2 如果存在则保存attrcount。如果attrcount和传入的nValues不一致则报错。
	attrCount = *((int*)(rmRecord.pData + ATTR_COUNT_OFF));
	if (attrCount != nValues) {
		delete[] rmRecord.pData;
		return INVALID_VALUES;
	}

	delete[] rmRecord.pData;

	//	 2.2. 扫描SYSCOLUMNS文件。
	std::vector<AttrEntry> attributes;
	std::string str_indexName;
	int recordSize = 0;
	if ((rc = ColumnEntryGet(relName, &attrCount, attributes))) {
		return rc;
	}

	for (int i = 0; i < attrCount; i++) {
		//		 2.2.1 检查传入values中属性值的类型是否和SYSCOLUMNS记录的相同。
		if (values[i].type != attributes[i].attrType) {
			return INVALID_VALUES;
		}
		//		 2.2.2 如果传入的value类型是chars，判断values.data长度是否和SYSCOLUMNS记录相同
		if ((values[i].type == chars) && strlen((char*)values[i].data) >= attributes[i].attrLength) {
			return INVALID_VALUES;
		}
		recordSize += attributes[i].attrLength;
	}

	//3. 将values的数据整合为一个数据。
	RID rid;
	char* insertData = new char[recordSize];
	recordSize = 0;
	for (int i = 0; i < attrCount; i++) {
		memcpy(insertData + recordSize, values[i].data, attributes[i].attrLength);
		recordSize += attributes[i].attrLength;
	}

	//4. 利用relName打开RM文件.
	std::string rmFileName = "";
	RM_FileHandle rmFileHandle;

	rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		delete[] insertData;
		return rc;
	}
	//5. 对每一个保存建立了索引的属性打开对应的ix文件并保存索引。
	std::vector<IxEntry> ixEntrys;
	IxEntry curIxEntry;
	for (int i = 0; i < attrCount; i++) {
		if (attributes[i].ix_flag) {
			str_indexName = dbInfo.curDbName + "\\" + attributes[i].indexName + IX_FILE_SUFFIX;
			if ((rc = OpenIndex(str_indexName.c_str(), &curIxEntry.ixIndexHandle))) {
				return rc;
			}
			curIxEntry.attrOffset = attributes[i].attrOffset;
			ixEntrys.push_back(curIxEntry);
		}
	}

	//6. 用保存ix文件句柄，将建立了索引的values插入到对应ix文件.
	if ((rc = InsertRmAndIx(&rmFileHandle, ixEntrys, insertData))) {
		delete[] insertData;
		return rc;
	}
 	delete[] insertData;

	//7. 关闭文件句柄
	if ((rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}
	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		if ((rc = CloseIndex(&(ixEntrys[i].ixIndexHandle)))) {
			return rc;
		}
	}

	return SUCCESS;
}

// 目的：检查属性对relName是否合法。如果合法将属性的类型写入attrType。
// 1.如果bLhsIsAttr为1：
//	  1.1.检查lhsAttr的relName是否和传入的relName相同。不同返回false。
//	  1.2.检查lhsAttr的attrName在relName中是否存在。不同返回false。
// 2.否则返回true。
bool checkAttr(char* relName, int hsIsAttr, RelAttr& hsAttr, AttrType* attrType) {
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	// 1.如果bLhsIsAttr为1：
	if (hsIsAttr) {
		//	  1.1.检查lhsAttr的relName是否和传入的relName相同。不同返回false。
		if ( hsAttr.relName && strcmp(hsAttr.relName, relName)) {
			delete[] rmRecord.pData;
			return false;
		}

		//	  1.2.检查lhsAttr的attrName在relName中是否存在。不同返回false。
		if (hsAttr.attrName && ColumnSearchAttr(relName, hsAttr.attrName, &rmRecord)) {
			delete[] rmRecord.pData;
			return false;
		}
	}

	// 2.否则返回true
	*attrType = (AttrType)(*(int*)(rmRecord.pData + ATTR_TYPE_OFF));
	delete[] rmRecord.pData;
	return true;
}

//
// 目的：检查在relName表上，condition是否合法。
// 1. 如果bLhsIsAttr和bRhsIsAttr都为1，则报错。
// 2. 检查左右属性是否合法。
// 3. 检查比较运算左右类型是否一致。
bool CheckCondition(char* relName, Condition& condition) {
	// 1. 如果bLhsIsAttr和bRhsIsAttr都为0，则报错。
	if (!condition.bLhsIsAttr && !condition.bRhsIsAttr) {
		return false;
	}
	// 2. 检查左右属性是否合法。
	AttrType lType, rType;
	if (!checkAttr(relName, condition.bLhsIsAttr, condition.lhsAttr, &lType) ||
		!checkAttr(relName, condition.bRhsIsAttr, condition.rhsAttr, &rType)) {
		return false;
	}
	// 3. 检查比较运算左右类型是否一致。
	if (!condition.bLhsIsAttr) {
		lType = condition.lhsValue.type;
	}
	if (!condition.bRhsIsAttr) {
		rType = condition.rhsValue.type;
	}
	if (lType != rType) {
		return false;
	}

	return true;
}

//
//目的：该函数用来删除 relName 表中所有满足指定条件的元组以及该元组对应的索
//引项。 如果没有指定条件， 则此方法删除 relName 关系中所有元组。 如果包
//含多个条件， 则这些条件之间为与关系。
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查传入参数的合法性
//	 2.1. 扫描SYSTABLE.如果传入的relName表不存在就报错。
//	 2.2. 检查传入的conditions是否正确。
//3. 利用relName打开RM文件。
//4. 扫描SYSCOLUMNS文件。保存每一个建立了索引的ixIndexHandle.同时计算relName表记录的大小
//5. 将输入的nConditions和conditions转换成RM_FileScan的参数。
//6. 用rm句柄、nConditions和conditions创建rmFileScan。对筛选出的每一项记录进行删除。同时删除可能存在的索引记录。
//7. 关闭文件句柄
RC Delete(char* relName, int nConditions, Condition* conditions) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. 检查传入参数的合法性
	RM_Record rmRecord;
	RC rc;
	int attrCount = 0;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	//	 2.1. 扫描SYSTABLE.如果传入的relName表不存在就报错。
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}
	delete[] rmRecord.pData;

	//	 2.2. 检查传入的conditions是否正确。
	for (int i = 0; i < nConditions; i++) {
		if (!CheckCondition(relName, conditions[i])) {
			return INVALID_CONDITIONS;
		}
	}
	//3. 利用relName打开RM文件。
	std::string rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;//relName+".rm";
	RM_FileHandle rmFileHandle;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		return rc;
	}
	//4. 扫描SYSCOLUMNS文件。保存每一个建立了索引的ixIndexHandle
	std::string str_indexName;
	std::vector<AttrEntry> attributes;
	std::vector<IxEntry> ixEntrys;
	IxEntry curIxEntry;
	int recordSize = 0;

	if ((rc = ColumnEntryGet(relName, &attrCount, attributes))) {
		return rc;
	}
	for (int i = 0; i < attrCount; i++) {
		if (attributes[i].ix_flag) {
			str_indexName = dbInfo.curDbName + "\\" + attributes[i].indexName + IX_FILE_SUFFIX;
			if ((rc = OpenIndex(str_indexName.c_str(), &curIxEntry.ixIndexHandle))) {
				return rc;
			}
			curIxEntry.attrOffset = attributes[i].attrOffset;
			ixEntrys.push_back(curIxEntry);
		}
		recordSize += attributes[i].attrLength;
	}

	//5. 将输入的nConditions和conditions转换成Con类型。
	Con* cons = new Con[nConditions];
	memset(cons, 0, sizeof(Con) * nConditions);
	if ((rc = CreateConFromCondition(relName, nConditions, conditions, cons))) {
		delete[] cons;
		return rc;
	}

	//6. 用rm句柄、nConditions和conditions创建rmFileScan。对筛选出的每一项记录进行删除。同时删除可能存在的索引记录。
	RM_FileScan rmFileScan;
	RM_Record delRecord; //record to be deleted
	delRecord.pData = new char[recordSize];
	memset(delRecord.pData, 0, recordSize);

	if ((rc = OpenScan(&rmFileScan, &rmFileHandle, nConditions, cons))) {
		return rc;
	}

	while ((rc = GetNextRec(&rmFileScan, &delRecord)) == SUCCESS) {
		if ((rc = DeleteRmAndIx(&rmFileHandle, ixEntrys, &delRecord))) {
			delete[] cons;
			delete[] delRecord.pData;
			return rc;
		}
	}
	delete[] cons;
	delete[] delRecord.pData;
	if (rc != RM_EOF || (rc = CloseScan(&rmFileScan))) {
		return rc;
	}

	//6. 关闭文件句柄
	if ((rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}

	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		if ((rc = CloseIndex(&(ixEntrys[i].ixIndexHandle)))) {
			return rc;
		}
	}

	return SUCCESS;
}

//目的：该函数用来在 relName 表中插入具有指定属性值的新元组，
//1. 检查当前是否打开了一个数据库。如果没有则报错。
//2. 检查传入参数的合法性
//	 2.1. 查找relName。如果传入的relName表不存在就报错。
//	 2.2. 扫描SYSCOLUMNS文件。
//		 2.2.1 检查传入value中属性值的类型和长度是否和SYSCOLUMNS记录的相同。
//		 2.2.2 如果传入的value类型是chars，判断values.data长度是否和SYSCOLUMNS记录相同
//   2.3. 检查传入的conditions是否合法。如果合法则将conditions转换成Con结构。
//3. 利用relName打开RM文件.
//4. 如果attrName上建立了索引。则打开对应的ix文件并保存索引。
//5. 扫描RM文件，对每一项符合要求的记录，删除记录后重新插入新的记录。对索引文件进行同步操作。
//6. 关闭文件句柄
RC Update(char* relName, char* attrName, Value* value, int nConditions, Condition* conditions) {
	//1. 检查当前是否打开了一个数据库。如果没有则报错。
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. 检查传入参数的合法性
	RM_Record rmRecord;
	RC rc;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	//	 2.1. 查找relName。如果传入的relName表不存在就报错。
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}
	delete[] rmRecord.pData;

	//	 2.2. 扫描SYSCOLUMNS文件。
	AttrEntry attrEntry;
	if ((rc = ColumnMetaGet(relName, attrName, &attrEntry))) {
		return rc;
	}
	//		 2.2.1 检查传入values中属性值的类型是否和SYSCOLUMNS记录的相同。
	if (value->type != attrEntry.attrType) {
		return INVALID_VALUES;
	}
	//		 2.2.2 如果传入的value类型是chars，判断values.data长度是否和SYSCOLUMNS记录相同
	if ((value->type == chars) && strlen((char*)value->data) >= attrEntry.attrLength) {
		return INVALID_VALUES;
	}

	//   2.3. 检查传入的conditions是否合法。如果合法则将conditions转换成Con结构。
	for (int i = 0; i < nConditions; i++) {
		if (!CheckCondition(relName, conditions[i]))
			return INVALID_CONDITIONS;
	}
	Con* cons = new Con[nConditions];
	memset(cons, 0, sizeof(Con) * nConditions);
	if ((rc = CreateConFromCondition(relName, nConditions, conditions, cons))) {
		return rc;
	}

	//3. 利用relName打开RM文件.
	std::string rmFileName = "";
	RM_FileHandle rmFileHandle;
	rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		return rc;
	}
	//4. 对建立了索引的属性打开对应的ix文件并保存索引。
	std::vector<IxEntry> ixEntrys;
	IxEntry curIxEntry;
	std::string str_indexName;

	if (attrEntry.ix_flag) {
		str_indexName = dbInfo.curDbName + "\\" + attrEntry.indexName + IX_FILE_SUFFIX;
		if ((rc = OpenIndex(str_indexName.c_str(), &curIxEntry.ixIndexHandle))) {
			return rc;
		}
		curIxEntry.attrOffset = attrEntry.attrOffset;
		ixEntrys.push_back(curIxEntry);
	}

	//5. 扫描RM文件，对每一项符合要求的记录，删除记录后重新插入新的记录。对索引文件进行同步操作。
	RM_FileScan rmFileScan;
	RM_Record updateRecord;
	int recordSize;
	GetRecordSize(relName, &recordSize);
	updateRecord.pData = new char[recordSize];
	memset(updateRecord.pData, 0, recordSize);

	if ((rc = OpenScan(&rmFileScan, &rmFileHandle, nConditions, cons))) {
		return rc;
	}

	while ((rc = GetNextRec(&rmFileScan, &updateRecord)) == SUCCESS) {
		if ((rc = DeleteRmAndIx(&rmFileHandle, ixEntrys, &updateRecord))) {
			delete[] updateRecord.pData;
			return rc;
		}
		memcpy(updateRecord.pData + attrEntry.attrOffset, value->data, attrEntry.attrLength);
		if ((rc = InsertRmAndIx(&rmFileHandle, ixEntrys, updateRecord.pData))) {
			delete[] updateRecord.pData;
			return rc;
		}
	}

	delete[] updateRecord.pData;
	if (rc != RM_EOF || (rc = CloseScan(&rmFileScan))) {
		return rc;
	}

	//7. 关闭文件句柄
	if ((rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}

	if (attrEntry.ix_flag && ((rc = CloseIndex(&(ixEntrys[0].ixIndexHandle))))) {
		return rc;
	}

	return SUCCESS;
}


//目的：向SYSTABLES插入一条 relName/attrCount 记录
RC TableMetaInsert(char* relName, int attrCount) {
	RC rc;
	RID rid;

	char insertData[SIZE_SYS_TABLE];
	int* insertAttrCount;

	memset(insertData, 0, SIZE_SYS_TABLE);
	strcpy(insertData, relName);
	insertAttrCount = (int*)(insertData + ATTR_COUNT_OFF);
	*insertAttrCount = attrCount;

	if (rc = InsertRec(&(dbInfo.sysTables), insertData, &rid))
		return rc;

	return SUCCESS;
}

//
// 目的：在SYSTABLE表中删除relName匹配的那一项
// relName: 表名称
// 1. 判断 relName 是否存在, 不存在返回 TABLE_NOT_EXIST
// 2. 如果存在则从SYSTABLE中删除relName匹配的记录，
// 3. 如果存在则删除relName对应的rm文件
RC TableMetaDelete(char* relName) {
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_TABLE];
	memset(rmRecord.pData, 0, SIZE_SYS_TABLE);

	// 1. 判断 relName 是否存在, 不存在返回 TABLE_NOT_EXIST
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}


	// 2. 如果存在则从SYSTABLE中删除relName匹配的记录，
	if ((rc = DeleteRec(&(dbInfo.sysTables), &rmRecord.rid))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//3. 如果存在则删除relName对应的rm文件
	std::string rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if (!DeleteFile((LPCSTR)rmFileName.c_str())) {
		delete[] rmRecord.pData;
		return OS_FAIL;
	}
	delete[] rmRecord.pData;
	return SUCCESS;
}

//
// 目的：在SYSTABLES表中查找relName是否存在。
// relName: 表名称
// 返回值：
// 1. 不存在返回 TABLE_NOT_EXIST
// 2. 存在 返回 SUCCESS
// 
RC TableMetaSearch(char* relName, RM_Record* rmRecord) {
	RM_FileScan rmFileScan;
	RC rc;
	Con cons[1];

	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;
	if ((rc = OpenScan(&rmFileScan, &(dbInfo.sysTables), 1, cons)))
		return rc;
	// 1. 不存在返回 TABLE_NOT_EXIST
	rc = GetNextRec(&rmFileScan, rmRecord);
	if (rc == RM_EOF)
		return TABLE_NOT_EXIST;

	if (rc != SUCCESS)
		return rc;

	if ((rc = CloseScan(&rmFileScan)))
		return rc;

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
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[SIZE_SYS_TABLE];
	memset(rec.pData, 0, SIZE_SYS_TABLE);
	int recnt = 0;

	if (rc = OpenScan(&rmFileScan, &(dbInfo.sysTables), 0, NULL)) {
		delete[] rec.pData;
		return rc;
	}

	printf("Table Meta Table: \n");
	printf("+--------------------+----------+\n");
	printf("|     Table Name     |  AttrCnt |\n");
	printf("+--------------------+----------+\n");
	while ((rc = GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] rec.pData;
			return rc;
		}
		printf("|%-20s|%-10d|\n", rec.pData, *(int*)(rec.pData + ATTR_COUNT_OFF));
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
	cons[1].compOp = EQual; cons[1].LattrLength = SIZE_ATTR_NAME; cons[1].LattrOffset = ATTR_NAME_OFF;
	cons[1].Lvalue = NULL; cons[1].RattrLength = SIZE_ATTR_NAME; cons[1].RattrOffset = 0;
	cons[1].Rvalue = attrName;

	rmRecord->bValid = false;

	if ((rc = OpenScan(&rmFileScan, &(dbInfo.sysColumns), 2, cons)))
		return rc;

	// 扫描SYSCOLUMNS中relName和attrName是否存在。如果不存在则返回ATTR_NOT_EXIST
	rc = GetNextRec(&rmFileScan, rmRecord);
	if (rc == RM_EOF)
		return ATTR_NOT_EXIST;

	if (rc != SUCCESS)
		return rc;

	if ((rc = CloseScan(&rmFileScan)))
		return rc;

	return SUCCESS;
}

//
// 目的：将各个数据项写入pData中，用来向SYSCOLUMNS进行插入
RC ToData(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName, char* pData) {
	memset(pData, 0, SIZE_SYS_COLUMNS);
	memcpy(pData, relName, SIZE_TABLE_NAME);
	memcpy(pData + ATTR_NAME_OFF, attrName, SIZE_ATTR_NAME);
	memcpy(pData + ATTR_TYPE_OFF, &attrType, SIZE_ATTR_TYPE);
	memcpy(pData + ATTR_LENGTH_OFF, &attrLength, SIZE_ATTR_LENGTH);
	memcpy(pData + ATTR_OFFSET_OFF, &attrOffset, SIZE_ATTR_OFFSET);
	memcpy(pData + ATTR_IXFLAG_OFF, &ixFlag, SIZE_IX_FLAG);
	if (indexName)
		memcpy(pData + ATTR_INDEX_NAME_OFF, indexName, SIZE_INDEX_NAME);

	return SUCCESS;
}

//
// 目的：向SYSCOLUMNS中插入一条记录
// 1.检查输入relName的attrName项是否已经存在。如果存在返回ATTR_EXIST
// 2.向SYSCOLUMNS中插入记录值
//	 2.1. 将输入的各项值整合
//	 2.2. 向SYSCOLUMNS表插入
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName) {
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	// 1.检查输入relName的attrName项是否已经存在。如果存在返回ATTR_EXIST
	if ((rc = ColumnSearchAttr(relName, attrName, &rmRecord)) != ATTR_NOT_EXIST) {
		if (rc == SUCCESS) {
			delete[] rmRecord.pData;
			return ATTR_EXIST;
		}
		delete[] rmRecord.pData;
		return rc;
	}

	// 2.向SYSCOLUMNS中插入记录值

	//	 2.1. 将输入的各项值整合
	RID rid;
	char pData[SIZE_SYS_COLUMNS];
	ToData(relName, attrName, attrType, attrLength, attrOffset, ixFlag, indexName, pData);

	//	 2.2. 向SYSCOLUMNS表插入
	if ((rc = InsertRec(&(dbInfo.sysColumns), pData, &rid))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
// 目的：删除SYSCOLUMNS中relName的记录项以及可能存在的ix文件
// 1. 遍历SYSCOLUMNS:
//	  1.1. 删除relName相关的attr记录
//	  1.2. 对于有索引的attr，删除对应的ix文件
RC ColumnMetaDelete(char* relName) {
	RC rc;
	RM_Record rmRecord;
	RM_FileScan rmFileScan;
	Con cons[1];
	RID rid;
	std::string ixFileName;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	// 1. 遍历SYSCOLUMNS:
	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = EQual; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;
	rmRecord.bValid = false;

	if ((rc = OpenScan(&rmFileScan, &(dbInfo.sysColumns), 1, cons))) {
		delete[] rmRecord.pData;
		return rc;
	}


	while ((rc = GetNextRec(&rmFileScan, &rmRecord)) == SUCCESS) {
		//	  1.1. 删除relName相关的attr记录
		if ((rc = DeleteRec(&(dbInfo.sysColumns), &rmRecord.rid))) {
			delete[] rmRecord.pData;
			return rc;
		}

		//	  1.2.判断是否创建了索引。如果创建了索引：删除relName_atrrname对应的ix文件
		if ((*(rmRecord.pData + ATTR_IXFLAG_OFF)) == (char)1) {
			ixFileName = dbInfo.curDbName + "\\";
			ixFileName += (char*)(rmRecord.pData + ATTR_INDEX_NAME_OFF);
			ixFileName += IX_FILE_SUFFIX;
			if (!DeleteFile(ixFileName.c_str())) {
				delete[] rmRecord.pData;
				return SQL_SYNTAX;
			}
		}
	}

	if (rc != RM_EOF) {
		delete[] rmRecord.pData;
		return rc;
	}

	if ((rc = CloseScan(&rmFileScan))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}


//
// 目的：更新SYSCOLUMNS元数据表中的某一项记录 
// 1. 检查attrName是否存在。如果不存在则报错
// 2. 更新SYSCOLUMNS
RC ColumnMetaUpdate(char* relName, char* attrName, bool ixFlag, char* indexName)
{
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);
	char ix_c = (ixFlag) ? 1 : 0;

	if (rc = ColumnSearchAttr(relName, attrName, &rmRecord)) {
		delete[] rmRecord.pData;
		return rc;
	}

	if (ixFlag) {
		*(rmRecord.pData + ATTR_IXFLAG_OFF) = ix_c;
		if (indexName)
			memcpy(rmRecord.pData + ATTR_INDEX_NAME_OFF, indexName, SIZE_INDEX_NAME);

		if (rc = UpdateRec(&(dbInfo.sysColumns), &rmRecord)) {
			delete[] rmRecord.pData;
			return rc;
		}
	}
	
	delete[] rmRecord.pData;
	return SUCCESS;
}


//
// 目的：扫描获取relName表的attrName属性的类型、长度
// 1. 判断attrName是否存在
// 2. 将信息封装进attribute
RC ColumnMetaGet(char* relName, char* attrName, AttrEntry* attribute) {
	// 1. 检查relName和attrName是否存在。如果不存在则报错
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);
	if ((rc = ColumnSearchAttr(relName, attrName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}


	// 2. 将信息封装进attribute
	AttrType* attrType = (AttrType*)(rmRecord.pData + ATTR_TYPE_OFF);
	attribute->attrType = *attrType;
	int* attrLength = (int*)(rmRecord.pData + ATTR_LENGTH_OFF);
	attribute->attrLength = *attrLength;
	attribute->attrOffset = *(int*)(rmRecord.pData + ATTR_OFFSET_OFF);
	attribute->ix_flag = (*(rmRecord.pData + ATTR_IXFLAG_OFF) == (char)1);
	attribute->indexName = rmRecord.pData + ATTR_INDEX_NAME_OFF;

	delete[] rmRecord.pData;
	return SUCCESS;
}

RC ColumnMetaShow()
{
	RC rc;
	RM_FileScan rmFileScan;
	RM_Record rec;
	rec.pData = new char[SIZE_SYS_COLUMNS];
	memset(rec.pData, 0, SIZE_SYS_COLUMNS);
	int recnt = 0;

	if (rc = OpenScan(&rmFileScan, &(dbInfo.sysColumns), 0, NULL)) {
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
	printf("| AttrType ");
	printf("|  AttrLen ");
	printf("|AttrOffset");
	printf("| idx Flag ");
	printf("|     index Name     |\n");
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
	while ((rc = GetNextRec(&rmFileScan, &rec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] rec.pData;
			return rc;
		}
		printf("|%-20s|%-20s|%-10d|%-10d|%-10d|%-10d|%-20s|\n", rec.pData, rec.pData + ATTR_NAME_OFF,
			*(int*)(rec.pData + ATTR_TYPE_OFF), *(int*)(rec.pData + ATTR_LENGTH_OFF),
			*(int*)(rec.pData + ATTR_OFFSET_OFF), (int)(*(rec.pData + ATTR_IXFLAG_OFF)),
			rec.pData + ATTR_INDEX_NAME_OFF);
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

//
// 目的：从SYSCOLUMNS表中获取所有属性的记录信息
//		记录信息按照attrOffset由小到大排序返回。
RC ColumnEntryGet(char* relName, int* attrCount, std::vector<AttrEntry>& attributes)
{
	RC rc;
	RM_Record tableRec;
	tableRec.pData = new char[SIZE_SYS_TABLE];
	memset(tableRec.pData, 0, SIZE_SYS_TABLE);

	if (rc = TableMetaSearch(relName, &tableRec)) {
		delete[] tableRec.pData;
		return rc;
	}

	*attrCount = *(int*)(tableRec.pData + ATTR_COUNT_OFF);
	delete[] tableRec.pData;

	RM_Record columnRec;
	columnRec.pData = new char[SIZE_SYS_COLUMNS];
	memset(columnRec.pData, 0, SIZE_SYS_COLUMNS);

	RM_FileScan columnScan;
	Con cond;

	cond.attrType = chars; cond.bLhsIsAttr = 1; cond.bRhsIsAttr = 0;
	cond.compOp = EQual; cond.LattrLength = SIZE_TABLE_NAME; cond.LattrOffset = TABLE_NAME_OFF;
	cond.Lvalue = NULL; cond.RattrLength = SIZE_TABLE_NAME; cond.RattrOffset = 0;
	cond.Rvalue = relName;

	if (rc = OpenScan(&columnScan, &(dbInfo.sysColumns), 1, &cond)) {
		delete[] columnRec.pData;
		return rc;
	}

	AttrEntry attrEntry;

	while ((rc = GetNextRec(&columnScan, &columnRec)) != RM_EOF) {
		if (rc != SUCCESS) {
			delete[] columnRec.pData;
			return rc;
		}

		attrEntry.attrLength = *(int*)(columnRec.pData + ATTR_LENGTH_OFF);
		attrEntry.attrType = (AttrType)(*(int*)(columnRec.pData + ATTR_TYPE_OFF));
		attrEntry.attrOffset = *(int*)(columnRec.pData + ATTR_OFFSET_OFF);
		attrEntry.ix_flag = (*(columnRec.pData + ATTR_IXFLAG_OFF) == (char)1);
		attrEntry.indexName = columnRec.pData + ATTR_INDEX_NAME_OFF;
		attrEntry.attrName = columnRec.pData + ATTR_NAME_OFF;

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

//
//目的：从relName.rm文件中读取记录，插入到indexName.ix文件中。indexName文件是在
//		relName.rm记录中偏移为attrOffset的属性上建立的索引。
//1. 打开rm文件和ix文件
//2. 扫描rm文件
//3. 将记录插入ix文件
//4. 关闭句柄
RC CreateIxFromTable(char* relName, char* indexName, int attrOffset) {
	RC rc;
	RM_FileHandle rmFileHandle;
	IX_IndexHandle ixIndexHandle;
	RM_Record rmRecord;
	RM_FileScan rmFileScan;
	Con cons[1];
	RID rid;
	std::string rmFileName, ixFileName;
	int recordSize;
	if (rc = GetRecordSize(relName, &recordSize))
		return rc;
	rmRecord.pData = new char[recordSize];
	memset(rmRecord.pData, 0, recordSize);

	//1. 打开rm文件和ix文件
	rmFileName = rmFileName + dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	ixFileName = ixFileName + dbInfo.curDbName + "\\" + indexName + IX_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle)) ||
		(rc = OpenIndex((char*)ixFileName.c_str(), &ixIndexHandle))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//2. 扫描rm文件
	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = NO_OP; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;


	if ((rc = OpenScan(&rmFileScan, &rmFileHandle, 1, cons))) {
		delete[] rmRecord.pData;
		return rc;
	}

	void* pData;
	while ((rc = GetNextRec(&rmFileScan, &rmRecord)) == SUCCESS) {
		//3. 将记录插入ix文件
		pData = (void*)(rmRecord.pData + attrOffset);
		if ((rc = InsertEntry(&ixIndexHandle, pData, &rmRecord.rid))) {
			delete[] rmRecord.pData;
			return rc;
		}
	}

	if (rc != RM_EOF) {
		delete[] rmRecord.pData;
		return rc;
	}

	//4. 关闭句柄
	if ((rc = CloseScan(&rmFileScan)) || (rc = RM_CloseFile(&rmFileHandle)) ||
		(rc = CloseIndex(&ixIndexHandle))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}


//
// 目的：将conditions数组中的信息转换成Con格式的cons数组
RC  CreateConFromCondition(char* relName, int nConditions, Condition* conditions, Con* cons) {
	RC rc;
	AttrEntry attrEntry;
	std::string attrName;
	for (int i = 0; i < nConditions; i++) {
		cons[i].bLhsIsAttr = conditions[i].bLhsIsAttr;
		cons[i].bRhsIsAttr = conditions[i].bRhsIsAttr;
		cons[i].compOp = conditions[i].op;
		cons[i].Lvalue = conditions[i].lhsValue.data;
		cons[i].Rvalue = conditions[i].rhsValue.data;
		if (cons[i].bLhsIsAttr) {
			if ((rc = ColumnMetaGet(relName, conditions[i].lhsAttr.attrName, &attrEntry))) {
				return rc;
			}
			cons[i].attrType = attrEntry.attrType;
			cons[i].LattrLength = attrEntry.attrLength;
			cons[i].LattrOffset = attrEntry.attrOffset;
			cons[i].RattrLength = attrEntry.attrLength;
			cons[i].RattrOffset = attrEntry.attrOffset;
		}
		if (cons[i].bRhsIsAttr) {
			if ((rc = ColumnMetaGet(relName, conditions[i].rhsAttr.attrName, &attrEntry))) {
				return rc;
			}
			cons[i].attrType = attrEntry.attrType;
			cons[i].LattrLength = attrEntry.attrLength;
			cons[i].LattrOffset = attrEntry.attrOffset;
			cons[i].RattrLength = attrEntry.attrLength;
			cons[i].RattrOffset = attrEntry.attrOffset;
		}
	}
	return SUCCESS;
}


//
// 目的：向RM文件插入记录。同时向可能存在的索引文件插入记录。
RC InsertRmAndIx(RM_FileHandle* rmFileHandle, std::vector<IxEntry>& ixEntrys, char* pData) {
	RC rc;
	RID rid;
	if ((rc = InsertRec(rmFileHandle, pData, &rid))) {
		return rc;
	}
	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		if ((rc = InsertEntry(&(ixEntrys[i].ixIndexHandle), pData + ixEntrys[i].attrOffset, &rid))) {
			return rc;
		}
	}
	return SUCCESS;
}

//
// 目的：从rmFilehandle中删除delRecord指定的记录。
//	     同时将记录从所有该表的所有索引文件上删除。
RC DeleteRmAndIx(RM_FileHandle* rmFileHandle, std::vector<IxEntry>& ixEntrys, RM_Record* delRecord) {
	RC rc;
	if ((rc = DeleteRec(rmFileHandle, &delRecord->rid))) {
		return rc;
	}
	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		if ((rc = DeleteEntry(&(ixEntrys[i].ixIndexHandle), delRecord->pData + ixEntrys[i].attrOffset, &delRecord->rid, true))) {
			return rc;
		}
	}
	return SUCCESS;
}

//
// 目的：获取指定表的记录大小
RC GetRecordSize(char* relName, int* recordSize) {
	int attrCount;
	std::vector<AttrEntry> attrEntrys;
	RC rc;
	if ((rc = ColumnEntryGet(relName, &attrCount, attrEntrys))) {
		return rc;
	}
	int getSize = 0;
	for (int i = 0; i < attrCount; i++) {
		getSize += attrEntrys[i].attrLength;
	}
	*recordSize = getSize;
	return SUCCESS;
}

void showRelName(char* relName) {
	printf("Table:%s \n", relName);
	printf("+--------------------");
	printf("+--------------------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+----------");
	printf("+--------------------+\n");
}

void showRecord(RM_Record* rmRecord, std::vector<AttrEntry>& attributes) {
	int attrCount = attributes.size();
	char* pData = rmRecord->pData;
	for (int i = 0; i < attrCount; i++) {
		switch (attributes[i].attrType)
		{
		case chars:
			printf("%s\t", pData + attributes[i].attrOffset);
			break;
		case ints:
			printf("%d\t", *((int*)(pData + attributes[i].attrOffset)));
			break;
		case floats:
			printf("%lf\t", *((float*)(pData + attributes[i].attrOffset)));
			break;
		default:
			break;
		}
	}
	printf("\n");
}

//
// 目的：打印出relName表中的所有记录
RC ShowTable(char* relName) {
	RM_FileHandle rmFileHandle;
	RM_FileScan rmFileScan;
	int recordSize, attrCount;
	RC rc;
	std::vector<AttrEntry> attributes;

	if ((rc = ColumnEntryGet(relName, &attrCount, attributes))) {
		return rc;
	}
	recordSize = 0;
	for (int i = 0; i < attrCount; i++) {
		recordSize += attributes[i].attrLength;
	}

	std::string rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		return rc;
	}

	RM_Record rmRecord;
	rmRecord.pData = new char[recordSize];
	memset(rmRecord.pData, 0, recordSize);
	Con cons[1];
	cons[0].attrType = chars; cons[0].bLhsIsAttr = 1; cons[0].bRhsIsAttr = 0;
	cons[0].compOp = NO_OP; cons[0].LattrLength = SIZE_TABLE_NAME; cons[0].LattrOffset = 0;
	cons[0].Lvalue = NULL; cons[0].RattrLength = SIZE_TABLE_NAME; cons[0].RattrOffset = 0;
	cons[0].Rvalue = relName;

	if ((rc = OpenScan(&rmFileScan, &rmFileHandle, 1, cons))) {
		delete[] rmRecord.pData;
		return rc;
	}

	showRelName(relName);
	int recordNum = 0;
	while ((rc = GetNextRec(&rmFileScan, &rmRecord)) == SUCCESS) {
		showRecord(&rmRecord, attributes);
		recordNum++;
	}
	printf("一共%d条记录\n\n", recordNum);

	delete[] rmRecord.pData;
	if (rc != RM_EOF || (rc = CloseScan(&rmFileScan)) ||
		(rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}

	return SUCCESS;
}

//
// 目的：打印attrName上的索引信息
RC ShowIndex(char* relName, char* attrName, bool def, int cutLen) {
	RC rc;
	AttrEntry attrEntry;
	if ((rc = ColumnMetaGet(relName, attrName, &attrEntry))) {
		return rc;
	}

	IX_IndexHandle ixIndexHandle;
	std::string ixFileName = dbInfo.curDbName + "\\" + attrEntry.indexName + IX_FILE_SUFFIX;
	if ((rc = OpenIndex(ixFileName.c_str(), &ixIndexHandle)) ||
		(rc = printBPlusTree(&ixIndexHandle, ixIndexHandle.fileHeader.rootPage, (def) ? cutLen : attrEntry.attrLength, 0)) ||
		(rc = CloseIndex(&ixIndexHandle))) {
		return rc;
	}

	return SUCCESS;
}