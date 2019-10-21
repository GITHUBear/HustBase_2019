// HustBaseView.h : interface of the CHustBaseView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HustBaseVIEW_H__EA1E3FD5_D6CA_479E_93B9_3ED67C25283A__INCLUDED_)
#define AFX_HustBaseVIEW_H__EA1E3FD5_D6CA_479E_93B9_3ED67C25283A__INCLUDED_

#include "EditArea.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEditArea;
//class CScrollView;

class CHustBaseView : public CScrollView
{
public: // create from serialization only
	CHustBaseView();
	virtual ~CHustBaseView();
	DECLARE_DYNCREATE(CHustBaseView)

// Attributes
public:
	CHustBaseDoc* GetDocument();
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHustBaseView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ShowInfo(UINT ID_TYPE, char* extraInfo);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:

	//{{AFX_MSG(CHustBaseView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in HustBaseView.cpp
inline CHustBaseDoc* CHustBaseView::GetDocument()
   { return (CHustBaseDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HustBaseVIEW_H__EA1E3FD5_D6CA_479E_93B9_3ED67C25283A__INCLUDED_)
