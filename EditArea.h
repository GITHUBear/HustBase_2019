#if !defined(AFX_EDITAREA_H__77474671_9C31_4BFD_A1BA_455CD4D98828__INCLUDED_)
#define AFX_EDITAREA_H__77474671_9C31_4BFD_A1BA_455CD4D98828__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditArea.h : header file

#include "str.h"

class CHustBaseDoc;

/////////////////////////////////////////////////////////////////////////////
// CEditArea view

class CEditArea : public CEditView
{
public:
	CEditArea();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditArea)

// Attributes
public:
	virtual ~CEditArea();
	
	//select.c 里的函数

	//void	readdic(struct tab_dic_type *tab,int *tab_num,struct col_dic_type *col,int *col_num)


// Operations
public:
	void ShowMessage(int,char**);
	void ShowSelResult(int col_num,int row_num,char ** fields,char *** result);
	void showSelResult(int row_num, int col_num);
	int iReadDictstruct(char tabname[][20],int *tabnum,char colname[][20][20],int colnum[],AttrType coltype[][20],int collength[][20],int coloffset[][20],int iscolindex[][20]);
	void displayInfo();
	CHustBaseDoc* GetDocument();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditArea)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
//	virtual ~CEditArea();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditArea)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnRunBtn();
	afx_msg void OnUpdateRun(CCmdUI* pCmdUI);
	afx_msg void OnChange();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in HustBaseView.cpp
inline CHustBaseDoc* CEditArea::GetDocument()
{ return (CHustBaseDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
//以上都是CEditArea的内容。
/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITAREA_H__77474671_9C31_4BFD_A1BA_455CD4D98828__INCLUDED_)
#ifndef __EditArea_H_
#define __EditArea_H_

#endif
