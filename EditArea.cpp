// EditArea.cpp : implementation file
//

#include "stdafx.h"

#include "str.h"

#include "HustBaseDoc.h"
#include "MainFrm.h"
#include "HustBase.h"
#include "EditArea.h"
#include "HustBaseView.h"
#include "string.h"
#include "stdio.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditArea


IMPLEMENT_DYNCREATE(CEditArea, CEditView)

CEditArea::CEditArea()
{
}

CEditArea::~CEditArea()
{
}

BEGIN_MESSAGE_MAP(CEditArea, CEditView)
	//{{AFX_MSG_MAP(CEditArea)
	ON_WM_CREATE()
	ON_COMMAND(ID_RUN, OnRunBtn)
	ON_COMMAND(ID_FILE_OPEN, OnOpenSqlFile)
	ON_UPDATE_COMMAND_UI(ID_RUN, OnUpdateRun)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CEditArea::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (-1 == CEditView::OnCreate(lpCreateStruct))
	{
		return -1;
	}
	GetDocument()->m_pEditView = this;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CEditArea drawing

void CEditArea::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CEditArea diagnostics

#ifdef _DEBUG
void CEditArea::AssertValid() const
{
	CEditView::AssertValid();
}

void CEditArea::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CHustBaseDoc* CEditArea::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHustBaseDoc)));
	return (CHustBaseDoc*)m_pDocument;
}
#endif //_DEBUG

