// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "HustBase.h"

#include "HustBaseDoc.h"
#include "MainFrm.h"
#include "HustBaseView.h"
#include "EditArea.h"
#include "TreeList.h"
//#include "splitterTabWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.x=50;//改变主窗口的初始位置
	cs.y=50;
	cs.cx=1150;//改变主窗口的初始大小
	cs.cy=700;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

	m_wmSplitter1.CreateStatic(this, 3, 1);
	
	CRect eRect;
	GetClientRect(&eRect);
	

	if (!m_wmSplitter2.CreateStatic(&m_wmSplitter1, 1, 2, WS_CHILD | WS_VISIBLE, m_wmSplitter1.IdFromRowCol(1, 0)))	
	{
		return FALSE;
	}
	if (!m_wmSplitter2.CreateView(0, 0, RUNTIME_CLASS(CTreeList), 
		CSize(4 * eRect.Width()/5, 2 * eRect.Height()/3), pContext))
	{
		return FALSE;
	}
	if (!m_wmSplitter2.CreateView(0, 1, RUNTIME_CLASS(CTableList), CSize(4 * eRect.Width(), 2* eRect.Height()/3), pContext))//显示信息
	{
		return FALSE;
	}
	if (!m_wmSplitter1.CreateView(0, 0, RUNTIME_CLASS(CEditArea), CSize(eRect.Width(), eRect.Height()/6), pContext))	//树状控件
	{
		return FALSE;
	}	
	if (!m_wmSplitter1.CreateView(2, 0, RUNTIME_CLASS(CHustBaseView), CSize(eRect.Width(), eRect.Height()/6), pContext))	//树状控件
	{
		return FALSE;
	}
	m_wmSplitter1.SetRowInfo(0, 50, 0);
	m_wmSplitter1.SetRowInfo(1, 300, 0);
	m_wmSplitter2.SetColumnInfo(0, 150, 0);
	return TRUE;
}
