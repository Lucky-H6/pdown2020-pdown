//////////////////////////////////////////////////////////////////////////
//  Class Name: STreeFile
//     Version: 2012.12.16 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////


#pragma once
#include <vector>
#include <string>
#include "souistd.h"
#include "core/SPanel.h"
#include "control/STree.h"
#include "../UIAdapter/DownItem.h"
using namespace SOUI;

namespace SOUI {


	typedef struct tagTVITEM_FILE {
		DowningItem FileInfo;
		bool            bIsDir = false;
		int            nImage;//img=1 收起状态图标
		int            nSelectedImage;//selImg=2 展开状态图标

		HSTREEITEM    hItem;
		BOOL        bCollapsed;
		BOOL        bVisible;
		BOOL        bHasChildren;
		int            nLevel;
		int            nCheckBoxValue;
		int         nContentWidth;
		DWORD        dwToggleState;
		DWORD       dwCheckBoxState;

		tagTVITEM_FILE()
		{
			nImage = -1;
			nSelectedImage = -1;

			hItem = NULL;
			bCollapsed = FALSE;
			bVisible = TRUE;
			bHasChildren = FALSE;
			nLevel = 0;
			nCheckBoxValue = STVICheckBox_UnChecked;
			nContentWidth = 0;
			dwToggleState = WndState_Normal;
			dwCheckBoxState = WndState_Normal;
		}

	} TVITEMFILE, * LPTVITEMFILE;


