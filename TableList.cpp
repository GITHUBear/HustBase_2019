// TableList.cpp : implementation file
//

#include "stdafx.h"
#include "HustBaseDoc.h"
#include "HustBase.h"
#include "TableList.h"
#include "EditArea.h"
#include "RM_Manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTableList

IMPLEMENT_DYNCREATE(CTableList, CListView)

CTableList::CTableList()
{
}

CTableList::~CTableList()
{
}


BEGIN_MESSAGE_MAP(CTableList, CListView)
	//{{AFX_MSG_MAP(CTableList)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CTableList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	GetDocument()->m_pListView = this;

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CTableList drawing

void CTableList::OnDraw(CDC* pDC)
{
	CHustBaseDoc* pDoc = GetDocument();
	ASSERT(pDoc);
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CTableList diagnostics

#ifdef _DEBUG
void CTableList::AssertValid() const
{
	CListView::AssertValid();
}

void CTableList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CHustBaseDoc* CTableList::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHustBaseDoc)));
	return (CHustBaseDoc*)m_pDocument;
}
#endif //_DEBUG


void CTableList::ClearList()
{
	CListCtrl &ctrlList = GetListCtrl();
	ctrlList.DeleteAllItems();
	while (ctrlList.DeleteColumn(0));
	UpdateWindow();
}

void CTableList::OnInitialUpdate() 
{
	GetListCtrl().ModifyStyle(NULL, LVS_REPORT);
	CListView::OnInitialUpdate();
}

void CTableList::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
	// TODO: Add your specialized code here and/or call the base class
	GetListCtrl().ModifyStyle(NULL, LVS_REPORT);
	CListView::OnInitialUpdate();	
	CListView::CalcWindowRect(lpClientRect, nAdjustType);
}


void CTableList::displayTabInfo(CString ParentNode)
{
	RM_FileHandle fileHandle;
	fileHandle.bOpen=0;
	RC rc;
	RM_Record rec;
	RM_FileScan FileScan;
	FileScan.bOpen=0;
	Con condition;

	char tabname[20][20];
	char colname[20][20][20];
	AttrType coltype[20][20];
	int tabnum,colnum[20];
	int collength[20][20];
	int coloffset[20][20];
	int iscolix[20][20];
	int i,j,k=0;
	CString path;
	CString t;

	DWORD cchCurDir; 
 	LPTSTR lpszCurDir; 	
 	TCHAR tchBuffer[BUFFER]; 
 	lpszCurDir = tchBuffer; 
 	GetCurrentDirectory(cchCurDir, lpszCurDir); 
 	path=lpszCurDir;
 	path+="\\SYSTABLES.xx";

	rc=RM_OpenFile((LPSTR)(LPCTSTR)path,&fileHandle);//去SYSTABLES表中获取表名
	if(rc!=SUCCESS)
		AfxMessageBox("打开系统表文件失败");
	condition.bLhsIsAttr=1;
	condition.bRhsIsAttr=0;
	condition.LattrOffset=0;
	condition.attrType=chars;
	condition.compOp=EQual;
	condition.Rvalue=(LPSTR)(LPCTSTR)ParentNode;

	rc=OpenScan(&FileScan,&fileHandle,1,&condition);
	if(rc!=SUCCESS)
		AfxMessageBox("初始化文件扫描失败");
	rc=GetNextRec(&FileScan,&rec);
	if(rc!=SUCCESS)//点击的不是表名，不做显示处理
	{
		rc=RM_CloseFile(&fileHandle);
		if(rc!=SUCCESS)
			AfxMessageBox("系统表文件关闭失败");
		return;
	}
	rc=RM_CloseFile(&fileHandle);
	if(rc!=SUCCESS)
		AfxMessageBox("表系统文件关闭失败");

 	CHustBaseDoc *pDoc = GetDocument();
 	CListCtrl & clc = GetListCtrl();
// 	CTreeCtrl & ctc = GetDocument()->m_pTreeView->GetTreeCtrl();
 		
 	LV_COLUMN lv;
 	lv.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
 	lv.fmt = LVCFMT_CENTER;
 	
	pDoc->m_pEditView->iReadDictstruct(tabname,&tabnum,colname,colnum,coltype,collength,coloffset,iscolix);//调用iReadDictstruct函数获取表信息和列信息
	for(i=0;i<tabnum;i++)
	{
		if(strcmp((LPSTR)(LPCTSTR)ParentNode,tabname[i])==0)
			break;
	}
	ClearList();
 	for(j=1;j<=colnum[i];j++)//绘制表头，这里从1开始是为了解决第一列必须左对齐的问题，若从0开始则第一列必定是左对齐的
 	{
		lv.cx=15*12;
 		lv.pszText=colname[i][j-1];
		lv.fmt=LVCFMT_CENTER;
 		clc.InsertColumn(j,&lv);
 	}
	fileHandle.bOpen=0;
	rc=RM_OpenFile((LPSTR)(LPCTSTR)ParentNode,&fileHandle);
	if(rc!=SUCCESS)
		AfxMessageBox("数据表文件打开失败");
	FileScan.bOpen=0;
	rc=OpenScan(&FileScan,&fileHandle,0,NULL);
	if(rc!=SUCCESS)
		AfxMessageBox("初始化文件扫描失败");
	while(GetNextRec(&FileScan,&rec)==SUCCESS)
	{
		clc.InsertItem(k,"");
		for(j=0;j<colnum[i];j++)
		{
			if(coltype[i][j]==chars)
			{
				char temp[21];
				memcpy(temp,rec.pData+coloffset[i][j],collength[i][j]);
				t=temp;				
				clc.SetItemText(k,j,t);
			}
			else if(coltype[i][j]==ints)
			{
				int temp;
				memcpy(&temp,rec.pData+coloffset[i][j],sizeof(int));
				t.Format("%d",temp);
				clc.SetItemText(k,j,t);
			}
			else if(coltype[i][j]==floats)
			{
				float temp;
				memcpy(&temp,rec.pData+coloffset[i][j],sizeof(float));
				t.Format("%f",temp);
				clc.SetItemText(k,j,t);
			}
		}
		k++;
	}
	rc=RM_CloseFile(&fileHandle);
	if(rc!=SUCCESS)
		AfxMessageBox("关闭数据表文件失败");
}