#if !defined(AFX_TABLELIST_H__C4FFF28A_AAE5_44EB_973D_6757B83A2094__INCLUDED_)
#define AFX_TABLELIST_H__C4FFF28A_AAE5_44EB_973D_6757B83A2094__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TableList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTableList view

class CTableList : public CListView
{
protected:
	CTableList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTableList)

// Attributes
public:

// Operations
public:
	void displayTabInfo(CString ParentNode);
//	void DispTable(const HTREEITEM);
	CHustBaseDoc* GetDocument();
	void ClearList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTableList)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTableList();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTableList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#ifndef _DEBUG  // debug version in SimpleTreeView.cpp
inline CHustBaseDoc* CTableList::GetDocument()
{ return (CHustBaseDoc*)m_pDocument; }
#endif
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLELIST_H__C4FFF28A_AAE5_44EB_973D_6757B83A2094__INCLUDED_)