	class SOUI_EXP STreeFile
		: public SScrollView
		, protected CSTree<LPTVITEMFILE>
	{
		SOUI_CLASS_NAME(STreeFile, L"treefile")
	public:

		STreeFile();

		virtual ~STreeFile();

		HSTREEITEM InsertItem(DowningItem& FileInfo, bool bIsDir, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST, BOOL bEnsureVisible = TRUE);
		HSTREEITEM InsertItem(DowningItem& FileInfo, bool bIsDir, int nImage,
			int nSelectedImage, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST, BOOL bEnsureVisible = TRUE);

		BOOL RemoveItem(HSTREEITEM hItem);
		void RemoveAllItems();

		HSTREEITEM GetRootItem() const;
		HSTREEITEM GetNextSiblingItem(HSTREEITEM hItem) const;
		HSTREEITEM GetPrevSiblingItem(HSTREEITEM hItem) const;
		HSTREEITEM GetChildItem(HSTREEITEM hItem, BOOL bFirst = TRUE) const;
		HSTREEITEM GetParentItem(HSTREEITEM hItem) const;
		HSTREEITEM GetNextItem(HSTREEITEM hItem) const { return CSTree<LPTVITEMFILE>::GetNextItem(hItem); }


		BOOL GetItemFileID(HSTREEITEM hItem, SStringT& strText) const;
		BOOL GetItemImage(HSTREEITEM hItem, int& nImage, int& nSelectedImage) const;
		BOOL SetItemImage(HSTREEITEM hItem, int nImage, int nSelectedImage);

		BOOL ItemHasChildren(HSTREEITEM hItem);

		BOOL GetCheckState(HSTREEITEM hItem) const;
		BOOL SetCheckState(HSTREEITEM hItem, BOOL bCheck);

		BOOL Expand(HSTREEITEM hItem, UINT nCode = TVE_EXPAND);
		BOOL EnsureVisible(HSTREEITEM hItem);

		void PageUp();
		void PageDown();

		HSTREEITEM HitTest(CPoint& pt);

		/*获取所有已选中的文件ID*/
		std::vector<std::wstring> GetAllCheckedFileID();
		/*绑定state显示text控件，选择文件时同步显示*/
		void BindStateText(SWindow* statetext);
		/*刷新state显示*/
		void RefreshStateText();

	protected:

		virtual BOOL CreateChildren(pugi::xml_node xmlNode);
		virtual void LoadBranch(HSTREEITEM hParent, pugi::xml_node xmlNode);
		virtual void LoadItemAttribute(pugi::xml_node xmlNode, LPTVITEMFILE pItem);

		HSTREEITEM InsertItem(LPTVITEMFILE pItemObj, HSTREEITEM hParent, HSTREEITEM hInsertAfter, BOOL bEnsureVisible);
		HSTREEITEM InsertItem(pugi::xml_node xmlNode, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST, BOOL bEnsureVisible = FALSE);

		BOOL IsAncestor(HSTREEITEM hItem1, HSTREEITEM hItem2);
		BOOL VerifyItem(HSTREEITEM hItem) const;

		void SetChildrenVisible(HSTREEITEM hItem, BOOL bVisible);

		void SetChildrenState(HSTREEITEM hItem, int nCheckValue);
		BOOL CheckChildrenState(HSTREEITEM hItem, BOOL bCheck);
		void CheckState(HSTREEITEM hItem);

		virtual void ItemLayout();
		virtual void CalcItemContentWidth(LPTVITEMFILE pItem);
		virtual int  CalcItemWidth(const LPTVITEMFILE pItem);
		virtual int  GetMaxItemWidth();
		virtual int  GetMaxItemWidth(HSTREEITEM hItem);
		int  GetItemShowIndex(HSTREEITEM hItemObj);
		BOOL GetItemRect(LPTVITEMFILE pItem, CRect& rcItem);

		void RedrawItem(HSTREEITEM hItem);
		virtual void DrawItem(IRenderTarget* pRT, const CRect& rc, HSTREEITEM hItem);
		virtual void DrawLines(IRenderTarget* pRT, const CRect& rc, HSTREEITEM hItem);

		int ItemHitTest(HSTREEITEM hItem, CPoint& pt) const;
		void ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);
		void ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);

		void ItemLButtonDown(HSTREEITEM hItem, UINT nFlags, CPoint pt);
		void ItemLButtonUp(HSTREEITEM hItem, UINT nFlags, CPoint pt);
		void ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags, CPoint pt);

		void ItemMouseMove(HSTREEITEM hItem, UINT nFlags, CPoint pt);
		void ItemMouseLeave(HSTREEITEM hItem);

		static std::wstring FormatFileSize(int64_t filesize);
	protected:

		void OnDestroy();

		void OnPaint(IRenderTarget* pRT);

		void OnLButtonDown(UINT nFlags, CPoint pt);
		void OnLButtonUp(UINT nFlags, CPoint pt);
		void OnLButtonDbClick(UINT nFlags, CPoint pt);
		void OnRButtonDown(UINT nFlags, CPoint pt);

		void OnMouseMove(UINT nFlags, CPoint pt);
		void OnMouseLeave();

		virtual void OnNodeFree(LPTVITEMFILE& pItemData) {
			delete pItemData;
		}
		virtual void OnInsertItem(LPTVITEMFILE& pItemData)
		{
		}
	protected:
		SWindow* m_StateText;

		HSTREEITEM    m_hHoverItem;
		HSTREEITEM    m_hCaptureItem;

		int            m_nVisibleItems = 0;
		int         m_nMaxItemWidth = 0;
		int         m_nRootWidth = 0;

		UINT        m_uItemMask;
		int         m_nItemOffset = 0;
		CRect       m_rcToggle;
		CRect       m_rcCheckBox;
		CRect       m_rcIcon;
		int            m_nItemHoverBtn = 0;
		int            m_nItemPushDownBtn = 0;

		int            m_nItemHei = 0, m_nIndent = 0, m_nItemMargin = 0;
		BOOL        m_bCheckBox;
		BOOL        m_bRightClickSel;
		ISkinObj* m_pItemBgSkin, * m_pItemSelSkin;
		ISkinObj* m_pIconSkin, * m_pToggleSkin, * m_pCheckSkin;
		ISkinObj* m_pLineSkin;

		COLORREF m_crItemBg, m_crItemSelBg;
		COLORREF m_crItemText, m_crItemSelText;
		BOOL	 m_bHasLines; /**< has lines*/


		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"indent", m_nIndent, TRUE)
			ATTR_INT(L"itemHeight", m_nItemHei, TRUE)
			ATTR_INT(L"itemMargin", m_nItemMargin, TRUE)
			ATTR_INT(L"checkBox", m_bCheckBox, TRUE)
			ATTR_INT(L"rightClickSel", m_bRightClickSel, TRUE)
			ATTR_SKIN(L"itemBkgndSkin", m_pItemBgSkin, TRUE)
			ATTR_SKIN(L"itemSelSkin", m_pItemSelSkin, TRUE)
			ATTR_SKIN(L"toggleSkin", m_pToggleSkin, TRUE)
			ATTR_SKIN(L"iconSkin", m_pIconSkin, TRUE)
			ATTR_SKIN(L"checkSkin", m_pCheckSkin, TRUE)
			ATTR_SKIN(L"lineSkin", m_pLineSkin, TRUE)
			ATTR_COLOR(L"colorItemBkgnd", m_crItemBg, FALSE)
			ATTR_COLOR(L"colorItemSelBkgnd", m_crItemSelBg, FALSE)
			ATTR_COLOR(L"colorItemText", m_crItemText, FALSE)
			ATTR_COLOR(L"colorItemSelText", m_crItemSelText, FALSE)
			ATTR_BOOL(L"hasLines", m_bHasLines, TRUE)
			SOUI_ATTRS_END()

			SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_RBUTTONDOWN(OnRButtonDown);
		MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
			SOUI_MSG_MAP_END()
	};

}//namespace SOUI
