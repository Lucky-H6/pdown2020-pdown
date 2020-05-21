#pragma once
#include <string>
using namespace std;
struct HomeItem
{
	/*完整的链接+密码*/
	wstring Link = L"";
	/*链接发布者用户名 短的UI显示用*/
	wstring UserName = L"";
	/*链接发布者用户ID 加密后的搜索用*/
	wstring UserID = L"";
	/*文件名 UI显示用*/
	wstring FileName = L"";
};