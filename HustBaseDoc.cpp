// HustBaseDoc.cpp : implementation of the CHustBaseDoc class
//

#include "stdafx.h"
#include "HustBase.h"

#include "HustBaseDoc.h"
#include "TreeList.h"
#include "TableList.h"
#include "EditArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHustBaseDoc


IMPLEMENT_DYNCREATE(CHustBaseDoc, CDocument)

BEGIN_MESSAGE_MAP(CHustBaseDoc, CDocument)
	//{{AFX_MSG_MAP(CHustBaseDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHustBaseDoc construction/destruction

CHustBaseDoc::CHustBaseDoc()
	:m_pTreeView(0), m_pListView(0), m_pEditView(0)
{
	// TODO: add one-time construction code here
	isEdit = false;
}

CHustBaseDoc::~CHustBaseDoc()
{
}
bool CHustBaseDoc::isEdit=false;
POSITION CHustBaseDoc::InsertData()
{
 	POSITION pos;//=NULL; //µÈ´ý
 
 	SetModifiedFlag();
 	return pos ;
}
//extern CHustBaseApp theApp;
BOOL CHustBaseDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	if (CHustBaseApp::pathvalue)
	{
		m_pTreeView->PopulateTree();
		
	}
	
	
	return TRUE;
}

CHustBaseDoc * CHustBaseDoc::GetDoc()
{
      CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
      return (CHustBaseDoc *) pFrame->GetActiveDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CHustBaseDoc serialization

void CHustBaseDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHustBaseDoc diagnostics

#ifdef _DEBUG
void CHustBaseDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHustBaseDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHustBaseDoc commands

void CHustBaseDoc::DeleteContents() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDocument::DeleteContents();
}

BOOL CHustBaseDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if (CHustBaseApp::pathvalue)
	{
		m_pTreeView->PopulateTree();
	}
	

	// TODO: Add your specialized creation code here
	
	return TRUE;
}
