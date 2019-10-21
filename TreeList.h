#if !defined(AFX_TREELIST_H__FA3CF1B7_BF63_431D_A0D2_81B7777C67D4__INCLUDED_)
#define AFX_TREELIST_H__FA3CF1B7_BF63_431D_A0D2_81B7777C67D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeList.h : header file
//
class CHustBaseDoc;
/////////////////////////////////////////////////////////////////////////////
// CTreeList view

class CTreeList : public CTreeView
{
protected:
	CTreeList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTreeList)

// Attributes
public:
	CHustBaseDoc* GetDocument();
//	CString path;
// Operations
public:
	bool PathFlag;

public:
	
	static bool openorNot;//=false;
	void PopulateTree();
//	BOOL LocateAndInsert(const CAnimalInfo &, const POSITION);
//µÈ´ý
	void DeleteSelection();
	void EditSelection();
	void ModifySelection();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeList)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTreeList();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	HTREEITEM InsertNode(const HTREEITEM, const CString &str, const DWORD);
	void InsertDBNodes();
	//{{AFX_MSG(CTreeList)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnOpenDB();
	//}}AFX_MSG

//	HTREEITEM FindNode(const HTREEITEM, const CString &) const;
//	HTREEITEM InsertNode(const HTREEITEM, const CString &str, const DWORD);
// 	void ModifyTable(const HTREEITEM selectedNode, const DWORD itemData);
// 	void ModifyType(const HTREEITEM selectedNode, const DWORD itemData);
	
	DECLARE_MESSAGE_MAP()


private:
		CBitmap bm;
};

#ifndef _DEBUG  // debug version in TreeView.cpp
inline CHustBaseDoc* CTreeList::GetDocument()
{ return (CHustBaseDoc*)m_pDocument; }
#endif
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREELIST_H__FA3CF1B7_BF63_431D_A0D2_81B7777C67D4__INCLUDED_)
