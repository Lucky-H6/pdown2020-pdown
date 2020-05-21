#pragma once
#include "../Utils/JsonHelper.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "../Utils/Base64.h"
#include "DBHelper.h"

struct SerAddNewLink {
	std::wstring state = L"error";
	std::wstring bdlink = L"";
	std::wstring surl = L"";
	std::wstring pwd = L"";
	std::wstring bdclnd = L"";
	std::wstring fileliststate = L"";
	std::wstring shareid = L"";
	std::wstring uk = L"";
};

struct SerFileItem
{
	wstring share_fsid = L"";
	bool isdir = false;
	size_t local_mtime = 0;
	wstring path = L"";;
	wstring server_filename = L"";
	int64_t size = 0;

	wstring FileTime = L"";
	wstring sizestr = L"";
	vector<SerFileItem> ChildList;

	SerFileItem() {}
	SerFileItem(std::wstring share_fsid, bool isdir, size_t local_mtime, std::wstring path, std::wstring server_filename, int64_t size)
		:share_fsid(std::move(share_fsid)), isdir(isdir), local_mtime(local_mtime), path(std::move(path)), server_filename(std::move(server_filename)), size(size), ChildList()
	{
		FileTime = S2W::StringtoWString(StringHelper::FormatJsTimeStr(local_mtime, true), CP_UTF8);
		sizestr = StringHelper::FormatFileSize(size);
	}
};

class ServerAPI_Add {

	static inline std::wstring Url(std::wstring part) {
		return ServerIPAPI + part;
	}
	static inline std::wstring Header(std::wstring part = L"") {
		return L"";
	}
public:
	/*
	服务器返回解析出的百度链接信息（bdlink,surl,pwd,bdclnd,filestate）
	result.state: "success" 或者 "error"
	*/
	static SerAddNewLink AddNewLink(std::wstring linkstr) {

		SerAddNewLink result = SerAddNewLink();

		
		return result;
	}

private:

public:
	/*从服务器读取缓存的 首页文件列表(会保存到ChildList,更新shareid,uk) 返回"success" 或者 错误信息*/
	static std::wstring GetLinkIndexFileList(SerAddNewLink& link, vector<JsonFileItem>& ChildList) {
		
		return L"网络错误";
	}
	/*把json备份到服务器并返回解析出的 首页文件列表 返回"success" 或者 错误信息*/
	static std::wstring	PostLinkIndexFileList(SerAddNewLink& link, std::wstring& jsonstr) {

		return L"网络错误";
	}

	/*把完整的文件列表发送到服务器缓存起来*/
	static std::wstring	PostLinkDirFileList(std::wstring bdlink, vector<JsonFileItem>& FileList) {

		
		return L"网络错误";
	}

	/*从服务器读取缓存的 完整文件列表(会保存到ChildList) 返回"success" 或者 错误信息*/
	static std::wstring GetLinkDirFileList(std::wstring bdlink, vector<SerFileItem>& ChildList) {
		
		return L"网络错误";
	}

};
