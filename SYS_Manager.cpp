#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>

DB_INFO dbInfo;

void ExecuteAndMessage(char* sql, CEditArea* editArea) {//根据执行的语句类型在界面上显示执行结果。此函数需修改
	std::string s_sql = sql;
	if (s_sql.find("select") == 0) {
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//����ѯ�������һ�£�����������������ʽ
		//����editArea->ShowSelResult(col_num,row_num,fields,rows);
		int col_num = 5;
		int row_num = 3;
		char** fields = new char* [5];
		for (int i = 0; i < col_num; i++) {
			fields[i] = new char[20];
			memset(fields[i], 0, 20);
			fields[i][0] = 'f';
			fields[i][1] = i + '0';
		}
		char*** rows = new char** [row_num];
		for (int i = 0; i < row_num; i++) {
			rows[i] = new char* [col_num];
			for (int j = 0; j < col_num; j++) {
				rows[i][j] = new char[20];
				memset(rows[i][j], 0, 20);
				rows[i][j][0] = 'r';
				rows[i][j][1] = i + '0';
				rows[i][j][2] = '+';
				rows[i][j][3] = j + '0';
			}
		}
		editArea->ShowSelResult(col_num, row_num, fields, rows);
		for (int i = 0; i < 5; i++) {
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	RC rc = execute(sql);
	int row_num = 0;
	char** messages;
	switch (rc) {
	case SUCCESS:
		row_num = 1;
		messages = new char* [row_num];
		messages[0] = "�����ɹ�";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char* [row_num];
		messages[0] = "���﷨����";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char* [row_num];
		messages[0] = "����δʵ��";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	}
}

RC execute(char* sql) {
	sqlstr* sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX

	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////�ж�SQL���Ϊselect���

			//break;

		case 2:
			//�ж�SQL���Ϊinsert���

		case 3:
			//�ж�SQL���Ϊupdate���
			break;

		case 4:
			//�ж�SQL���Ϊdelete���
			break;

		case 5:
			//�ж�SQL���ΪcreateTable���
			break;

		case 6:
			//�ж�SQL���ΪdropTable���
			break;

		case 7:
			//�ж�SQL���ΪcreateIndex���
			break;

		case 8:
			//�ж�SQL���ΪdropIndex���
			break;

		case 9:
			//�ж�Ϊhelp��䣬���Ը���������ʾ
			break;

		case 10:
			//�ж�Ϊexit��䣬�����ɴ˽����˳�����
			break;
		}
	}
	else {
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
		return rc;
	}
}


