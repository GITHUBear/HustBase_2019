// HustBaseView.cpp : implementation of the CHustBaseView class
//

#include "stdafx.h"
#include "HustBase.h"

#include "HustBaseDoc.h"
#include "MainFrm.h"
#include "HustBaseView.h"
#include "EditArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHustBaseView

IMPLEMENT_DYNCREATE(CHustBaseView, CScrollView)

BEGIN_MESSAGE_MAP(CHustBaseView, CScrollView)
	//{{AFX_MSG_MAP(CHustBaseView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHustBaseView construction/destruction

CHustBaseView::CHustBaseView()
{
	// TODO: add construction code here

}

CHustBaseView::~CHustBaseView()
{
}

BOOL CHustBaseView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style =cs.style | WS_VSCROLL;
	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CHustBaseView drawing

void CHustBaseView::OnDraw(CDC* pDC)
{
	CHustBaseDoc* pDoc = (CHustBaseDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
// 	if (pDoc->isEdit)		//窗口改变大小时重绘；
// 	{
	int i , j;
	CFont font;
	font.CreatePointFont(118, "微软雅黑", NULL);//设置字体
	CFont *pOldFont = pDC->SelectObject(&font);
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);

	if (pDoc->isEdit)		//窗口改变大小时重绘；
 	{
		POINT point1 = {1,1} ,point2 = {pDoc->selColNum*180+1,1}, 
			point3 = {pDoc->selColNum*180+1,pDoc->selRowNum*25+1}, 
			point4 = {1, pDoc->selRowNum*25+1};
		POINT pframe[5];
			
		pframe[0] = point1;
		pframe[1] = point2;
		pframe[2] = point3;
		pframe[3] = point4;
			
			
		CPen pen1(PS_SOLID, 3, RGB(100, 100, 100));
		CPen * pOldPen = pDC->SelectObject(&pen1);
		pDC->Polyline(pframe, 4);
		pDC->MoveTo(point4);
		pDC->LineTo(point1);
		pDC->SelectObject(pOldPen);
			
	//		CPen pen2(PS_DASH, 1, RGB(200, 200, 200));
			
		POINT pBegin, pEnd;
		for (i = 1; i<pDoc->selRowNum; i++)
		{
			pBegin.x = 1;
			pBegin.y = 25*i+1;
			pEnd.x = pDoc->selColNum*180+1;
			pEnd.y = 25*i+1;
			
			pDC->MoveTo(pBegin);
			pDC->LineTo(pEnd);
		}
		
		for (i = 1; i<pDoc->selColNum; i++)
		{
			pBegin.y = 1;
			pBegin.x = 180*i+1;
			pEnd.y = pDoc->selRowNum*25+1;
			pEnd.x = 180*i+1;
			pDC->MoveTo(pBegin);
			pDC->LineTo(pEnd);
		}	

		for(i = 0; i<pDoc->selRowNum; i++)
			for(j=0; j<pDoc->selColNum; j++)
			{
				pDC->TextOut(8+j*180 , 4+i*25, pDoc->selResult[i][j]);
			}
	}
	else
	{
		for(i = 0; i<pDoc->infoCount; i++)
			pDC->TextOut(1, 20*i, pDoc->Info[i]);
	}
}

void CHustBaseView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) //只有在一开始的时候使用一次，但是之后文档变化了，却没有调用
{
	// TODO: Add your specialized code here and/or call the base class
}
/////////////////////////////////////////////////////////////////////////////
// CHustBaseView printing

BOOL CHustBaseView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CHustBaseView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CHustBaseView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CHustBaseView diagnostics

#ifdef _DEBUG
void CHustBaseView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CHustBaseView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CHustBaseDoc* CHustBaseView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHustBaseDoc)));
	return (CHustBaseDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
void CHustBaseView::OnSize(UINT nType, int cx, int cy)
{
 	this->ShowWindow(SW_MAXIMIZE);
 
 	CWnd * pchild = this->GetWindow(GW_CHILD);
 	if (pchild!=NULL)
 	{
 		CRect rect;
 		this->GetWindowRect(&rect);
 		pchild->ScreenToClient(&rect);
 		pchild->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
 							SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CHustBaseView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();
	// TODO: Add your specialized code here and/or call the base class
	CSize sizeTotal;
	
	sizeTotal.cx = sizeTotal.cy = 200;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CHustBaseView::ShowInfo(UINT ID_TYPE, char* extraInfo)
{
	CString str;
	str.LoadString(ID_TYPE);

	CMainFrame* main = (CMainFrame*)AfxGetApp()->m_pMainWnd;	//获取窗口指针
	CWnd* pPane = (CWnd*)main->m_wmSplitter1.GetPane(2, 0);
	CClientDC dc(pPane);
	
	CRect cr;
	pPane->GetClientRect(cr);
	COLORREF clr=dc.GetBkColor();	
	
	dc.FillSolidRect(cr, clr);	//清空之前输出的文字；
	
	if (str)
	{
		CFont font;
		font.CreatePointFont(118, "微软雅黑", NULL);//设置字体
		CFont *pOldFont = dc.SelectObject(&font);
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		dc.TextOut(0, 0, str);
		dc.TextOut(0, 16, extraInfo);
	}
}
