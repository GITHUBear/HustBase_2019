// TreeList.cpp : implementation file
//

#include "stdafx.h"

#include "HustBaseDoc.h"
#include "HustBase.h"

#include "TreeList.h"
#include "TableList.h"
#include "EditArea.h"
#include "str.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeList
class HustBase;
IMPLEMENT_DYNCREATE(CTreeList, CTreeView)

CTreeList::CTreeList()
{
}

CTreeList::~CTreeList()
{
}

bool CTreeList::openorNot=true;
BEGIN_MESSAGE_MAP(CTreeList, CTreeView)
	//{{AFX_MSG_MAP(CTreeList)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
//	ON_COMMAND(ID_OPENDB, OnOpenDB)
//	ON_UPDATE_COMMAND_UI(ID_OPENDB, OnUpdateDB)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

int CTreeList::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CTreeView::OnCreate(lpcs)==-1)
	{
		return -1;
	}
	GetDocument()->m_pTreeView = this;	
	
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CTreeList drawing

void CTreeList::OnDraw(CDC* pDC)
{
	CHustBaseDoc* pDoc = GetDocument();
	ASSERT(pDoc);
	// TODO: add draw code here
}


/////////////////////////////////////////////////////////////////////////////
// CTreeList diagnostics

#ifdef _DEBUG
void CTreeList::AssertValid() const
{
	CTreeView::AssertValid();
}

void CTreeList::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CHustBaseDoc* CTreeList::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHustBaseDoc)));
	return (CHustBaseDoc*)m_pDocument;
}
#endif //_DEBUG


void CTreeList::OnSize(UINT nType, int cx, int cy)
{
	this->ShowWindow(SW_MAXIMIZE);
	CWnd *pChild = this->GetWindow(GW_CHILD);
	if (pChild!=NULL)
	{
		CRect rect;
		this->GetWindowRect(&rect);
		pChild->ScreenToClient(&rect);
		pChild->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
			SWP_NOACTIVATE | SWP_NOZORDER);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CTreeList message handlers


void CTreeList::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	GetTreeCtrl().ModifyStyle(NULL, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT);
	// TODO: Add your specialized code here and/or call the base class
}

void CTreeList::InsertDBNodes()
{
//	_dic_type dicStruct; 

	HTREEITEM hItem, hsubItem, hCItem;
	hsubItem = NULL;
	char tabname[20][20];
	char colname[20][20][20];
	AttrType coltype[20][20];
	int tabnum,colnum[20];
	int collength[20][20];
	int coloffset[20][20];
	int iscolix[20][20];

//**************************小图标
	CImageList *m_pImageList = new CImageList();

	m_pImageList->Create(IDB_BITMAP1,15,3,RGB(0,0,0));
	bm.LoadBitmap(IDB_BITMAP2);			//表
	m_pImageList->Add(&bm, RGB(0, 0, 0));
	bm.DeleteObject();
	bm.LoadBitmap(IDB_BITMAP3);				//列
	m_pImageList->Add(&bm, RGB(0, 0, 0));
	bm.DeleteObject();
	bm.LoadBitmap(IDB_BITMAP4);				//文件
	m_pImageList->Add(&bm, RGB(0, 0, 0));
	bm.DeleteObject();
	GetTreeCtrl().SetImageList(m_pImageList,TVSIL_NORMAL);
//**************************小图标

	GetDocument()->m_pEditView->iReadDictstruct(tabname,&tabnum,colname,colnum,coltype,collength,coloffset,iscolix);
	for (int i=0; i<tabnum; i++)
	{
		hItem = GetTreeCtrl().InsertItem(tabname[i],1,1, TVI_ROOT);
		hCItem = GetTreeCtrl().InsertItem("列", 3, 3, hItem, hCItem);
		for (int j=0; j < colnum[i]; j++)
		{
			if(iscolix[i][j]==0)
				hsubItem = GetTreeCtrl().InsertItem(colname[i][j],2,2,hCItem,hsubItem);//无索引时的图标
			else
				hsubItem = GetTreeCtrl().InsertItem(colname[i][j],0,0,hCItem,hsubItem);//有索引时的图标
		}
		hsubItem = NULL;
	}
}

HTREEITEM CTreeList::InsertNode(const HTREEITEM ParentNode, const CString &str, const DWORD itemData)
{
	CTreeCtrl &treeCtrl = GetTreeCtrl();
	HTREEITEM node;

	node = treeCtrl.InsertItem(str, ParentNode, TVI_SORT);
	treeCtrl.SetItemData(node, itemData);

	return node;
}


void CTreeList::PopulateTree()
{	
	GetTreeCtrl().DeleteAllItems();
	GetDocument()->m_pListView->ClearList();	//等待修改
	openorNot=true;
	InsertDBNodes();
}


void CTreeList::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
// 	if (GetDocument()->isEdit)
// 	{
// 		InsertNode(TVI_ROOT, GetDocument()->get_text, 0);
// 	}
}

void CTreeList::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) //选中某个选项，显示其信息，Tree和List的一个桥梁函数
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	*pResult = 0;
	CString ItemText;
	CHustBaseDoc * pDoc = GetDocument();
	
	// Obtain a reference to the CTreeControl object that is
	// embedded in this view class.
	CTreeCtrl & ctlTree = GetTreeCtrl();
	
	// Determining which item the user has selected is a 2-step
	// process. You first obtain the handle to the selected item.
	HTREEITEM selectedNode = ctlTree.GetSelectedItem();
	
	// In the second step, you call a specific function
	// (GetItemText, GetItemData, GetItemState or GetItemImage)
	// to obtain a specific piece of information from the selected item,
	if(ctlTree.ItemHasChildren(selectedNode)==1)//点击的节点存在子项则做进一步处理，不存在子项即点击的项目为列名，不做处理
	{
		ItemText = ctlTree.GetItemText(selectedNode);
		
			// For root nodes, the list will iterate over the selected
			// item and display all the types it contains.
		if (openorNot)
		{
			pDoc->m_pListView->displayTabInfo(ItemText);
		}
	}

}