void CEditArea::OnSize(UINT nType, int cx, int cy)
{
	this->ShowWindow(SW_MAXIMIZE);
	CWnd* pChild = this->GetWindow(GW_CHILD);
	if (pChild != NULL)
	{
		CRect rect;
		this->GetWindowRect(&rect);
		pChild->ScreenToClient(&rect);
		pChild->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
			SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void CEditArea::OnChange()
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CEditView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO: Add your control notification handler code here
	GetEditCtrl().SetModify(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// CEditArea message handlers
void CEditArea::OnRunBtn()
{
	// TODO: Add your command handler code here
//	HWND hWnd;	
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();
	CString analyzeResult;

	sqlstr* sql_str = NULL;	//�������ϱ�����FLAG�Ľṹ�����

	RC rc;

	CMainFrame* main = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CWnd* pPaneShow = (CWnd*)main->m_wmSplitter1.GetPane(0, 0);

	pPaneShow->GetWindowText(pDoc->get_text);
	char* str = (LPSTR)(LPCTSTR)pDoc->get_text;


	pDoc->isEdit = false;
	ExecuteAndMessage(str, this);//���ԶԴ˺��������޸�������ҳ��չʾ����Ϣ
	
	pDoc = CHustBaseDoc::GetDoc();
	CHustBaseApp::pathvalue = true;

	pDoc->m_pTreeView->PopulateTree();
}

void CEditArea::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();

}

void CEditArea::OnOpenSqlFile()
{
	//�������ļ���ť���˴�Ӧ��ʾ�û������ļ�����λ�ã�������OpenSqlFile����
	//ʹ���ļ���������ڻ���ļ���Ŀ¼
	//�����ļ���������Ի���û��ļ�·������
	CFileDialog dialog(TRUE);    //����Ҫ�趨Ĭ��·���Ȳ���
	dialog.m_ofn.lpstrTitle = "��ѡ���ļ���λ��";
	if (dialog.DoModal() == IDOK)
	{
		CString filePath = dialog.GetPathName();
		//��CString��������ת��Ϊchar*��������
		std::string str_filePath = filePath;
		OpenSqlFile((char*)str_filePath.c_str(),this);
		CHustBaseDoc* pDoc;
		pDoc = CHustBaseDoc::GetDoc();
		CHustBaseApp::pathvalue = true;

		pDoc->m_pTreeView->PopulateTree();
	}
}

void CEditArea::OnUpdateRun(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CString testText;
	CMainFrame* main = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CWnd* pPaneShow = main->m_wmSplitter1.GetPane(0, 0);

	pPaneShow->GetWindowText(testText);

	if (0 == testText.GetLength())
	{

		pCmdUI->Enable(FALSE);
	}
	else {
		pCmdUI->Enable(CanButtonClick());
	}
}


void CEditArea::displayInfo()
{
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();
	CMainFrame* main = (CMainFrame*)AfxGetApp()->m_pMainWnd;	//��ȡ����ָ��
	CWnd* pPane = (CWnd*)main->m_wmSplitter1.GetPane(2, 0);
	CClientDC dc(pPane);

	CRect cr;
	pPane->GetClientRect(cr);
	COLORREF clr = dc.GetBkColor();

	dc.FillSolidRect(cr, clr);	//���֮ǰ��������֣�

	CFont font;
	font.CreatePointFont(118, "΢���ź�", NULL);//��������
	CFont* pOldFont = dc.SelectObject(&font);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	for (int i = 0; i < pDoc->infoCount; i++)
	{
		dc.TextOut(0, 20 * i, pDoc->Info[i]);
	}
}

int CEditArea::iReadDictstruct(char tabname[][20], int* tabnum, char colname[][20][20], int colnum[], AttrType coltype[][20], int collength[][20], int coloffset[][20], int iscolindex[][20])//��д�µĴ���ϵͳ���ϵͳ����Ϣ�ĺ���
{
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();

	RM_FileHandle fileHandle, colfilehandle;
	fileHandle.bOpen = 0;
	colfilehandle.bOpen = 0;
	RM_FileScan FileScan1, FileScan2;
	FileScan1.bOpen = 0;
	FileScan2.bOpen = 0;
	RM_Record rec1, rec2;
	Con condition;
	RC rc;
	CString t;//test

	int i = 0, j = 0;
	DWORD cchCurDir = BUFFER;
	LPTSTR lpszCurDir;
	TCHAR tchBuffer[BUFFER];
	lpszCurDir = tchBuffer;
	GetCurrentDirectory(cchCurDir, lpszCurDir);

	CString	Path = lpszCurDir;

	CString table = Path + "\\SYSTABLES";
	CString column = Path + "\\SYSCOLUMNS";

	rc = RM_OpenFile((LPSTR)(LPCTSTR)table, &fileHandle);//ȥSYSTABLES���л�ȡ����
	if (rc != SUCCESS)
		AfxMessageBox("��ϵͳ���ļ�ʧ��");
	rc = RM_OpenFile((LPSTR)(LPCTSTR)column, &colfilehandle);//ȥSYSCOLUMNS���л�ȡ����
	if (rc != SUCCESS)
		AfxMessageBox("��ϵͳ���ļ�ʧ��");
	rc = OpenScan(&FileScan1, &fileHandle, 0, NULL);
	if (rc != SUCCESS)
		AfxMessageBox("��ʼ�����ļ�ɨ��ʧ��");

	rec1.pData = new char[SIZE_SYS_TABLE];
	memset(rec1.pData, 0, SIZE_SYS_TABLE);
	rec2.pData = new char[SIZE_SYS_COLUMNS];
	memset(rec2.pData, 0, SIZE_SYS_COLUMNS);
	while (GetNextRec(&FileScan1, &rec1) == SUCCESS)
	{
		strcpy(tabname[i], rec1.pData);
		condition.bLhsIsAttr = 1;
		condition.bRhsIsAttr = 0;
		//	condition.LattrLength=strlen(tabname[i])+1;
		condition.LattrOffset = 0;
		condition.attrType = chars;
		condition.compOp = EQual;
		condition.Rvalue = tabname[i];
		rc = OpenScan(&FileScan2, &colfilehandle, 1, &condition);
		if (rc != SUCCESS) {
			AfxMessageBox("��ʼ�����ļ�ɨ��ʧ��");
		}

		while (GetNextRec(&FileScan2, &rec2) == SUCCESS)
		{
			strcpy(colname[i][j], rec2.pData + 21);
			memcpy(&coltype[i][j], rec2.pData + 42, sizeof(AttrType));
			memcpy(&collength[i][j], rec2.pData + 46, sizeof(int));
			memcpy(&coloffset[i][j], rec2.pData + 50, sizeof(int));
			if (*(rec2.pData + 54) == '1')
				iscolindex[i][j] = 1;
			else
				iscolindex[i][j] = 0;
			j++;
		}
		colnum[i] = j;
		j = 0;
		i++;
		FileScan2.bOpen = 0;
	}
	*tabnum = i;
	rc = RM_CloseFile(&fileHandle);
	if (rc != SUCCESS) {
		AfxMessageBox("�ر�ϵͳ���ļ�ʧ��");
	}
	rc = RM_CloseFile(&colfilehandle);
	if (rc != SUCCESS) {
		AfxMessageBox("�ر�ϵͳ���ļ�ʧ��");
	}
	delete[] rec1.pData;
	delete[] rec2.pData;
	return 1;
}

void CEditArea::ShowSelResult(int col_num, int row_num, char** fields, char*** result) {
	row_num = min(row_num, 100);
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();
	for (int i = 0; i < col_num; i++) {
		memcpy(pDoc->selResult[0][i], fields[i], 20);
	}
	for (int i = 0; i < row_num; i++) {
		for (int j = 0; j < col_num; j++) {
			memcpy(pDoc->selResult[i + 1][j], result[i][j], 20);
		}
	}
	this->showSelResult(1 + row_num, col_num);
}

void CEditArea::showSelResult(int row_num, int col_num)
{
	int i, j;
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();	//ʵ�ָ�����ͨ�ţ�ͨ���ĵ���Ϣ

	CMainFrame* main = (CMainFrame*)AfxGetApp()->m_pMainWnd;	//��ȡ����ָ��
	CWnd* pPane = (CWnd*)main->m_wmSplitter1.GetPane(2, 0);
	CClientDC dc(pPane);

	CRect cr;
	pPane->GetClientRect(cr);
	COLORREF clr = dc.GetBkColor();

	dc.FillSolidRect(cr, clr);	//���֮ǰ��������֣�

	CFont font;
	font.CreatePointFont(118, "΢���ź�", NULL);//��������
	CFont* pOldFont = dc.SelectObject(&font);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	POINT point1 = { 1,1 }, point2 = { col_num * 180 + 1,1 },
		point3 = { col_num * 180 + 1,row_num * 25 + 1 },
		point4 = { 1, row_num * 25 + 1 };
	POINT pframe[5];

	pframe[0] = point1;
	pframe[1] = point2;
	pframe[2] = point3;
	pframe[3] = point4;


	CPen pen1(PS_SOLID, 3, RGB(100, 100, 100));
	CPen* pOldPen = dc.SelectObject(&pen1);
	dc.Polyline(pframe, 4);
	dc.MoveTo(point4);
	dc.LineTo(point1);
	dc.SelectObject(pOldPen);

	POINT pBegin, pEnd;
	for (i = 1; i < row_num; i++)
	{
		pBegin.x = 1;
		pBegin.y = 25 * i + 1;
		pEnd.x = col_num * 180 + 1;
		pEnd.y = 25 * i + 1;

		dc.MoveTo(pBegin);
		dc.LineTo(pEnd);
	}

	for (i = 1; i < col_num; i++)
	{
		pBegin.y = 1;
		pBegin.x = 180 * i + 1;
		pEnd.y = row_num * 25 + 1;
		pEnd.x = 180 * i + 1;
		dc.MoveTo(pBegin);
		dc.LineTo(pEnd);
	}

	for (i = 0; i < row_num; i++)
		for (j = 0; j < col_num; j++)
		{
			dc.TextOut(8 + j * 180, 4 + i * 25, pDoc->selResult[i][j]);
		}

}

/*
0<=count<=5
��Ϣ��಻�ܳ�������
*/
void CEditArea::ShowMessage(int count, char* strs[]) {
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();
	pDoc->infoCount = count;
	for (int i = 0; i < count && i < 5; i++) {
		pDoc->Info[i] = strs[i];
	}
	displayInfo();
}