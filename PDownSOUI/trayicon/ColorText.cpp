#include "stdafx.h"
#include "souistd.h"
#include "ColorText.h"
#include <string>
using namespace SOUI;

ColorText::ColorText()
{
	m_bMsgTransparent = TRUE;
	m_style.SetAlign(DT_LEFT);
	m_style.SetVAlign(DT_TOP);
}

void ColorText::DrawText(IRenderTarget* pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat) {
	//pRT->DrawText(pszBuf + iBegin, cchText, pRect, uFormat);

	UINT format = uFormat;// DT_SINGLELINE& DT_LEFT& DT_VCENTER;
	if (cchText == -1) cchText = (int)_tcslen(pszBuf);

	//pRT->DrawText(pszBuf, cchText, rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
	SIZE szWord;
	pRT->MeasureText(_T("A"), 1, &szWord);
	int nLineHei = szWord.cy;
	int nLeft = pRect->left;
	int nRight = pRect->right;
	int nBottom = pRect->top + nLineHei;
	COLORREF color = pRT->GetTextColor();
	COLORREF colorred = RGBA(255, 0, 0, 255);

	std::wstring str = std::wstring(pszBuf);
	int find_start = 0, find_end = 0, pos_now = 0, pos_end = cchText;

	CRect rcText;
	while (true) {
		find_start = str.find(L'[', pos_now);
		if (find_start == std::wstring::npos) break;
		pRT->SetTextColor(color);
		rcText.SetRect(nLeft, pRect->top, nRight, nBottom);
		pRT->DrawTextW((pszBuf + pos_now), find_start - pos_now, &rcText, format);

		rcText.SetRect(0, 0, 0, 0);
		pRT->MeasureText((pszBuf + pos_now), find_start - pos_now, &szWord);
		nLeft += szWord.cx;

		pos_now = find_start;
		find_end = str.find(L']', pos_now);
		if (find_end == std::wstring::npos) break;
		pos_now++;//跳过[不显示
		nLeft += 1;//间隔

		pRT->SetTextColor(colorred);
		rcText.SetRect(nLeft, pRect->top, nRight, nBottom);
		pRT->DrawTextW((pszBuf + pos_now), find_end - pos_now, &rcText, format);

		rcText.SetRect(0, 0, 0, 0);
		pRT->MeasureText((pszBuf + pos_now), find_end - pos_now, &szWord);
		nLeft += szWord.cx;

		pos_now = find_end;
		pos_now++;//跳过]不显示
		nLeft += 1;//间隔
	}
	if (pos_now < pos_end) {
		pRT->SetTextColor(color);
		//pRT->OffsetViewportOrg(pRect->left, pRect->top + nLineHei*2);
		rcText.SetRect(nLeft, pRect->top, nRight, nBottom);
		pRT->DrawTextW((pszBuf + pos_now), pos_end - pos_now, &rcText, format);

		rcText.SetRect(0, 0, 0, 0);
		pRT->MeasureText((pszBuf + pos_now), pos_end - pos_now, &szWord);
		nLeft += szWord.cx;
	}

	pRect->right = nLeft;
	pRect->bottom = nBottom;
}


/*
CFont font;

font.CreateFont(
50, // nHeight
0, // nWidth
0, // nEscapement
0, // nOrientation
FW_NORMAL, // nWeight
FALSE, // bItalic
FALSE, // bUnderline
0, // cStrikeOut
ANSI_CHARSET, // nCharSet
OUT_DEFAULT_PRECIS, // nOutPrecision
CLIP_DEFAULT_PRECIS, // nClipPrecision
DEFAULT_QUALITY, // nQuality
DEFAULT_PITCH | FF_SWISS,
_T("Arial") // nPitchAndFamily Arial
);
pDc->SelectObject(&font);

*/