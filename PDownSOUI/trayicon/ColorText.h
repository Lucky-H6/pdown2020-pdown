#pragma once
#include "core/SWnd.h"
/**
 * @class      SStatic
 * @brief      静态文本控件类
 *
 * Describe    静态文本控件可支持多行，有多行属性时，\n可以强制换行
 * Usage       <text>inner text example</text>
 */
class SOUI_EXP ColorText : public SWindow
{
    SOUI_CLASS_NAME(ColorText, L"colortext")
public:
    /**
     * SStatic::SStatic
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    ColorText();
    /**
     * SStatic::SDrawText
     * @brief    绘制文本
     * @param    IRenderTarget *pRT -- 绘制设备句柄
     * @param    LPCTSTR pszBuf -- 文本内容字符串
     * @param    int cchText -- 字符串长度
     * @param    LPRECT pRect -- 指向矩形结构RECT的指针
     * @param    UINT uFormat --  正文的绘制选项
     *
     * Describe  对DrawText封装
     */
    virtual void DrawText(IRenderTarget* pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

protected:
   
    SOUI_ATTRS_BEGIN()
        SOUI_ATTRS_END()
};