//
//Ŀ�ģ���·�� dbPath �´���һ����Ϊ dbName �Ŀտ⣬������Ӧ��ϵͳ�ļ���
//1. ���dbpath��dbname�ĺϷ��ԡ�
//2. �ж�dbpath�Լ�dbname�Ƿ��dbInfo�б�����ͬ�������ͬ�����CloseDb��
//3. ��OS������dbpath·�������ļ��С�
//4. ����SYSTABLES�ļ���SYSCOLUMNS�ļ���
RC CreateDB(char* dbpath, char* dbname) {

	RC rc;
	//1. ���dbpath��dbname�ĺϷ��ԡ�
	if (dbpath == NULL || dbname == NULL)
		return DB_NAME_ILLEGAL;

	//2. �ж�dbpath�Լ�dbname�Ƿ��dbInfo�б�����ͬ�������ͬ�����CloseDb��
	if (dbInfo.curDbName.size() && dbInfo.curDbName.compare(dbname) != 0) {
		if ((rc = CloseDB()))
			return rc;
	}

	//3. ��OS������dbpath·�������ļ��С�
	std::string dbPath = dbpath;
	dbPath = dbPath + "\\" + dbname;
	if (CreateDirectory(dbPath.c_str(), NULL)) {
		if (SetCurrentDirectory(dbpath)) {
			//4. ����SYSTABLES�ļ���SYSCOLUMNS�ļ�
			std::string sysTablePath = dbname;
			sysTablePath = sysTablePath + "\\" + TABLE_META_NAME;
			std::string sysColumnsPath = dbname;
			sysColumnsPath = sysColumnsPath + "\\" + COLUMN_META_NAME;
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
//Ŀ�ģ�ɾ�����ݿ��Ӧ��Ŀ¼�Լ�Ŀ¼�µ������ļ�
//      Ĭ�ϲ��������ļ��У����Բ����еݹ鴦��
//1. �ж�dbname�Ƿ�Ϊ�գ�����dbInfo.path���Ƿ����dbname������������򱨴���
//2. �ж�ɾ�����Ƿ��ǵ�ǰDB�������Ҫ�رյ�ǰĿ¼.
//3. �����õ�dbnameĿ¼�µ������ļ�����ɾ��,ע��Ҫ����.��..Ŀ¼
//4. ɾ��dbnameĿ¼
RC DropDB(char* dbname) {

	//1. �ж�dbname�Ƿ�Ϊ�գ�����dbInfo.path���Ƿ����dbname������������򱨴���
	std::string dbPath = dbname;
	if (dbname == NULL || access(dbPath.c_str(), 0) == -1)
		return DB_NOT_EXIST;

	//2. �ж�ɾ�����Ƿ��ǵ�ǰDB
	RC rc;
	if (dbInfo.curDbName.compare(dbname) == 0) {
		//�رյ�ǰĿ¼
		if ((rc = CloseDB()))
			return rc;
	}

	HANDLE hFile;
	WIN32_FIND_DATA  pNextInfo;

	dbPath += "\\*";
	hFile = FindFirstFile(dbPath.c_str(), &pNextInfo);
	if (hFile == INVALID_HANDLE_VALUE)
		return OS_FAIL;

	//3. ����ɾ���ļ����µ������ļ�
	std::string fileName;
	while (FindNextFile(hFile, &pNextInfo)) {
		if (pNextInfo.cFileName[0] == '.')//����.��..Ŀ¼
			continue;
		fileName = dbname;
		fileName = fileName + "\\" + pNextInfo.cFileName;
		if (!DeleteFile(fileName.c_str()))
			return OS_FAIL;
	}

	//4. ɾ��dbnameĿ¼
	if (!RemoveDirectory(dbname))
		return OS_FAIL;

	return SUCCESS;
}

//
//Ŀ�ģ��ı�ϵͳ�ĵ�ǰ���ݿ�Ϊ dbName ��Ӧ���ļ����е����ݿ�
//1. ���dbname�Ƿ�Ϊ�ա�
//2. �����ǰ����db.
//	 2.1 ����򿪵�Ŀ¼��dbname��ͬ���򷵻ء�
//	 2.2 ����رյ�ǰ�򿪵�Ŀ¼��
//3. ��SYSTABLES��SYSCOLUMNS,����dbInfo
//4. ����curDbName
RC OpenDB(char* dbname) {

	RC rc;
	//1. ���dbname�Ƿ�Ϊ�ա�
	if (dbname == NULL)
		return SQL_SYNTAX;

	//2. �����ǰ����db.
	if (dbInfo.curDbName.size()) {
		//	 2.1 ����򿪵�Ŀ¼��dbname��ͬ���򷵻ء�
		if (dbInfo.curDbName.compare(dbname) == 0)
			return SUCCESS;
		//	 2.2 ����رյ�ǰ�򿪵�Ŀ¼��
		if ((rc = CloseDB()))
			return rc;
	}

	//2. ��SYSTABLES��SYSCOLUMNS,����dbInfo
	std::string sysTablePath = dbname;
	sysTablePath = sysTablePath + "\\" + TABLE_META_NAME;
	std::string sysColumnsPath = dbname;
	sysColumnsPath = sysColumnsPath + "\\" + COLUMN_META_NAME;

	if ((rc = RM_OpenFile((char*)sysTablePath.c_str(), &(dbInfo.sysTables))) ||
		(rc = RM_OpenFile((char*)sysColumnsPath.c_str(), &(dbInfo.sysColumns))))
		return rc;

	//3. ����curDbName
	dbInfo.curDbName = dbname;

	return SUCCESS;
}



//
//Ŀ��:�رյ�ǰ���ݿ⡣�رյ�ǰ���ݿ��д򿪵������ļ�
//1. ��鵱ǰ�Ƿ����DB�����curDbNameΪ���򱨴�����������curDbNameΪ�ա�
//2. ���ñ����SYS�ļ����close�������ر�SYS�ļ���
RC CloseDB() {
	//1. ��鵱ǰ�Ƿ����DB�����curDbNameΪ���򱨴�����������curDbNameΪ�ա�
	if (!dbInfo.curDbName.size())
		return SQL_SYNTAX;
	dbInfo.curDbName.clear();

	//2. ���ñ����SYS�ļ����close�������ر�SYS�ļ���
	RM_CloseFile(&(dbInfo.sysTables));
	RM_CloseFile(&(dbInfo.sysColumns));

	return SUCCESS;
}

bool CanButtonClick() {//��Ҫ����ʵ��
	//�����ǰ�����ݿ��Ѿ���
	if (dbInfo.curDbName.size())
		return true;
	//�����ǰû�����ݿ��
	return false;
}

//
//Ŀ�ģ���鴫���attributes�Ƿ������������������������
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
//Ŀ��:����һ����Ϊ relName �ı���
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
//2. �����������ĺϷ��ԣ�
//	 2.1. ���atrrCount�Ƿ���� MAXATTRS��
//	 2.2. ���relName�Ƿ����20���ֽڡ�
//	 2.3. ���relName�Ƿ��SYSTABLES�Լ�SYSCOLUMNSͬ����
//   2.4. ���attributes�Ƿ�Ϸ���
//3. ����Ƿ��Ѿ�����relName��������Ѿ����ڣ��򷵻ش���
//4. ��SYSTABLES��SYSCOLUMNS���д���Ԫ��Ϣ,������relName����¼�Ĵ�С��
//5. ������Ӧ��RM�ļ�
RC CreateTable(char* relName, int attrCount, AttrInfo* attributes) {
	//���� attrCount ��ʾ��ϵ�����Ե�������ȡֵΪ1 �� MAXATTRS ֮�䣩 �� 
	//���� attributes ��һ������Ϊ attrCount �����顣 �����¹�ϵ�е� i �����ԣ�
	//attributes �����еĵ� i ��Ԫ�ذ������ơ� ���ͺ����Եĳ��ȣ��� AttrInfo �ṹ���壩

	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return DB_NOT_EXIST;

	//2. �����������ĺϷ��ԣ�

	//	 2.1. ���atrrCount�Ƿ���� MAXATTRS��
	if (attrCount > dbInfo.MAXATTRS)
		return TOO_MANY_ATTR;
	//	 2.2. ���relName�Ƿ����20���ֽڡ�
	if (strlen(relName) >= SIZE_TABLE_NAME)
		return TABLE_NAME_ILLEGAL;
	//	 2.3. ���relName�Ƿ��SYSTABLES�Լ�SYSCOLUMNSͬ����
	if (strcmp(relName, TABLE_META_NAME) == 0 ||
		strcmp(relName, COLUMN_META_NAME) == 0)
		return TABLE_NAME_ILLEGAL;
	//   2.4. ���attributes�Ƿ�Ϸ���
	if (!attrVaild(attrCount, attributes))
		return INVALID_ATTRS;

	//3. ����Ƿ��Ѿ�����relName��������Ѿ����ڣ��򷵻ش���
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

	//4. ��SYSTABLES��SYSCOLUMNS���д���Ԫ��Ϣ,������relName����¼�Ĵ�С��

	//��SYSTABLES������Ϣ
	if ((rc = TableMetaInsert(relName, attrCount))) {
		//����ʧ��
		delete[] rmRecord.pData;
		return rc;
	}

	//��SYSCOLUMNS������Ϣ
	char indexName[1];//indexNameΪ��
	int rmRecordSize = 0;
	indexName[0] = 0;
	for (int i = 0; i < attrCount; i++) {
		if ((rc = ColumnMetaInsert(relName, attributes[i].attrName, attributes[i].attrType,
			attributes[i].attrLength, rmRecordSize, 0, indexName))) {
			delete[] rmRecord.pData;
			return rc;
		}
		//�����¼�Ĵ�С
		rmRecordSize += attributes[i].attrLength;
	}

	//5. ������Ӧ��RM�ļ�
	//����Լ�����ļ���Ϊ��relName.rm��
	std::string filePath = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_CreateFile((char*)filePath.c_str(), rmRecordSize))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//Ŀ�ģ�������Ϊ relName �ı��Լ��ڸñ��Ͻ���������������
//1. ��鵱ǰ�Ƿ��Ѿ�����һ�����ݿ⣬���û���򱨴���
//2. ���relName�Ƿ��SYSTABLES�Լ�SYSCOLUMNSͬ����
//3. ��SYSTABLES��ɾ��relName��Ӧ�ļ�¼�Լ�relName��Ӧ��rm�ļ�
//4. ɾ��SYSCOLUMNS�еļ�¼���Լ����ܴ��ڵ�ix�ļ�
RC DropTable(char* relName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	RC rc;

	//2. ���relName�Ƿ��SYSTABLES�Լ�SYSCOLUMNSͬ����
	if (strcmp(relName, TABLE_META_NAME) == 0 ||
		strcmp(relName, COLUMN_META_NAME) == 0) {
		return TABLE_NAME_ILLEGAL;
	}

	//3. ��SYSTABLES��ɾ��relName��Ӧ�ļ�¼�Լ�relName��Ӧ��rm�ļ�
	if ((rc = TableMetaDelete(relName))) {
		return rc;
	}

	//4. ɾ��SYSCOLUMNS�еļ�¼���Լ����ܴ��ڵ�ix�ļ�
	if ((rc = ColumnMetaDelete(relName))) {
		return rc;
	}

	return SUCCESS;
}

//�ú����ڹ�ϵ relName ������ attrName �ϴ�����Ϊ indexName ��������
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
//2. �ж����������indexName�����Ƿ�Ϸ���
//3. ����Ƿ��Ѿ����ڶ�Ӧ������������Ѿ������򱨴���
//4. ������Ӧ��������
//	 4.1. ����IX�ļ�
//	 4.2. ����SYS_COLUMNS
//   4.3. ɨ��RM�ļ��������м�¼��Ϣ���뵽������IX�ļ�
RC CreateIndex(char* indexName, char* relName, char* attrName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. �ж����������indexName�����Ƿ�Ϸ���
	if (strlen(indexName) >= SIZE_INDEX_NAME) {
		return INDEX_NAME_ILLEGAL;
	}
	std::string ixFileName = dbInfo.curDbName + "\\" + indexName + IX_FILE_SUFFIX;

	//3. ����Ƿ��Ѿ����ڶ�Ӧ������������Ѿ������򱨴���
	RM_Record rmRecord;
	RC rc;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	if ((rc = ColumnSearchAttr(relName, attrName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}

	char* ix_flag = rmRecord.pData + ATTR_IXFLAG_OFF;//�޷���Scan��Conditions�м���ix_flag���жϡ���ʹ��charsҲ�޷��Ƚϡ�
	if ((*ix_flag) == (char)1) {//�Ѿ������˶�Ӧ������
		delete[] rmRecord.pData;
		return INDEX_EXIST;
	}

	//4. ������Ӧ��������
	//	 4.1. ����IX�ļ�
	int attrType = *((int*)(rmRecord.pData + ATTR_TYPE_OFF));
	int attrLength = *((int*)(rmRecord.pData + ATTR_LENGTH_OFF));
	int attrOffset = *((int*)(rmRecord.pData + ATTR_OFFSET_OFF));
	if ((rc = CreateIndex((char*)ixFileName.c_str(), (AttrType)attrType, attrLength))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//	 4.2. ����SYS_COLUMNS
	if ((rc = ColumnMetaUpdate(relName, attrName, 1, indexName))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//   4.3. ɨ��RM�ļ��������м�¼��Ϣ���뵽������IX�ļ�
	if ((rc = CreateIxFromTable(relName, indexName, attrOffset))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//Ŀ�ģ��ú�������ɾ����Ϊ indexName �������� 
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
//2. ��SYSCOLUMNS�м�������Ƿ���ڡ�����������������Լ����indexname����Ψһ�������в��ң�
//	 2.1 ����������򱨴���
//	 2.2. ɾ��ix�ļ�
//	 2.3.�����޸�SYSCOLUMNS�ж�Ӧ��¼��ix_flagΪ0.

RC DropIndex(char* indexName) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. ��SYSCOLUMNS�м�������Ƿ���ڡ�����������������Լ����indexname����Ψһ�������в��ң�
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

	//	 2.1 ����������򱨴���
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
	if ((*ix_flag) == (char)0) {//û�д���������
		delete[] rmRecord.pData;
		return INDEX_NOT_EXIST;
	}
	if ((rc = CloseScan(&rmFileScan))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//	2.2. ɾ��ix�ļ�
	std::string ixFilePath = dbInfo.curDbName + "\\" + indexName + IX_FILE_SUFFIX;
	if (!DeleteFile((LPCSTR)ixFilePath.c_str())) {
		delete[] rmRecord.pData;
		return OS_FAIL;
	}

	//	 2.3.�����޸�SYSCOLUMNS�ж�Ӧ��¼��ix_flagΪ0.
	ix_flag = rmRecord.pData + ATTR_IXFLAG_OFF;
	*ix_flag = (char)0;
	if ((rc = UpdateRec(&(dbInfo.sysColumns), &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
//Ŀ�ģ��ú��������� relName ���в������ָ������ֵ����Ԫ�飬
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
//2. ��鴫������ĺϷ���
//	 2.1. ɨ��SYSTABLE
//		 2.1.1 ��������relName�������ھͱ�����
//		 2.1.2 ��������򱣴�attrcount�����attrcount�ʹ����nValues��һ���򱨴���
//	 2.2. ɨ��SYSCOLUMNS�ļ���
//		 2.2.1 ��鴫��value������ֵ�����ͺͳ����Ƿ��SYSCOLUMNS��¼����ͬ��
//		 2.2.2 ��������value������chars���ж�values.data�����Ƿ��SYSCOLUMNS��¼��ͬ
//3. ��values����������Ϊһ�����ݡ�
//4. ����relName��RM�ļ�.
//5. ��ÿһ�����潨�������������Դ򿪶�Ӧ��ix�ļ�������������
//6. ��RM�ļ������¼��ͬʱ����ܴ��ڵ������ļ������¼��
//7. �ر��ļ����
RC Insert(char* relName, int nValues, Value* values) {
	//nValues Ϊ����ֵ������ values Ϊ��Ӧ������ֵ���顣 
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. ��鴫������ĺϷ���
	//	 2.1. ɨ��SYSTABLE
	RM_FileScan rmFileScan;
	RM_Record rmRecord;
	RC rc;
	int attrCount = 0;

	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	//		 2.1.1 ��������relName�������ھͱ�����
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//		 2.1.2 ��������򱣴�attrcount�����attrcount�ʹ����nValues��һ���򱨴���
	attrCount = *((int*)(rmRecord.pData + ATTR_COUNT_OFF));
	if (attrCount != nValues) {
		delete[] rmRecord.pData;
		return INVALID_VALUES;
	}

	delete[] rmRecord.pData;

	//	 2.2. ɨ��SYSCOLUMNS�ļ���
	std::vector<AttrEntry> attributes;
	std::string str_indexName;
	int recordSize = 0;
	if ((rc = ColumnEntryGet(relName, &attrCount, attributes))) {
		return rc;
	}

	for (int i = 0; i < attrCount; i++) {
		//		 2.2.1 ��鴫��values������ֵ�������Ƿ��SYSCOLUMNS��¼����ͬ��
		if (values[i].type != attributes[i].attrType) {
			return INVALID_VALUES;
		}
		//		 2.2.2 ��������value������chars���ж�values.data�����Ƿ��SYSCOLUMNS��¼��ͬ
		if ((values[i].type == chars) && strlen((char*)values[i].data) >= attributes[i].attrLength) {
			return INVALID_VALUES;
		}
		recordSize += attributes[i].attrLength;
	}

	//3. ��values����������Ϊһ�����ݡ�
	RID rid;
	char* insertData = new char[recordSize];
	recordSize = 0;
	for (int i = 0; i < attrCount; i++) {
		memcpy(insertData + recordSize, values[i].data, attributes[i].attrLength);
		recordSize += attributes[i].attrLength;
	}

	//4. ����relName��RM�ļ�.
	std::string rmFileName = "";
	RM_FileHandle rmFileHandle;

	rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		delete[] insertData;
		return rc;
	}
	//5. ��ÿһ�����潨�������������Դ򿪶�Ӧ��ix�ļ�������������
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

	//6. �ñ���ix�ļ��������������������values���뵽��Ӧix�ļ�.
	if ((rc = InsertRmAndIx(&rmFileHandle, ixEntrys, insertData))) {
		delete[] insertData;
		return rc;
	}
	delete[] insertData;

	//7. �ر��ļ����
	if ((rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}
	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		curIxEntry = ixEntrys[i];
		if ((rc = CloseIndex(&curIxEntry.ixIndexHandle))) {
			return rc;
		}
	}

	return SUCCESS;
}

// Ŀ�ģ�������Զ�relName�Ƿ�Ϸ�������Ϸ������Ե�����д��attrType��
// 1.���bLhsIsAttrΪ1��
//	  1.1.���lhsAttr��relName�Ƿ�ʹ����relName��ͬ����ͬ����false��
//	  1.2.���lhsAttr��attrName��relName���Ƿ���ڡ���ͬ����false��
// 2.���򷵻�true��
bool checkAttr(char* relName, int hsIsAttr, RelAttr& hsAttr, AttrType* attrType) {
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	// 1.���bLhsIsAttrΪ1��
	if (hsIsAttr) {
		//	  1.1.���lhsAttr��relName�Ƿ�ʹ����relName��ͬ����ͬ����false��
		if (strcmp(hsAttr.relName, relName)) {
			delete[] rmRecord.pData;
			return false;
		}
		//	  1.2.���lhsAttr��attrName��relName���Ƿ���ڡ���ͬ����false��
		if (ColumnSearchAttr(relName, hsAttr.attrName, &rmRecord)) {
			delete[] rmRecord.pData;
			return false;
		}
	}

	// 2.���򷵻�true
	*attrType = (AttrType)(*(int*)(rmRecord.pData + ATTR_TYPE_OFF));
	delete[] rmRecord.pData;
	return true;
}

//
// Ŀ�ģ������relName���ϣ�condition�Ƿ�Ϸ���
// 1. ���bLhsIsAttr��bRhsIsAttr��Ϊ1���򱨴���
// 2. ������������Ƿ�Ϸ���
// 3. ���Ƚ��������������Ƿ�һ�¡�
bool CheckCondition(char* relName, Condition& condition) {
	// 1. ���bLhsIsAttr��bRhsIsAttr��Ϊ0���򱨴���
	if (!condition.bLhsIsAttr && !condition.bRhsIsAttr) {
		return false;
	}
	// 2. ������������Ƿ�Ϸ���
	AttrType lType, rType;
	if (!checkAttr(relName, condition.bLhsIsAttr, condition.lhsAttr, &lType) ||
		!checkAttr(relName, condition.bRhsIsAttr, condition.rhsAttr, &rType)) {
		return false;
	}
	// 3. ���Ƚ��������������Ƿ�һ�¡�
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
//Ŀ�ģ��ú�������ɾ�� relName ������������ָ��������Ԫ���Լ���Ԫ���Ӧ����
//��� ���û��ָ�������� ��˷���ɾ�� relName ��ϵ������Ԫ�顣 �����
//����������� ����Щ����֮��Ϊ���ϵ��
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
//2. ��鴫������ĺϷ���
//	 2.1. ɨ��SYSTABLE.��������relName�������ھͱ�����
//	 2.2. ��鴫���conditions�Ƿ���ȷ��
//3. ����relName��RM�ļ���
//4. ɨ��SYSCOLUMNS�ļ�������ÿһ��������������ixIndexHandle.ͬʱ����relName����¼�Ĵ�С
//5. �������nConditions��conditionsת����RM_FileScan�Ĳ�����
//6. ��rm�����nConditions��conditions����rmFileScan����ɸѡ����ÿһ���¼����ɾ����ͬʱɾ�����ܴ��ڵ�������¼��
//7. �ر��ļ����
RC Delete(char* relName, int nConditions, Condition* conditions) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. ��鴫������ĺϷ���
	RM_Record rmRecord;
	RC rc;
	int attrCount = 0;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	//	 2.1. ɨ��SYSTABLE.��������relName�������ھͱ�����
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}
	delete[] rmRecord.pData;

	//	 2.2. ��鴫���conditions�Ƿ���ȷ��
	for (int i = 0; i < nConditions; i++) {
		if (!CheckCondition(relName, conditions[i])) {
			return INVALID_CONDITIONS;
		}
	}
	//3. ����relName��RM�ļ���
	std::string rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;//relName+".rm";
	RM_FileHandle rmFileHandle;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		return rc;
	}
	//4. ɨ��SYSCOLUMNS�ļ�������ÿһ��������������ixIndexHandle
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

	//5. �������nConditions��conditionsת����Con���͡�
	Con* cons = new Con[nConditions];
	memset(cons, 0, sizeof(Con) * nConditions);
	if ((rc = CreateConFromCondition(relName, nConditions, conditions, cons))) {
		delete[] cons;
		return rc;
	}

	//6. ��rm�����nConditions��conditions����rmFileScan����ɸѡ����ÿһ���¼����ɾ����ͬʱɾ�����ܴ��ڵ�������¼��
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

	//6. �ر��ļ����
	if ((rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}

	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		curIxEntry = ixEntrys[i];
		if ((rc = CloseIndex(&curIxEntry.ixIndexHandle))) {
			return rc;
		}
	}

	return SUCCESS;
}

//Ŀ�ģ��ú��������� relName ���в������ָ������ֵ����Ԫ�飬
//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
//2. ��鴫������ĺϷ���
//	 2.1. ����relName����������relName�������ھͱ�����
//	 2.2. ɨ��SYSCOLUMNS�ļ���
//		 2.2.1 ��鴫��value������ֵ�����ͺͳ����Ƿ��SYSCOLUMNS��¼����ͬ��
//		 2.2.2 ��������value������chars���ж�values.data�����Ƿ��SYSCOLUMNS��¼��ͬ
//   2.3. ��鴫���conditions�Ƿ�Ϸ�������Ϸ���conditionsת����Con�ṹ��
//3. ����relName��RM�ļ�.
//4. ���attrName�Ͻ�������������򿪶�Ӧ��ix�ļ�������������
//5. ɨ��RM�ļ�����ÿһ�����Ҫ��ļ�¼��ɾ����¼�����²����µļ�¼���������ļ�����ͬ��������
//6. �ر��ļ����
RC Update(char* relName, char* attrName, Value* value, int nConditions, Condition* conditions) {
	//1. ��鵱ǰ�Ƿ����һ�����ݿ⡣���û���򱨴���
	if (dbInfo.curDbName.size() == 0)
		return SQL_SYNTAX;

	//2. ��鴫������ĺϷ���
	RM_Record rmRecord;
	RC rc;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	//	 2.1. ����relName����������relName�������ھͱ�����
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}
	delete[] rmRecord.pData;

	//	 2.2. ɨ��SYSCOLUMNS�ļ���
	AttrEntry attrEntry;
	if ((rc = ColumnMetaGet(relName, attrName, &attrEntry))) {
		return rc;
	}
	//		 2.2.1 ��鴫��values������ֵ�������Ƿ��SYSCOLUMNS��¼����ͬ��
	if (value->type != attrEntry.attrType) {
		return INVALID_VALUES;
	}
	//		 2.2.2 ��������value������chars���ж�values.data�����Ƿ��SYSCOLUMNS��¼��ͬ
	if ((value->type == chars) && strlen((char*)value->data) >= attrEntry.attrLength) {
		return INVALID_VALUES;
	}

	//   2.3. ��鴫���conditions�Ƿ�Ϸ�������Ϸ���conditionsת����Con�ṹ��
	for (int i = 0; i < nConditions; i++) {
		if (!CheckCondition(relName, conditions[i]))
			return INVALID_CONDITIONS;
	}
	Con* cons = new Con[nConditions];
	memset(cons, 0, sizeof(Con) * nConditions);
	if ((rc = CreateConFromCondition(relName, nConditions, conditions, cons))) {
		return rc;
	}

	//3. ����relName��RM�ļ�.
	std::string rmFileName = "";
	RM_FileHandle rmFileHandle;
	rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle))) {
		return rc;
	}
	//4. �Խ��������������Դ򿪶�Ӧ��ix�ļ�������������
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

	//5. ɨ��RM�ļ�����ÿһ�����Ҫ��ļ�¼��ɾ����¼�����²����µļ�¼���������ļ�����ͬ��������
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

	//7. �ر��ļ����
	if ((rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}
	if (attrEntry.ix_flag && ((rc = CloseIndex(&curIxEntry.ixIndexHandle)))) {
		return rc;
	}

	return SUCCESS;
}


//Ŀ�ģ���SYSTABLES����һ�� relName/attrCount ��¼
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
// Ŀ�ģ���SYSTABLE����ɾ��relNameƥ�����һ��
// relName: ������
// 1. �ж� relName �Ƿ����, �����ڷ��� TABLE_NOT_EXIST
// 2. ����������SYSTABLE��ɾ��relNameƥ��ļ�¼��
// 3. ���������ɾ��relName��Ӧ��rm�ļ�
RC TableMetaDelete(char* relName) {
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_TABLE];
	memset(rmRecord.pData, 0, SIZE_SYS_TABLE);

	// 1. �ж� relName �Ƿ����, �����ڷ��� TABLE_NOT_EXIST
	if ((rc = TableMetaSearch(relName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}


	// 2. ����������SYSTABLE��ɾ��relNameƥ��ļ�¼��
	if ((rc = DeleteRec(&(dbInfo.sysTables), &rmRecord.rid))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//3. ���������ɾ��relName��Ӧ��rm�ļ�
	std::string rmFileName = dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	if (!DeleteFile((LPCSTR)rmFileName.c_str())) {
		delete[] rmRecord.pData;
		return OS_FAIL;
	}
	delete[] rmRecord.pData;
	return SUCCESS;
}

//
// Ŀ�ģ���SYSTABLES���в���relName�Ƿ���ڡ�
// relName: ������
// ����ֵ��
// 1. �����ڷ��� TABLE_NOT_EXIST
// 2. ���� ���� SUCCESS
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
	// 1. �����ڷ��� TABLE_NOT_EXIST
	rc = GetNextRec(&rmFileScan, rmRecord);
	if (rc == RM_EOF)
		return TABLE_NOT_EXIST;

	if (rc != SUCCESS)
		return rc;

	if ((rc = CloseScan(&rmFileScan)))
		return rc;

	// 2. ���� ���� SUCCESS
	return SUCCESS;
}

// 
// Ŀ�ģ���ӡ��SYSTABLES���ݱ�
// RC ���� RM Scan �����з����Ĵ��� (��ȥ RM_EOF)
// ��� Scan ���� Success
// �����ʽ�����ÿ�Щ������ mysql ������
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
// Ŀ�ģ� ɨ��SYSCOLUMNS��relName��attrName�Ƿ���ڡ�����������򷵻�ATTR_NOT_EXIST
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

	// ɨ��SYSCOLUMNS��relName��attrName�Ƿ���ڡ�����������򷵻�ATTR_NOT_EXIST
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
// Ŀ�ģ�������������д��pData�У�������SYSCOLUMNS���в���
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
// Ŀ�ģ���SYSCOLUMNS�в���һ����¼
// 1.�������relName��attrName���Ƿ��Ѿ����ڡ�������ڷ���ATTR_EXIST
// 2.��SYSCOLUMNS�в����¼ֵ
//	 2.1. ������ĸ���ֵ����
//	 2.2. ��SYSCOLUMNS������
RC ColumnMetaInsert(char* relName, char* attrName, int attrType,
	int attrLength, int attrOffset, bool ixFlag, char* indexName) {
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	// 1.�������relName��attrName���Ƿ��Ѿ����ڡ�������ڷ���ATTR_EXIST
	if ((rc = ColumnSearchAttr(relName, attrName, &rmRecord)) != ATTR_NOT_EXIST) {
		if (rc == SUCCESS) {
			delete[] rmRecord.pData;
			return ATTR_EXIST;
		}
		delete[] rmRecord.pData;
		return rc;
	}

	// 2.��SYSCOLUMNS�в����¼ֵ

	//	 2.1. ������ĸ���ֵ����
	RID rid;
	char pData[SIZE_SYS_COLUMNS];
	ToData(relName, attrName, attrType, attrLength, attrOffset, ixFlag, indexName, pData);

	//	 2.2. ��SYSCOLUMNS������
	if ((rc = InsertRec(&(dbInfo.sysColumns), pData, &rid))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}

//
// Ŀ�ģ�ɾ��SYSCOLUMNS��relName�ļ�¼���Լ����ܴ��ڵ�ix�ļ�
// 1. ����SYSCOLUMNS:
//	  1.1. ɾ��relName��ص�attr��¼
//	  1.2. ������������attr��ɾ����Ӧ��ix�ļ�
RC ColumnMetaDelete(char* relName) {
	RC rc;
	RM_Record rmRecord;
	RM_FileScan rmFileScan;
	Con cons[1];
	RID rid;
	std::string ixFileName;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);

	// 1. ����SYSCOLUMNS:
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
		//	  1.1. ɾ��relName��ص�attr��¼
		if ((rc = DeleteRec(&(dbInfo.sysColumns), &rmRecord.rid))) {
			delete[] rmRecord.pData;
			return rc;
		}

		//	  1.2.�ж��Ƿ񴴽������������������������ɾ��relName_atrrname��Ӧ��ix�ļ�
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
// Ŀ�ģ�����SYSCOLUMNSԪ���ݱ��е�ĳһ���¼ 
// 1. ���attrName�Ƿ���ڡ�����������򱨴�
// 2. ����SYSCOLUMNS
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
// Ŀ�ģ�ɨ���ȡrelName����attrName���Ե����͡�����
// 1. �ж�attrName�Ƿ����
// 2. ����Ϣ��װ��attribute
RC ColumnMetaGet(char* relName, char* attrName, AttrEntry* attribute) {
	// 1. ���relName��attrName�Ƿ���ڡ�����������򱨴�
	RC rc;
	RM_Record rmRecord;
	rmRecord.pData = new char[SIZE_SYS_COLUMNS];
	memset(rmRecord.pData, 0, SIZE_SYS_COLUMNS);
	if ((rc = ColumnSearchAttr(relName, attrName, &rmRecord))) {
		delete[] rmRecord.pData;
		return rc;
	}


	// 2. ����Ϣ��װ��attribute
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
// Ŀ�ģ���SYSCOLUMNS���л�ȡ�������Եļ�¼��Ϣ
//		��¼��Ϣ����attrOffset��С�������򷵻ء�
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
//Ŀ�ģ���relName.rm�ļ��ж�ȡ��¼�����뵽indexName.ix�ļ��С�indexName�ļ�����
//		relName.rm��¼��ƫ��ΪattrOffset�������Ͻ�����������
//1. ��rm�ļ���ix�ļ�
//2. ɨ��rm�ļ�
//3. ����¼����ix�ļ�
//4. �رվ��
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

	//1. ��rm�ļ���ix�ļ�
	rmFileName = rmFileName + dbInfo.curDbName + "\\" + relName + RM_FILE_SUFFIX;
	ixFileName = ixFileName + dbInfo.curDbName + "\\" + indexName + IX_FILE_SUFFIX;
	if ((rc = RM_OpenFile((char*)rmFileName.c_str(), &rmFileHandle)) ||
		(rc = OpenIndex((char*)ixFileName.c_str(), &ixIndexHandle))) {
		delete[] rmRecord.pData;
		return rc;
	}

	//2. ɨ��rm�ļ�
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
		//3. ����¼����ix�ļ�
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

	//4. �رվ��
	if ((rc = CloseScan(&rmFileScan)) || (rc = RM_CloseFile(&rmFileHandle)) ||
		(rc = CloseIndex(&ixIndexHandle))) {
		delete[] rmRecord.pData;
		return rc;
	}

	delete[] rmRecord.pData;
	return SUCCESS;
}


//
// Ŀ�ģ���conditions�����е���Ϣת����Con��ʽ��cons����
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
// Ŀ�ģ���RM�ļ������¼��ͬʱ����ܴ��ڵ������ļ������¼��
RC InsertRmAndIx(RM_FileHandle* rmFileHandle, std::vector<IxEntry>& ixEntrys, char* pData) {
	RC rc;
	RID rid;
	if ((rc = InsertRec(rmFileHandle, pData, &rid))) {
		return rc;
	}
	int numIndexs = ixEntrys.size();
	IxEntry curIxEntry;
	for (int i = 0; i < numIndexs; i++) {
		curIxEntry = ixEntrys[i];
		if ((rc = InsertEntry(&curIxEntry.ixIndexHandle, pData + curIxEntry.attrOffset, &rid))) {
			return rc;
		}
	}
	return SUCCESS;
}

//
// Ŀ�ģ���rmFilehandle��ɾ��delRecordָ���ļ�¼��
//	     ͬʱ����¼�����иñ������������ļ���ɾ����
RC DeleteRmAndIx(RM_FileHandle* rmFileHandle, std::vector<IxEntry>& ixEntrys, RM_Record* delRecord) {
	RC rc;
	if ((rc = DeleteRec(rmFileHandle, &delRecord->rid))) {
		return rc;
	}
	IxEntry curIxEntry;
	int numIndexs = ixEntrys.size();
	for (int i = 0; i < numIndexs; i++) {
		curIxEntry = ixEntrys[i];
		if ((rc = DeleteEntry(&curIxEntry.ixIndexHandle, delRecord->pData + curIxEntry.attrOffset, &delRecord->rid, true))) {
			return rc;
		}
	}
	return SUCCESS;
}

//
// Ŀ�ģ���ȡָ�����ļ�¼��С
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
// Ŀ�ģ���ӡ��relName���е����м�¼
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
	printf("һ��%d����¼\n\n", recordNum);

	delete[] rmRecord.pData;
	if (rc != RM_EOF || (rc = CloseScan(&rmFileScan)) ||
		(rc = RM_CloseFile(&rmFileHandle))) {
		return rc;
	}

	return SUCCESS;
}

//
// Ŀ�ģ���ӡattrName�ϵ�������Ϣ
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