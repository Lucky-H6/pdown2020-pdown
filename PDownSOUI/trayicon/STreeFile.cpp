//////////////////////////////////////////////////////////////////////////
//  Class Name: STreeFile
//     Creator: JinHui
//     Version: 2012.12.16 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "souistd.h"
#include "STreeFile.h"
#include <algorithm>

namespace SOUI {

	STreeFile::STreeFile()
		: m_nItemHei(20)
		, m_StateText(NULL)
		, m_nIndent(18)
		, m_nItemMargin(4)
		, m_hHoverItem(NULL)
		, m_hCaptureItem(NULL)
		, m_pItemBgSkin(NULL)
		, m_pItemSelSkin(NULL)
		, m_pIconSkin(NULL)
		, m_pLineSkin(GETBUILTINSKIN(SKIN_SYS_TREE_LINES))
		, m_pToggleSkin(GETBUILTINSKIN(SKIN_SYS_TREE_TOGGLE))
		, m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_TREE_CHECKBOX))
		, m_crItemBg(RGBA(255, 255, 255, 255))
		, m_crItemSelBg(RGBA(0, 0, 136, 255))
		, m_crItemText(RGBA(0, 0, 0, 255))
		, m_crItemSelText(RGBA(255, 255, 255, 255))
		, m_nVisibleItems(0)
		, m_nMaxItemWidth(0)
		, m_nRootWidth(0)
		, m_bCheckBox(FALSE)
		, m_bRightClickSel(FALSE)
		, m_uItemMask(0)
		, m_nItemOffset(0)
		, m_nItemHoverBtn(STVIBtn_None)
		, m_nItemPushDownBtn(STVIBtn_None)
		, m_bHasLines(FALSE)
	{
		m_bClipClient = TRUE;
		m_bFocusable = TRUE;
		m_evtSet.addEvent(EVENTID(EventTCSelChanging));
		m_evtSet.addEvent(EVENTID(EventTCSelChanged));
		m_evtSet.addEvent(EVENTID(EventTCCheckState));
		m_evtSet.addEvent(EVENTID(EventTCExpand));
		m_evtSet.addEvent(EVENTID(EventTCDbClick));
	}

	STreeFile::~STreeFile()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////////

	HSTREEITEM STreeFile::InsertItem(DowningItem& FileInfo, bool bIsDir, HSTREEITEM hParent, HSTREEITEM hInsertAfter, BOOL bEnsureVisible)
	{
		return InsertItem(FileInfo, bIsDir, (bIsDir ? 0 : 2), (bIsDir ? 1 : 2), hParent, hInsertAfter, bEnsureVisible);//修正
	}

	HSTREEITEM STreeFile::InsertItem(DowningItem& FileInfo, bool bIsDir, int nImage,
		int nSelectedImage,
		HSTREEITEM hParent, HSTREEITEM hInsertAfter, BOOL bEnsureVisible)
	{
		LPTVITEMFILE pItemObj = new TVITEMFILE();

		pItemObj->FileInfo = FileInfo;//修正
		pItemObj->nImage = nImage;
		pItemObj->nSelectedImage = nSelectedImage;
		pItemObj->bIsDir = bIsDir;
		pItemObj->bCollapsed = false;
		pItemObj->nCheckBoxValue = STVICheckBox_Checked;


		return InsertItem(pItemObj, hParent, hInsertAfter, bEnsureVisible);
	}

	BOOL STreeFile::RemoveItem(HSTREEITEM hItem)
	{
		if (!hItem) return FALSE;
		HSTREEITEM hParent = GetParentItem(hItem);

		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

		BOOL bVisible = pItem->bVisible;
		int nItemWidth = GetMaxItemWidth(hItem);
		int nCheckBoxValue = pItem->nCheckBoxValue;
		if (bVisible)
		{
			if (GetChildItem(hItem) && pItem->bCollapsed == FALSE)
			{
				SetChildrenVisible(hItem, FALSE);
			}
		}

		if (IsAncestor(hItem, m_hHoverItem)) m_hHoverItem = NULL;
		if (IsAncestor(hItem, m_hCaptureItem)) m_hCaptureItem = NULL;

		DeleteItem(hItem);

		//去掉父节点的展开标志
		if (hParent && !GetChildItem(hParent))
		{
			LPTVITEMFILE pParent = GetItem(hParent);
			pParent->bHasChildren = FALSE;
			pParent->bCollapsed = FALSE;
			CalcItemContentWidth(pParent);
		}

		if (m_bCheckBox && hParent)
		{
			//如果父节点同为不选或全选，则不用改变状态，其他情况都需要重新判断
			if (nCheckBoxValue != GetItem(hParent)->nCheckBoxValue || nCheckBoxValue == STVICheckBox_PartChecked)
				CheckState(hParent);
		}

		if (bVisible)
		{
			m_nVisibleItems--;

			//重新计算x最大尺寸
			if (nItemWidth == m_nMaxItemWidth)
				GetMaxItemWidth();

			CSize szView(m_nMaxItemWidth, m_nVisibleItems * m_nItemHei);
			SetViewSize(szView);
			Invalidate();
		}
		return TRUE;
	}

	void STreeFile::RemoveAllItems()
	{
		DeleteAllItems();
		m_nVisibleItems = 0;
		m_hHoverItem = NULL;
		m_hCaptureItem = NULL;
		m_nMaxItemWidth = 0;
		SetViewSize(CSize(0, 0));
		SetViewOrigin(CPoint(0, 0));
	}

	HSTREEITEM STreeFile::GetRootItem() const
	{
		return GetChildItem(STVI_ROOT);
	}

	HSTREEITEM STreeFile::GetNextSiblingItem(HSTREEITEM hItem) const
	{
		return CSTree<LPTVITEMFILE>::GetNextSiblingItem(hItem);
	}

	HSTREEITEM STreeFile::GetPrevSiblingItem(HSTREEITEM hItem) const
	{
		return CSTree<LPTVITEMFILE>::GetPrevSiblingItem(hItem);
	}

	HSTREEITEM STreeFile::GetChildItem(HSTREEITEM hItem, BOOL bFirst/* =TRUE*/) const
	{
		return CSTree<LPTVITEMFILE>::GetChildItem(hItem, bFirst);
	}

	HSTREEITEM STreeFile::GetParentItem(HSTREEITEM hItem) const
	{
		return CSTree<LPTVITEMFILE>::GetParentItem(hItem);
	}





	BOOL STreeFile::GetItemImage(HSTREEITEM hItem, int& nImage, int& nSelectedImage) const
	{
		if (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (pItem)
			{
				nImage = pItem->nImage;
				nSelectedImage = pItem->nSelectedImage;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL STreeFile::SetItemImage(HSTREEITEM hItem, int nImage, int nSelectedImage)
	{
		if (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (pItem)
			{
				pItem->nImage = nImage;
				pItem->nSelectedImage = nSelectedImage;
				return TRUE;
			}
		}
		return FALSE;
	}


	BOOL STreeFile::ItemHasChildren(HSTREEITEM hItem)
	{
		if (!hItem) return FALSE;

		return  GetChildItem(hItem) != NULL;
	}

	BOOL STreeFile::GetCheckState(HSTREEITEM hItem) const
	{
		if (!m_bCheckBox) return FALSE;

		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
		if (pItem->nCheckBoxValue == STVICheckBox_UnChecked)
			return FALSE;

		return TRUE;
	}

	BOOL STreeFile::SetCheckState(HSTREEITEM hItem, BOOL bCheck)
	{
		if (!m_bCheckBox) return FALSE;

		int nCheck = bCheck ? STVICheckBox_Checked : STVICheckBox_UnChecked;

		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
		if (pItem->nCheckBoxValue != nCheck)
		{
			pItem->nCheckBoxValue = nCheck;

			//置子孙结点
			if (CSTree<LPTVITEMFILE>::GetChildItem(hItem))
				SetChildrenState(hItem, nCheck);

			//检查父结点状态
			CheckState(GetParentItem(hItem));

			Invalidate();
		}
		RefreshStateText();
		return TRUE;
	}

	BOOL STreeFile::Expand(HSTREEITEM hItem, UINT nCode)
	{
		BOOL bRet = FALSE;
		if (CSTree<LPTVITEMFILE>::GetChildItem(hItem))
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (nCode == TVE_COLLAPSE && !pItem->bCollapsed)
			{
				pItem->bCollapsed = TRUE;
				SetChildrenVisible(hItem, FALSE);
				bRet = TRUE;
			}
			if (nCode == TVE_EXPAND && pItem->bCollapsed)
			{
				pItem->bCollapsed = FALSE;
				SetChildrenVisible(hItem, TRUE);
				bRet = TRUE;
			}
			if (nCode == TVE_TOGGLE)
			{
				pItem->bCollapsed = !pItem->bCollapsed;
				SetChildrenVisible(hItem, !pItem->bCollapsed);
				bRet = TRUE;
			}
			if (bRet)
			{
				EventTCExpand evt(this);
				evt.hItem = hItem;
				evt.bCollapsed = pItem->bCollapsed;
				FireEvent(evt);

				GetMaxItemWidth();
				CSize szView(m_nMaxItemWidth, m_nVisibleItems * m_nItemHei);
				SetViewSize(szView);
				Invalidate();
			}
		}
		return bRet;
	}

	BOOL STreeFile::EnsureVisible(HSTREEITEM hItem)
	{
		if (!VerifyItem(hItem)) return FALSE;

		LPTVITEMFILE pItem = GetItem(hItem);
		if (!pItem->bVisible)
		{
			HSTREEITEM hParent = GetParentItem(hItem);
			while (hParent)
			{
				LPTVITEMFILE pParent = GetItem(hParent);
				if (pParent->bCollapsed) Expand(hParent, TVE_EXPAND);
				hParent = GetParentItem(hParent);
			}
		}
		int iVisible = GetItemShowIndex(hItem);
		int yOffset = iVisible * m_nItemHei;
		if (yOffset + m_nItemHei > m_ptOrigin.y + m_rcClient.Height())
		{
			SetScrollPos(TRUE, yOffset + m_nItemHei - m_rcClient.Height(), TRUE);
		}
		else if (yOffset < m_ptOrigin.y)
		{
			SetScrollPos(TRUE, yOffset, TRUE);
		}
		return TRUE;
	}

	void STreeFile::PageUp()
	{
		OnScroll(TRUE, SB_PAGEUP, 0);
	}

	void STreeFile::PageDown()
	{
		OnScroll(TRUE, SB_PAGEDOWN, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////

	BOOL STreeFile::CreateChildren(pugi::xml_node xmlNode)
	{
		if (!xmlNode) return FALSE;

		RemoveAllItems();
		ItemLayout();

		pugi::xml_node xmlItem = xmlNode.child(L"item");

		if (xmlItem) LoadBranch(STVI_ROOT, xmlItem);

		return TRUE;
	}

	void STreeFile::LoadBranch(HSTREEITEM hParent, pugi::xml_node xmlItem)
	{
		while (xmlItem)
		{
			HSTREEITEM hItem = InsertItem(xmlItem, hParent);

			pugi::xml_node xmlChild = xmlItem.child(L"item");
			if (xmlChild)
			{
				LoadBranch(hItem, xmlChild);
				Expand(hItem, xmlItem.attribute(L"expand").as_bool(true) ? TVE_EXPAND : TVE_COLLAPSE);
			}
			xmlItem = xmlItem.next_sibling(L"item");
		}
	}

	void STreeFile::LoadItemAttribute(pugi::xml_node xmlItem, LPTVITEMFILE pItem)
	{
		for (pugi::xml_attribute attr = xmlItem.first_attribute(); attr; attr = attr.next_attribute())
		{
			if (!_wcsicmp(attr.name(), L"img"))
				pItem->nImage = attr.as_int(0);
			else if (!_wcsicmp(attr.name(), L"selImg"))
				pItem->nSelectedImage = attr.as_int(0);

		}
	}

	HSTREEITEM STreeFile::InsertItem(LPTVITEMFILE pItemObj, HSTREEITEM hParent, HSTREEITEM hInsertAfter, BOOL bEnsureVisible)
	{
		SASSERT(pItemObj);

		CRect rcClient;
		GetClientRect(rcClient);
		m_nRootWidth = rcClient.Width();//修正横向滚动条
		pItemObj->nLevel = GetItemLevel(hParent) + 1;

		BOOL bCheckState = FALSE;

		if (hParent != STVI_ROOT)
		{
			LPTVITEMFILE pParentItem = GetItem(hParent);
			if (pParentItem->bCollapsed || !pParentItem->bVisible)
				pItemObj->bVisible = FALSE;

			if (pParentItem->nCheckBoxValue != pItemObj->nCheckBoxValue)
				bCheckState = TRUE;

			if (!GetChildItem(hParent) && !pParentItem->bHasChildren)
			{
				pParentItem->bHasChildren = TRUE;
				CalcItemContentWidth(pParentItem);
			}
		}

		CalcItemContentWidth(pItemObj);

		HSTREEITEM hRet = CSTree<LPTVITEMFILE>::InsertItem(pItemObj, hParent, hInsertAfter);
		pItemObj->hItem = hRet;
		OnInsertItem(pItemObj);

		if (pItemObj->bVisible)
		{
			m_nVisibleItems++;

			int nViewWidth = CalcItemWidth(pItemObj);
			m_nMaxItemWidth = (std::max)(nViewWidth, m_nMaxItemWidth);

			CSize szView(m_nMaxItemWidth, m_nVisibleItems * m_nItemHei);
			SetViewSize(szView);
			Invalidate();
		}

		if (bCheckState) CheckState(hParent);
		if (bEnsureVisible) EnsureVisible(hRet);
		return hRet;
	}

	HSTREEITEM STreeFile::InsertItem(pugi::xml_node xmlItem, HSTREEITEM hParent/*=STVI_ROOT*/, HSTREEITEM hInsertAfter/*=STVI_LAST*/, BOOL bEnsureVisible/*=FALSE*/)
	{
		LPTVITEMFILE pItemObj = new TVITEMFILE();

		LoadItemAttribute(xmlItem, pItemObj);
		return InsertItem(pItemObj, hParent, hInsertAfter, bEnsureVisible);
	}


	BOOL STreeFile::IsAncestor(HSTREEITEM hItem1, HSTREEITEM hItem2)
	{
		while (hItem2)
		{
			if (hItem2 == hItem1) return TRUE;
			hItem2 = GetParentItem(hItem2);
		}
		return FALSE;
	}

	void STreeFile::SetChildrenVisible(HSTREEITEM hItem, BOOL bVisible)
	{
		HSTREEITEM hChild = GetChildItem(hItem);
		while (hChild)
		{
			LPTVITEMFILE pItem = GetItem(hChild);
			pItem->bVisible = bVisible;
			m_nVisibleItems += bVisible ? 1 : -1;
			if (!pItem->bCollapsed) SetChildrenVisible(hChild, bVisible);
			hChild = GetNextSiblingItem(hChild);
		}
	}

	void STreeFile::SetChildrenState(HSTREEITEM hItem, int nCheckValue)
	{
		HSTREEITEM hChildItem = CSTree<LPTVITEMFILE>::GetChildItem(hItem);
		while (hChildItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hChildItem);
			pItem->nCheckBoxValue = nCheckValue;
			SetChildrenState(hChildItem, nCheckValue);
			hChildItem = CSTree<LPTVITEMFILE>::GetNextSiblingItem(hChildItem);
		}
	}

	//子孙结点状态一致返回TRUE, 否则返回FALSE
	BOOL STreeFile::CheckChildrenState(HSTREEITEM hItem, BOOL bCheck)
	{
		HSTREEITEM hChildItem = CSTree<LPTVITEMFILE>::GetChildItem(hItem);
		while (hChildItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hChildItem);

			int nCheckValue = bCheck ? STVICheckBox_Checked : STVICheckBox_UnChecked;
			//当前结点不一致立即返回
			if (pItem->nCheckBoxValue != nCheckValue)
				return FALSE;
			//检查子结点不一致立即返回
			else if (CheckChildrenState(hChildItem, bCheck) == FALSE)
				return FALSE;

			//检查子结点兄弟结点
			hChildItem = CSTree<LPTVITEMFILE>::GetNextSiblingItem(hChildItem);
		}
		return TRUE;
	}

	void STreeFile::CheckState(HSTREEITEM hItem)
	{
		if (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			int nOldState = pItem->nCheckBoxValue;
			pItem->nCheckBoxValue = STVICheckBox_UnChecked;
			bool bHasUnChecked = false;
			bool bHasChecked = false;
			bool bHasPartChecked = false;

			HSTREEITEM hChild = GetChildItem(hItem);
			while (hChild)
			{
				LPTVITEMFILE pChild = CSTree<LPTVITEMFILE>::GetItem(hChild);
				if (pChild->nCheckBoxValue == STVICheckBox_UnChecked)
					bHasUnChecked = true;
				else if (pChild->nCheckBoxValue == STVICheckBox_Checked)
					bHasChecked = true;
				else if (pChild->nCheckBoxValue == STVICheckBox_PartChecked)
					bHasPartChecked = true;

				if (bHasPartChecked || (bHasUnChecked && bHasChecked))
					break; //已确定半选，提前结束循环
				hChild = GetNextSiblingItem(hChild);
			}

			if (bHasPartChecked || (bHasUnChecked && bHasChecked)) //子节点有半选，则父节点也要半选
				pItem->nCheckBoxValue = STVICheckBox_PartChecked;
			else if (bHasChecked && !bHasUnChecked) //子节点都是全选，则父节点也是全选
				pItem->nCheckBoxValue = STVICheckBox_Checked;

			if (pItem->nCheckBoxValue != nOldState)
				CheckState(GetParentItem(hItem));
		}
	}

	void STreeFile::ItemLayout()
	{
		int nOffset = 0;
		CSize sizeSkin;

		m_uItemMask = 0;
		m_rcToggle.SetRect(0, 0, 0, 0);
		m_rcCheckBox.SetRect(0, 0, 0, 0);
		m_rcIcon.SetRect(0, 0, 0, 0);

		//计算位置    
		if (m_pToggleSkin || m_bHasLines)
		{
			m_uItemMask |= STVIMask_Toggle;
			CSize szToggle;
			if (m_bHasLines)
			{
				szToggle = CSize(m_nIndent, m_nIndent);
			}
			else
			{
				szToggle = m_pToggleSkin->GetSkinSize();
			}
			m_rcToggle.SetRect(
				nOffset,
				(m_nItemHei - szToggle.cy) / 2,
				nOffset + szToggle.cx,
				m_nItemHei - (m_nItemHei - szToggle.cy) / 2);
			nOffset += szToggle.cx;
		}

		if (m_bCheckBox && m_pCheckSkin)
		{
			m_uItemMask |= STVIMask_CheckBox;
			sizeSkin = m_pCheckSkin->GetSkinSize();
			m_rcCheckBox.SetRect(
				nOffset,
				(m_nItemHei - sizeSkin.cy) / 2,
				nOffset + sizeSkin.cx,
				m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
			nOffset += sizeSkin.cx;
		}

		if (m_pIconSkin)
		{
			m_uItemMask |= STVIMask_Icon;
			sizeSkin = m_pIconSkin->GetSkinSize();
			m_rcIcon.SetRect(
				nOffset,
				(m_nItemHei - sizeSkin.cy) / 2,
				nOffset + sizeSkin.cx,
				m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
			nOffset += sizeSkin.cx;
		}

		m_nItemOffset = nOffset;
	}

	void STreeFile::CalcItemContentWidth(LPTVITEMFILE pItem)
	{
		if (m_nRootWidth > 0) {//修正
			int left = m_nIndent * pItem->nLevel;
			pItem->nContentWidth = m_nRootWidth - left;
			return;
		}
		SAutoRefPtr<IRenderTarget> pRT;
		GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
		BeforePaintEx(pRT);

		int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);
		CRect rcTest;
		DrawText(pRT, pItem->FileInfo.server_filename.c_str(), -1, rcTest, nTestDrawMode | DT_CALCRECT);

		pItem->nContentWidth = rcTest.Width() + m_nItemOffset + 2 * m_nItemMargin;
	}

	int STreeFile::GetMaxItemWidth(HSTREEITEM hItem)
	{
		int nItemWidth = 0, nChildrenWidth = 0;

		LPTVITEMFILE pItem = GetItem(hItem);
		if (pItem->bVisible)
			nItemWidth = CalcItemWidth(pItem);
		else
			return 0;

		HSTREEITEM hChild = GetChildItem(hItem);
		while (hChild)
		{
			nChildrenWidth = GetMaxItemWidth(hChild);
			if (nChildrenWidth > nItemWidth)
				nItemWidth = nChildrenWidth;

			hChild = GetNextSiblingItem(hChild);
		}

		return nItemWidth;
	}

	int  STreeFile::GetMaxItemWidth()
	{
		int nItemWidth = 0;
		m_nMaxItemWidth = 0;
		HSTREEITEM hItem = CSTree<LPTVITEMFILE>::GetNextItem(STVI_ROOT);

		while (hItem)
		{
			nItemWidth = GetMaxItemWidth(hItem);
			if (nItemWidth > m_nMaxItemWidth)
				m_nMaxItemWidth = nItemWidth;
			hItem = GetNextSiblingItem(hItem);
		}

		return m_nMaxItemWidth;
	}

	int STreeFile::GetItemShowIndex(HSTREEITEM hItemObj)
	{
		int iVisible = -1;
		HSTREEITEM hItem = GetNextItem(STVI_ROOT);
		while (hItem)
		{
			LPTVITEMFILE pItem = GetItem(hItem);
			if (pItem->bVisible) iVisible++;
			if (hItem == hItemObj)
			{
				return iVisible;
			}
			if (pItem->bCollapsed)
			{//跳过被折叠的项
				HSTREEITEM hChild = GetChildItem(hItem, FALSE);
				while (hChild)
				{
					hItem = hChild;
					hChild = GetChildItem(hItem, FALSE);
				}
			}
			hItem = GetNextItem(hItem);
		}
		return -1;
	}

	BOOL STreeFile::GetItemRect(LPTVITEMFILE pItemObj, CRect& rcItem)
	{
		if (pItemObj->bVisible == FALSE) return FALSE;

		CRect rcClient;
		GetClientRect(rcClient);
		int iFirstVisible = m_ptOrigin.y / m_nItemHei;
		int nPageItems = (rcClient.Height() + m_nItemHei - 1) / m_nItemHei + 1;

		int iVisible = -1;
		HSTREEITEM hItem = CSTree<LPTVITEMFILE>::GetNextItem(STVI_ROOT);
		while (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (pItem->bVisible) iVisible++;
			if (iVisible > iFirstVisible + nPageItems) break;
			if (iVisible >= iFirstVisible && pItem == pItemObj)
			{
				CRect rcRet(m_nIndent * pItemObj->nLevel, 0, rcClient.Width(), m_nItemHei);
				rcRet.OffsetRect(rcClient.left - m_ptOrigin.x, rcClient.top - m_ptOrigin.y + iVisible * m_nItemHei);
				rcItem = rcRet;
				return TRUE;
			}
			if (pItem->bCollapsed)
			{//跳过被折叠的项
				HSTREEITEM hChild = GetChildItem(hItem, FALSE);
				while (hChild)
				{
					hItem = hChild;
					hChild = GetChildItem(hItem, FALSE);
				}
			}
			hItem = CSTree<LPTVITEMFILE>::GetNextItem(hItem);
		}
		return FALSE;
	}

	//自动修改pt的位置为相对当前项的偏移量
	HSTREEITEM STreeFile::HitTest(CPoint& pt)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		CPoint pt2 = pt;
		pt2.y -= rcClient.top - m_ptOrigin.y;
		int iItem = pt2.y / m_nItemHei;
		if (iItem >= m_nVisibleItems) return NULL;

		HSTREEITEM hRet = NULL;

		int iVisible = -1;
		HSTREEITEM hItem = CSTree<LPTVITEMFILE>::GetNextItem(STVI_ROOT);
		while (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (pItem->bVisible) iVisible++;
			if (iVisible == iItem)
			{
				CRect rcItem(m_nIndent * pItem->nLevel, 0, rcClient.Width(), m_nItemHei);
				rcItem.OffsetRect(rcClient.left - m_ptOrigin.x, rcClient.top - m_ptOrigin.y + iVisible * m_nItemHei);
				pt -= rcItem.TopLeft();
				hRet = hItem;
				break;
			}
			if (pItem->bCollapsed)
			{//跳过被折叠的项
				HSTREEITEM hChild = GetChildItem(hItem, FALSE);
				while (hChild)
				{
					hItem = hChild;
					hChild = GetChildItem(hItem, FALSE);
				}
			}
			hItem = CSTree<LPTVITEMFILE>::GetNextItem(hItem);
		}
		return hRet;
	}



	void STreeFile::BindStateText(SWindow* statetext)
	{
		m_StateText = statetext;
		RefreshStateText();
	}
	std::wstring STreeFile::FormatFileSize(int64_t filesize) {

		if (filesize < 0) return L"0B";
		wchar_t szText[64];
		double bytes = (double)filesize;
		DWORD cIter = 0;
		std::wstring pszUnits[] = { (L"B"), (L"KB"), (L"MB"), (L"GB"), (L"TB") };
		DWORD cUnits = 5;
		// move from bytes to KB, to MB, to GB and so on diving by 1024
		while (bytes >= 1000 && cIter < (cUnits - 1))
		{
			bytes /= 1024;
			cIter++;
		}
		int slen = swprintf_s(szText, 64, (L"%.2f"), bytes);
		/*
		0.00
		10.00
		100.00
		*/
		if (slen > 5) slen = 5;
		std::wstring ret = std::wstring(szText, slen) + pszUnits[cIter];
		return ret;
	}
	void STreeFile::RefreshStateText()
	{
		if (!m_StateText) return;
		size_t count_all = 0, count_checked = 0;
		int64_t totalsize = 0;
		HSTREEITEM hItem = CSTree<LPTVITEMFILE>::GetNextItem(STVI_ROOT);
		while (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (!pItem->bIsDir) {//文件图标
				count_all++;
				if (pItem->nCheckBoxValue == STVICheckBox_Checked) {
					count_checked++;
					totalsize += pItem->FileInfo.size;
				}
			}

			hItem = CSTree<LPTVITEMFILE>::GetNextItem(hItem);
		}
		std::wstring state = L"已选中 " + std::to_wstring(count_checked) + L" / " + std::to_wstring(count_all) + L" 个文件  (";
		state += FormatFileSize(totalsize) + L")";
		m_StateText->SetWindowTextW(state.c_str());
	}

	void STreeFile::RedrawItem(HSTREEITEM hItem)
	{
		if (!IsVisible(TRUE)) return;
		CRect rcClient;
		GetClientRect(rcClient);

		int iFirstVisible = m_ptOrigin.y / m_nItemHei;
		int nPageItems = (rcClient.Height() + m_nItemHei - 1) / m_nItemHei + 1;
		int iItem = GetItemShowIndex(hItem);
		if (iItem != -1 && iItem >= iFirstVisible && iItem < iFirstVisible + nPageItems)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

			CRect rcItem(0, 0, CalcItemWidth(pItem), m_nItemHei);
			rcItem.OffsetRect(rcClient.left - m_ptOrigin.x,
				rcClient.top + m_nItemHei * iItem - m_ptOrigin.y);

			SAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(&rcItem, GRT_PAINTBKGND);

			SSendMessage(WM_ERASEBKGND, (WPARAM)(void*)pRT);

			DrawLines(pRT, rcItem, hItem);
			DrawItem(pRT, rcItem, hItem);

			ReleaseRenderTarget(pRT);
		}
	}

	void STreeFile::DrawItem(IRenderTarget* pRT, const CRect& rc, HSTREEITEM hItem)
	{
		CRect rcItemBg;
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

		pRT->OffsetViewportOrg(rc.left + pItem->nLevel * m_nIndent, rc.top);

		rcItemBg.SetRect(m_nItemOffset + m_nItemMargin, 0, rc.Width(), m_nItemHei);
		rcItemBg.right = rc.Width() - pItem->nLevel * m_nIndent;
		//绘制背景
		if (hItem == m_hHoverItem) {//修正
			CRect rcHoverBg; //修正
			rcHoverBg.SetRect(rcItemBg.left - 20, rcItemBg.top, rcItemBg.right, rcItemBg.bottom); //修正
			pRT->FillSolidRect(rcHoverBg, m_crItemSelBg);//修正
			pRT->SetTextColor(m_crItemSelText);//修正
		}
		else {
			pRT->SetTextColor(m_crItemText);//修正
		}

		if (STVIMask_CheckBox == (m_uItemMask & STVIMask_CheckBox))
		{
			int nImage = SState2Index::GetDefIndex(pItem->dwCheckBoxState, false);
			if (pItem->nCheckBoxValue == STVICheckBox_Checked)
				nImage += 3;
			else if (pItem->nCheckBoxValue == STVICheckBox_PartChecked)
				nImage += 6;
			m_pCheckSkin->DrawByIndex(pRT, m_rcCheckBox, nImage);
		}


		if (pItem->bCollapsed) {//收起状态 //修正
			if (pItem->nImage != -1) m_pIconSkin->DrawByIndex(pRT, m_rcIcon, pItem->nImage);//修正
		}
		else {//展开状态
			if (pItem->nSelectedImage != -1) m_pIconSkin->DrawByIndex(pRT, m_rcIcon, pItem->nSelectedImage);//修正
			else if (pItem->nImage != -1) m_pIconSkin->DrawByIndex(pRT, m_rcIcon, pItem->nImage);//修正
		}

		int left = m_nIndent * pItem->nLevel; //修正
		UINT align = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
		rcItemBg.OffsetRect(m_nItemMargin, 0);

		DowningItem& file = pItem->FileInfo;

		CRect rcFileSize;
		rcFileSize.SetRect(0, rcItemBg.top, 100, rcItemBg.bottom); //修正
		if (file.sizestr != L"") pRT->DrawText(file.sizestr.c_str(), -1, rcFileSize, DT_CALCRECT | DT_SINGLELINE);
		CRect rcFileTime;
		rcFileTime.SetRect(0, rcItemBg.top, 100, rcItemBg.bottom); //修正
		if (file.down_timestr != L"") pRT->DrawText(file.down_timestr.c_str(), -1, rcFileTime, DT_CALCRECT | DT_SINGLELINE);
		//pRT->DT_CALCRECT | DT_SINGLELINE
		int rcWidthTotal = rcFileSize.Width() + 8 + rcFileTime.Width() + 8;

		CRect rcFileName; //修正
		rcFileName.SetRect(rcItemBg.left, rcItemBg.top, rcItemBg.right - rcWidthTotal-20, rcItemBg.bottom); //修正 236
		pRT->DrawText(file.server_filename.c_str(), -1, rcFileName, align);    //修正

		COLORREF crSizeText = RGBA(134, 142, 150, 150);
		pRT->SetTextColor(crSizeText);//文件大小的文字颜色//修正

		if (file.sizestr != L"") {
			int rcFileSizeLeft = rcItemBg.right - rcWidthTotal - 8;
			rcFileSize.SetRect(rcFileSizeLeft, rcItemBg.top, rcFileSizeLeft + rcFileSize.Width(), rcItemBg.bottom); //修正
			pRT->DrawText(file.sizestr.c_str(), -1, rcFileSize, align | DT_RIGHT);    //修正
		}
		if (file.down_timestr != L"") {
			int rcFileTimeLeft = rcItemBg.right - rcFileTime.Width() - 8;
			rcFileTime.SetRect(rcFileTimeLeft, rcItemBg.top, rcFileTimeLeft + rcFileTime.Width(), rcItemBg.bottom); //修正 -120
			pRT->DrawText(file.down_timestr.c_str(), -1, rcFileTime, DT_VCENTER | DT_SINGLELINE);    //修正
		}
		pRT->SetTextColor(m_crItemText);
		pRT->OffsetViewportOrg(-rc.left - pItem->nLevel * m_nIndent, -rc.top);
	}

	void STreeFile::DrawLines(IRenderTarget* pRT, const CRect& rc, HSTREEITEM hItem)
	{
		if (m_nIndent == 0 || !m_pLineSkin || !m_bHasLines)
			return;
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
		SList<HSTREEITEM> lstParent;
		HSTREEITEM hParent = GetParentItem(hItem);
		while (hParent)
		{
			lstParent.AddHead(hParent);
			hParent = GetParentItem(hParent);
		}
		//draw parent flags.
		enum { plus, plus_join, plus_bottom, minus, minus_join, minus_bottom, line, line_join, line_bottom };//9 line states
		CRect rcLine = rc;
		rcLine.right = rcLine.left + m_nIndent;
		//	rcLine.DeflateRect(0, (rcLine.Height() - m_nIndent) / 2);
		SPOSITION pos = lstParent.GetHeadPosition();
		while (pos)
		{
			HSTREEITEM hParent = lstParent.GetNext(pos);
			HSTREEITEM hNextSibling = GetNextSiblingItem(hParent);
			if (hNextSibling)
			{
				m_pLineSkin->DrawByIndex(pRT, rcLine, line);
			}
			rcLine.OffsetRect(m_nIndent, 0);
		}
		bool hasNextSibling = GetNextSiblingItem(hItem) != NULL;
		bool hasPervSibling = GetPrevSiblingItem(hItem) != NULL;
		bool hasChild = GetChildItem(hItem) != NULL;
		int iLine = -1;
		if (hasChild)
		{//test if is collapsed
			if (pItem->bCollapsed)
			{
				if (lstParent.IsEmpty() && !hasPervSibling)//no parent
					iLine = plus;
				else if (hasNextSibling)
					iLine = plus_join;
				else
					iLine = plus_bottom;
			}
			else
			{
				if (lstParent.IsEmpty() && !hasPervSibling)//no parent
					iLine = minus;
				else if (hasNextSibling)
					iLine = minus_join;
				else
					iLine = minus_bottom;
			}
		}
		else
		{
			if (hasNextSibling)
				iLine = line_join;
			else
				iLine = line_bottom;
		}
		m_pLineSkin->DrawByIndex(pRT, rcLine, iLine);
	}

	int STreeFile::ItemHitTest(HSTREEITEM hItem, CPoint& pt) const
	{
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
		int nHitTestBtn = STVIBtn_None;

		if (STVIMask_Toggle == (m_uItemMask & STVIMask_Toggle)
			&& pItem->bHasChildren
			&& m_rcToggle.PtInRect(pt))
			nHitTestBtn = STVIBtn_Toggle;
		else if (STVIMask_CheckBox == (m_uItemMask & STVIMask_CheckBox)
			&& m_rcCheckBox.PtInRect(pt))
			nHitTestBtn = STVIBtn_CheckBox;

		return nHitTestBtn;
	}


	void STreeFile::ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove)
	{
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
		DWORD oldState = pItem->dwToggleState;
		pItem->dwToggleState |= dwStateAdd;
		pItem->dwToggleState &= ~dwStateRemove;
		if (oldState == pItem->dwToggleState) return;
		CRect rcItem, rcUpdate = m_rcToggle;
		if (GetItemRect(pItem, rcItem))
		{
			rcUpdate.OffsetRect(rcItem.left, rcItem.top);
			InvalidateRect(rcUpdate);
		}
	}

	void STreeFile::ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove)
	{
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
		DWORD oldState = pItem->dwCheckBoxState;
		pItem->dwCheckBoxState |= dwStateAdd;
		pItem->dwCheckBoxState &= ~dwStateRemove;
		if (oldState == pItem->dwCheckBoxState) return;

		CRect rcItem, rcUpdate = m_rcCheckBox;
		if (GetItemRect(pItem, rcItem))
		{
			rcUpdate.OffsetRect(rcItem.left, rcItem.top);
			InvalidateRect(rcUpdate);
		}

	}

	void STreeFile::ItemLButtonDown(HSTREEITEM hItem, UINT nFlags, CPoint pt)
	{
		int nHitTestBtn = ItemHitTest(hItem, pt);
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

		//清除原有pushdown按钮
		if (m_nItemPushDownBtn != nHitTestBtn)
		{
			if (m_nItemPushDownBtn == STVIBtn_Toggle &&
				WndState_PushDown == (pItem->dwToggleState & WndState_PushDown))
			{
				ModifyToggleState(hItem, 0, WndState_PushDown);
			}

			if (m_nItemPushDownBtn == STVIBtn_CheckBox &&
				WndState_PushDown == (pItem->dwCheckBoxState & WndState_PushDown))
			{
				ModifyChekcBoxState(hItem, 0, WndState_PushDown);
			}

			m_nItemPushDownBtn = nHitTestBtn;
		}

		//置新pushdown按钮
		if (m_nItemPushDownBtn != STVIBtn_None)
		{
			if (m_nItemPushDownBtn == STVIBtn_Toggle &&
				WndState_PushDown != (pItem->dwToggleState & WndState_PushDown))
			{
				ModifyToggleState(hItem, WndState_PushDown, 0);
				Expand(pItem->hItem, TVE_TOGGLE);
			}

			if (m_nItemPushDownBtn == STVIBtn_CheckBox &&
				WndState_PushDown != (pItem->dwCheckBoxState & WndState_PushDown))
			{
				BOOL bCheck =
					pItem->nCheckBoxValue == STVICheckBox_Checked ? FALSE : TRUE;
				ModifyChekcBoxState(hItem, WndState_PushDown, 0);
				SetCheckState(pItem->hItem, bCheck);
			}
		}

		if (nHitTestBtn == STVIBtn_None) {
			//点击非按钮区域
			if (WndState_PushDown != (pItem->dwCheckBoxState & WndState_PushDown)) {
				BOOL bCheck =
					pItem->nCheckBoxValue == STVICheckBox_Checked ? FALSE : TRUE;
				SetCheckState(pItem->hItem, bCheck);//反置checkbox
			}
		}
	}

	void STreeFile::ItemLButtonUp(HSTREEITEM hItem, UINT nFlags, CPoint pt)
	{
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

		if (m_nItemPushDownBtn != STVIBtn_None)
		{
			if (m_nItemPushDownBtn == STVIBtn_Toggle &&
				WndState_PushDown == (pItem->dwToggleState & WndState_PushDown))
			{
				ModifyToggleState(hItem, 0, WndState_PushDown);
			}

			if (m_nItemPushDownBtn == STVIBtn_CheckBox &&
				WndState_PushDown == (pItem->dwCheckBoxState & WndState_PushDown))
			{
				ModifyChekcBoxState(hItem, 0, WndState_PushDown);
				EventTCCheckState evt(this);
				evt.hItem = hItem;
				evt.uCheckState = pItem->dwCheckBoxState;
				FireEvent(evt);
			}

			m_nItemPushDownBtn = STVIBtn_None;
		}
	}

	void STreeFile::ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags, CPoint pt)
	{
		if (!hItem)
		{
			return;
		}
		int nHitTestBtn = ItemHitTest(hItem, pt);
		if (nHitTestBtn == STVIBtn_CheckBox)
			ItemLButtonDown(hItem, nFlags, pt);
		//产生双击事件 add by zhaosheng
		EventTCDbClick dbClick(this);
		dbClick.bCancel = FALSE;
		dbClick.hItem = hItem;
		FireEvent(dbClick);
		if (!dbClick.bCancel)
		{
			Expand(hItem, TVE_TOGGLE);
		}
	}

	void STreeFile::ItemMouseMove(HSTREEITEM hItem, UINT nFlags, CPoint pt)
	{
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

		int nHitTestBtn = ItemHitTest(hItem, pt);

		if (nHitTestBtn != m_nItemHoverBtn)
		{
			if (m_nItemHoverBtn == STVIBtn_Toggle &&
				WndState_Hover == (pItem->dwToggleState & WndState_Hover))
			{
				ModifyToggleState(hItem, 0, WndState_Hover);
			}

			if (m_nItemHoverBtn == STVIBtn_CheckBox &&
				WndState_Hover == (pItem->dwCheckBoxState & WndState_Hover))
			{
				ModifyChekcBoxState(hItem, 0, WndState_Hover);
			}

			m_nItemHoverBtn = nHitTestBtn;
		}

		if (m_nItemHoverBtn != STVIBtn_None)
		{
			if (m_nItemHoverBtn == STVIBtn_Toggle &&
				WndState_Hover != (pItem->dwToggleState & WndState_Hover))
			{
				ModifyToggleState(hItem, WndState_Hover, 0);
			}

			if (m_nItemHoverBtn == STVIBtn_CheckBox &&
				WndState_Hover != (pItem->dwCheckBoxState & WndState_Hover))
			{
				ModifyChekcBoxState(hItem, WndState_Hover, 0);
			}
		}


		CRect rcItem;//修正
		if (GetItemRect(pItem, rcItem))//修正
		{
			InvalidateRect(rcItem);//修正
		}
	}

	void STreeFile::ItemMouseLeave(HSTREEITEM hItem)
	{
		LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);

		if (m_nItemHoverBtn != STVIBtn_None)
		{
			if (m_nItemHoverBtn == STVIBtn_Toggle &&
				WndState_Hover == (pItem->dwToggleState & WndState_Hover))
			{
				ModifyToggleState(hItem, 0, WndState_Hover);
			}

			if (m_nItemHoverBtn == STVIBtn_CheckBox &&
				WndState_Hover == (pItem->dwCheckBoxState & WndState_Hover))
			{
				ModifyChekcBoxState(hItem, 0, WndState_Hover);
			}

			m_nItemHoverBtn = STVIBtn_None;
		}


		CRect rcItem;//修正
		if (GetItemRect(pItem, rcItem))//修正
		{
			InvalidateRect(rcItem);//修正
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////

	void STreeFile::OnDestroy()
	{
		DeleteAllItems();
		__super::OnDestroy();
	}

	void STreeFile::OnPaint(IRenderTarget* pRT)
	{
		if (IsUpdateLocked()) return;

		CRect rcClient;
		SPainter painter;
		BeforePaint(pRT, painter);

		GetClientRect(rcClient);
		m_nRootWidth = rcClient.Width();

		int iFirstVisible = m_ptOrigin.y / m_nItemHei;
		int nPageItems = (m_rcClient.Height() + m_nItemHei - 1) / m_nItemHei + 1;

		int iVisible = -1;
		HSTREEITEM hItem = CSTree<LPTVITEMFILE>::GetNextItem(STVI_ROOT);
		while (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (pItem->bVisible) iVisible++;
			if (iVisible > iFirstVisible + nPageItems) break;
			if (iVisible >= iFirstVisible)
			{
				CRect rcItem(0, 0, m_nRootWidth, m_nItemHei);
				rcItem.OffsetRect(rcClient.left - m_ptOrigin.x,
					rcClient.top - m_ptOrigin.y + iVisible * m_nItemHei);
				DrawLines(pRT, rcItem, hItem);
				DrawItem(pRT, rcItem, hItem);
			}
			if (pItem->bCollapsed)
			{//跳过被折叠的项
				HSTREEITEM hChild = GetChildItem(hItem, FALSE);
				while (hChild)
				{
					hItem = hChild;
					hChild = GetChildItem(hItem, FALSE);
				}
			}
			hItem = CSTree<LPTVITEMFILE>::GetNextItem(hItem);
		}
		AfterPaint(pRT, painter);
	}

	void STreeFile::OnLButtonDown(UINT nFlags, CPoint pt)
	{
		__super::OnLButtonDown(nFlags, pt);
		m_hHoverItem = HitTest(pt);


		if (m_hHoverItem)
		{
			m_hCaptureItem = m_hHoverItem;
			ItemLButtonDown(m_hHoverItem, nFlags, pt);
		}
	}

	void STreeFile::OnRButtonDown(UINT nFlags, CPoint pt)
	{
		CPoint pt2 = pt;
		if (!m_bRightClickSel)
		{
			__super::OnRButtonDown(nFlags, pt2);
			return;
		}

		m_hHoverItem = HitTest(pt);

		__super::OnRButtonDown(nFlags, pt2);
	}

	void STreeFile::OnLButtonUp(UINT nFlags, CPoint pt)
	{
		__super::OnLButtonUp(nFlags, pt);

		m_hHoverItem = HitTest(pt);

		if (m_hCaptureItem)
		{
			ItemLButtonUp(m_hCaptureItem, nFlags, pt);
			m_hCaptureItem = NULL;
			return;
		}

		if (m_hHoverItem)
			ItemLButtonUp(m_hHoverItem, nFlags, pt);
	}

	void STreeFile::OnLButtonDbClick(UINT nFlags, CPoint pt)
	{
		m_hHoverItem = HitTest(pt);
		ItemLButtonDbClick(m_hHoverItem, nFlags, pt);
	}

	void STreeFile::OnMouseMove(UINT nFlags, CPoint pt)
	{
		HSTREEITEM hHitTest = HitTest(pt);

		if (hHitTest != m_hHoverItem)
		{
			if (m_hHoverItem)
				ItemMouseLeave(m_hHoverItem);

			m_hHoverItem = hHitTest;
		}
		if (m_hHoverItem)
			ItemMouseMove(m_hHoverItem, nFlags, pt);
	}

	void STreeFile::OnMouseLeave()
	{
		if (m_hHoverItem)
		{
			ItemMouseLeave(m_hHoverItem);
			m_hHoverItem = NULL;
		}
	}



	int STreeFile::CalcItemWidth(const LPTVITEMFILE pItemObj)
	{
		return pItemObj->nContentWidth + pItemObj->nLevel * m_nIndent;
	}



	BOOL STreeFile::VerifyItem(HSTREEITEM hItem) const
	{
		if (!hItem) return FALSE;
#ifdef _DEBUG
		HSTREEITEM hRoot = CSTree<LPTVITEMFILE>::GetRootItem(hItem);
		while (CSTree<LPTVITEMFILE>::GetPrevSiblingItem(hRoot))
		{
			hRoot = CSTree<LPTVITEMFILE>::GetPrevSiblingItem(hRoot);
		}
		return hRoot == GetRootItem();
#endif
		return TRUE;
	}

}//namespace SOUI