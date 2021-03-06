// SkinListBox.cpp : 实现文件
//

#include "stdafx.h"
#include "Control.h"
#include "SkinListBox.h"

// CSkinListBox

IMPLEMENT_DYNAMIC(CSkinListBox, CListBox)

BEGIN_MESSAGE_MAP(CSkinListBox, CListBox)
	ON_WM_DESTROY()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CSkinListBox::CSkinListBox( void )
{
	m_pBackImgN = NULL;
	m_pSelectImg = NULL;
	m_pBackImgH = NULL;
	m_nHovenItem = 0;
	nItemHeight = 20;
	m_ItemStruct.clear();
}

CSkinListBox::~CSkinListBox( void )
{

}

BOOL CSkinListBox::SetBackNormalImage( LPCTSTR lpszFileName, CONST LPRECT lpNinePart /*= NULL*/ )
{
	UIRenderEngine->RemoveImage(m_pBackImgN);
	m_pBackImgN = UIRenderEngine->GetImage(lpszFileName);

	if (m_pBackImgN != NULL)
		m_pBackImgN->SetNinePart(lpNinePart);

	return (m_pBackImgN != NULL) ? TRUE : FALSE;
}

BOOL CSkinListBox::SetSelectImage( LPCTSTR lpszFileName, CONST LPRECT lpNinePart /*= NULL*/ )
{
	UIRenderEngine->RemoveImage(m_pSelectImg);
	m_pSelectImg = UIRenderEngine->GetImage(lpszFileName);

	if (m_pSelectImg != NULL)
		m_pSelectImg->SetNinePart(lpNinePart);

	return (m_pSelectImg != NULL) ? TRUE : FALSE;
}

BOOL CSkinListBox::SetHovenImage( LPCTSTR lpszFileName, CONST LPRECT lpNinePart /*= NULL*/ )
{
	UIRenderEngine->RemoveImage(m_pBackImgH);
	m_pBackImgH = UIRenderEngine->GetImage(lpszFileName);

	if (m_pBackImgH != NULL)
		m_pBackImgH->SetNinePart(lpNinePart);

	return (m_pBackImgH != NULL) ? TRUE : FALSE;
}

void CSkinListBox::OnDestroy()
{
	__super::OnDestroy();

	UIRenderEngine->RemoveImage(m_pBackImgN);
	UIRenderEngine->RemoveImage(m_pBackImgH);
	UIRenderEngine->RemoveImage(m_pSelectImg);

	RemoveScorll();
}

void CSkinListBox::OnNcPaint()
{
	//如果资源没有就是不想绘制边框了
	if ( m_pBackImgN == NULL ) 
	{
		__super::OnNcPaint();
		return;
	}

	CRect rcWindow;
	GetWindowRect(&rcWindow);

	CRect rcClient;
	GetClientRect(&rcClient);

	ClientToScreen(&rcClient);
	rcClient.OffsetRect(-rcWindow.left, -rcWindow.top);

	rcWindow.OffsetRect(-rcWindow.left, -rcWindow.top);

	CDC *pWindowDC = GetWindowDC();
	CMemoryDC MemDC(pWindowDC,rcWindow);

	DrawParentWndBg(GetSafeHwnd(),MemDC.GetSafeHdc());

	if (m_pBackImgN != NULL && !m_pBackImgN->IsNull())
		m_pBackImgN->Draw(&MemDC, rcWindow);
	
	pWindowDC->BitBlt(rcWindow.left,rcWindow.top,rcWindow.Width(),rcWindow.Height(),&MemDC,0,0,SRCCOPY);

	ReleaseDC(pWindowDC);
}

void CSkinListBox::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect rcItem(0,0,0,0);
	SetSel(-1,FALSE);

	for (int i =0; i<GetCount(); i++)
	{
		GetItemRect(i, &rcItem);

		if(rcItem.PtInRect(point))
		{
			if( IsSingleSel() )
				SetCurSel(i);
			else 
				SetSel(i);

			break;
		}
	}

	CWnd *pWnd = GetParent();
	if ( (pWnd == NULL) && (pWnd->GetSafeHwnd() != NULL) ) return;
	pWnd->PostMessage(WM_COMMAND,GetDlgCtrlID(),0);

	__super::OnRButtonUp(nFlags, point);
}

void CSkinListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rcItem(0,0,0,0);
	//保存旧状态，为了防止在同一个节点下因过多的刷新消耗cpu资源
	static int nOldIndex = -1;

	for (int i =0; i<GetCount(); i++)
	{
		GetItemRect(i, &rcItem);

		if(rcItem.PtInRect(point))
		{
			if( m_nHovenItem != i )  Invalidate(FALSE);

			m_nHovenItem = i;

			break;
		}
	}


	__super::OnMouseMove(nFlags, point);
}

void CSkinListBox::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	//没有节点就不用继续执行了
	if( GetCount()==0 ) return;

	//变量定义
	CRect rcItem=lpDrawItemStruct->rcItem;
	CDC * pDCControl=CDC::FromHandle(lpDrawItemStruct->hDC);

 	//创建缓冲
 	CDC BufferDC;
 	CBitmap ImageBuffer;
 	BufferDC.CreateCompatibleDC(pDCControl);
 	ImageBuffer.CreateCompatibleBitmap(pDCControl,rcItem.Width(),rcItem.Height());
 
 	//设置环境
 	BufferDC.SelectObject(&ImageBuffer);
	BufferDC.SelectObject(GetCtrlFont());

	//获取字符
	CString strString;
	GetText(lpDrawItemStruct->itemID,strString);

	//计算位置
	CRect rcString;
	rcString.SetRect(4,0,rcItem.Width()-8,rcItem.Height());

	//颜色定义
	COLORREF crTextColor=((lpDrawItemStruct->itemState&ODS_SELECTED)!=0)?m_colSelectText:m_colNormalText;

	//绘画背景
	BufferDC.FillSolidRect(0,0,rcItem.Width(),rcItem.Height(),m_colBack);

	//节点选中
	if ( (lpDrawItemStruct->itemState&ODS_SELECTED) != 0 )
	{
		if ( m_pSelectImg!= NULL && !m_pSelectImg->IsNull() )
		{
			m_pSelectImg->Draw(&BufferDC,CRect(0,0,rcItem.Width(),rcItem.Height()));
		}
	}

	//节点高亮
	else if ( m_nHovenItem == lpDrawItemStruct->itemID )
	{
		if ( m_pBackImgH!= NULL && !m_pBackImgH->IsNull() )
		{
			m_pBackImgH->Draw(&BufferDC,CRect(0,0,rcItem.Width(),rcItem.Height()));
		}
	}

	//绘画字符
	BufferDC.SetBkMode(TRANSPARENT);
	BufferDC.SetTextColor(crTextColor);
	BufferDC.DrawText(strString,&rcString,DT_VCENTER|DT_SINGLELINE);

	//绘画界面
	pDCControl->BitBlt(rcItem.left,rcItem.top,rcItem.Width(),rcItem.Height(),&BufferDC,0,0,SRCCOPY);

 	//清理资源
 	BufferDC.DeleteDC();
 	ImageBuffer.DeleteObject();
}

void CSkinListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	vector<tagItemStruct>::iterator iter = m_ItemStruct.begin();

	for (;iter != m_ItemStruct.end(); ++iter )
	{
		lpMeasureItemStruct->itemHeight = iter->itemHeight;
		lpMeasureItemStruct->itemWidth = iter->itemWidth;
	}
}

bool CSkinListBox::IsSingleSel()
{
	DWORD dwStyle = GetStyle();

	if ( ((dwStyle&LBS_EXTENDEDSEL)!=0) || ((dwStyle&LBS_MULTIPLESEL)!=0) )
	{
		return false;
	}

	return true;
}

void CSkinListBox::AddString(LPCTSTR lpszItem)
{
	InsertItem();

	__super::AddString(lpszItem);
}

int CSkinListBox::SetItemHeight( int nIndex, UINT cyItemHeight )
{
	if ( nIndex == -1 ) nItemHeight = cyItemHeight;
	
	vector<tagItemStruct>::iterator iter = m_ItemStruct.begin();

	for (;iter != m_ItemStruct.end(); ++iter )
	{
		if ( iter->itemID == nIndex )
		{
			iter->itemHeight = cyItemHeight;

			break;
		}
	}

	return __super::SetItemHeight(nIndex,cyItemHeight);
}

