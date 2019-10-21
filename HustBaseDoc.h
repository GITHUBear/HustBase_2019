// HustBaseDoc.h : interface of the CHustBaseDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HustBaseDOC_H__D8D49D3D_8A48_4037_84A0_E4124BD0B321__INCLUDED_)
#define AFX_HustBaseDOC_H__D8D49D3D_8A48_4037_84A0_E4124BD0B321__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeList.h"
#include "TableList.h"
#include "EditArea.h"

class CHustBaseDoc : public CDocument
{
protected: // create from serialization only
	CHustBaseDoc();
	DECLARE_DYNCREATE(CHustBaseDoc)

// Attributes
public:
	CString get_text;
	char *Info[5];
	int infoCount; 

	char selResult[101][20][20];
	int selRowNum;
	int selColNum;
//bool isEdit;
static	bool isEdit;
// Operations
public:
	POSITION InsertData ();
	static CHustBaseDoc * GetDoc();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHustBaseDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	CTableList * m_pListView;
	CTreeList * m_pTreeView;
	CEditArea * m_pEditView;

//	CList <CTableInfo, CTableInfo&> m_TableList;
	virtual ~CHustBaseDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
protected:
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CHustBaseDoc)
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HustBaseDOC_H__D8D49D3D_8A48_4037_84A0_E4124BD0B321__INCLUDED_)