int CSkinListBox::DeleteString( UINT nIndex )
{
 	vector<tagItemStruct>::iterator iter = m_ItemStruct.begin();
 
 	for (;iter != m_ItemStruct.end(); ++iter )
 	{
 		if ( iter->itemID == nIndex )
 		{
 			m_ItemStruct.erase(iter);
			break;
 		}
 	}

	return __super::DeleteString(nIndex);
}

void CSkinListBox::InsertItem()
{
	tagItemStruct ItemStruct;
	ZeroMemory(&ItemStruct,sizeof ItemStruct);

	CRect rcClient;
	GetClientRect(&rcClient);

	ItemStruct.itemID = m_ItemStruct.size();
	ItemStruct.itemHeight = nItemHeight;
	ItemStruct.itemWidth = rcClient.Width();

	m_ItemStruct.push_back(ItemStruct);
}

void CSkinListBox::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
{
	ISkinControl::SetAttribute(pstrName,pstrValue);
	
// 	if( _tcscmp(pstrName, _T("itemheight")) == 0 )  
// 	{
// 		LPTSTR pstr = NULL;
// 		SetItemHeight(-1,_tcstol(pstrValue, &pstr, 10));
// 	}
//	else 
	if( _tcscmp(pstrName, _T("scrollimage")) == 0 )  
	{
		SetScrollImage(this,pstrValue);
	}
}

void CSkinListBox::ParseItem( CXmlNode *root )
{
	if( root == NULL ) return;

	int nAttributes = root->GetAttributeCount();

	LPCTSTR pstrClass = NULL;
	LPCTSTR pstrName = NULL;
	LPCTSTR pstrValue = NULL;

	for( int i = 0; i < nAttributes; i++ ) 
	{
		pstrClass = root->GetName();
		pstrName = root->GetAttributeName(i);
		pstrValue = root->GetAttributeValue(i);

		if( _tcscmp(pstrClass, _T("bknormalimage")) == 0 ) 	
		{
			if( _tcscmp(pstrName, _T("value")) == 0 ) SetBackNormalImage(pstrValue);
			else if( _tcscmp(pstrName, _T("ninepart")) == 0 )
			{
				LPTSTR pstr = NULL;
				CRect rc;
				rc.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
				rc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
				rc.right = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
				rc.bottom = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   

				if (m_pBackImgN != NULL)
					m_pBackImgN->SetNinePart(&rc);
			}
		}
		else if( _tcscmp(pstrClass, _T("selectimage")) == 0 ) 	
		{
			if( _tcscmp(pstrName, _T("value")) == 0 ) SetSelectImage(pstrValue);
			else if( _tcscmp(pstrName, _T("ninepart")) == 0 )
			{
				LPTSTR pstr = NULL;
				CRect rc;
				rc.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
				rc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
				rc.right = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
				rc.bottom = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   

				if (m_pSelectImg != NULL)
					m_pSelectImg->SetNinePart(&rc);
			}
		}
		else if( _tcscmp(pstrClass, _T("hovenimage")) == 0 ) 	
		{
			if( _tcscmp(pstrName, _T("value")) == 0 ) SetHovenImage(pstrValue);
			else if( _tcscmp(pstrName, _T("ninepart")) == 0 )
			{
				LPTSTR pstr = NULL;
				CRect rc;
				rc.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
				rc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
				rc.right = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
				rc.bottom = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   

				if (m_pBackImgH != NULL)
					m_pBackImgH->SetNinePart(&rc);
			}
		}
		else if( _tcscmp(pstrClass, _T("item")) == 0 ) 	
		{
			if( _tcscmp(pstrName, _T("text")) == 0 ) AddString(pstrValue);
		}
	}
}

BOOL CSkinListBox::CreateControl( CWnd* pParentWnd )
{
	if( !Create(WS_CHILD|WS_VISIBLE|WS_TABSTOP|LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS |WS_VSCROLL ,CRect(0,0,0,0),pParentWnd,0) )
		return FALSE;

	SetFont(CFont::FromHandle(UIRenderEngine->GetDeaultFont()));

	m_pOwnWnd = this;

	return TRUE;
}

BOOL CSkinListBox::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

	return __super::OnEraseBkgnd(pDC);
}
