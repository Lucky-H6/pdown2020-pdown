#pragma once
#include <string>
#include "../Utils/JsonHelper.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "BaiDuYun.h"
#include "ServerAPI_Add.h"
#include "../UIPage/UIAddNew.h"
#include "event/SNotifyCenter.h"
class DALAddNew {
private:
	static void ShowUIMessage(bool IsEndError, std::wstring msg) {
		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([IsEndError, msg] {
			UIAddNew::GetI()->ShowStepMsg(IsEndError, msg);
			});
	}
	static void ShowUIFileTree(vector<SerFileItem>& FileList) {
		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([FileList] {
			UIAddNew::GetI()->ShowFileTree(FileList);
			});
	}



	/*递归解析包含的所有文件夹*/
	static std::wstring GetSerDirFileList(SerAddNewLink& Link, JsonFileItem& DirItem) {
		std::wstring dirname = DirItem.server_filename;
		ShowUIMessage(false, L"开始解析文件夹:" + dirname);
		std::wstring isget = L"";
		std::string outjson = "";
		DirItem.ZiList.clear();
		DirItem.ZiList.reserve(1000);
		for (int r = 1; r < 20; r++) {//最多2000个文件
			if (r > 1) ShowUIMessage(false, L"开始解析文件夹" + to_wstring(r) + L":" + dirname);
			isget = BaiDuYun::GetLinkDirFileList(Link.bdlink, Link.bdclnd, Link.shareid, Link.uk, DirItem.path, r, outjson);
			if (isget != L"success") {
				ShowUIMessage(false, L"解析文件夹失败:" + dirname);
				return L"error";
			}
			Json::Value json_object;
			int iserrno = -1;
			isget = JsonHelper::GetJsonWith_errno(outjson, json_object, iserrno);
			if (isget != L"success") {
				if (iserrno == -21) ShowUIMessage(false, L"解析失败:分享链接已失效或被删除");
				else if (iserrno == -9) ShowUIMessage(false, L"解析失败:文件可能已经被删除");
				else if (iserrno == -3) ShowUIMessage(false, L"解析失败:百度返回操作太频繁");
				else ShowUIMessage(false, L"解析失败:百度返回错误代码 " + to_wstring(iserrno));
				//else ShowUIMessage(false, L"解析json失败:" + dirname);
				return L"error";
			}

			try {
				json_object = json_object["list"];
				/*
				[{	"category": 2,
	"fs_id": 651684469246892,
	"isdir": 0,
	"local_ctime": 1569138544,
	"local_mtime": 1569138550,
	"md5": "666acae16304fa85aa914f19d5f1d50a",
	"path": "\/1-300\/300.mp3",
	"server_ctime": 1569155773,
	"server_filename": "300.mp3",
	"server_mtime": 1578372053,
	"size": 11016723 }]
				*/
				if (json_object) {
					size_t len = json_object.size();
					ServerAPI_Add::GetSerJsonFileItemList(json_object, DirItem.ZiList);
					if (len < 100) break;//全部列出来了
				}
			}
			catch (...) {
				SLOGFMTE(L"unkownError %d", 2);
				ShowUIMessage(false, L"未知错误:" + dirname);
				return L"error";
			}
		}
		DirItem.ZiList.shrink_to_fit();
		ShowUIMessage(false, L"结束解析文件夹:" + dirname + L"  (" + to_wstring(DirItem.ZiList.size()) + L")");
		//遍历子文件夹
		bool iserror = false;
		for (auto iter = DirItem.ZiList.begin(); iter != DirItem.ZiList.end(); iter++)
		{
			if (iter->isdir) {
				std::wstring isok = GetSerDirFileList(Link, *iter);
				if (isok != L"success") iserror = true;
			}
		}
		return  (iserror ? L"error" : L"success");
	}

	static std::wstring GetLinkOnlyOneFileName(SerAddNewLink& Link, JsonFileItem& FileItem) {
		ShowUIMessage(false, L"开始解析文件名:" + FileItem.server_filename);
		std::wstring isget = L"";
		std::string outjson = "";

		isget = BaiDuYun::GetLinkOnlyOneFileName(Link.bdlink, Link.bdclnd, Link.shareid, Link.uk, outjson);
		if (isget != L"success") {
			ShowUIMessage(false, L"解析文件名失败:" + FileItem.server_filename);
			return L"error";
		}

		Json::Value json_object;
		int iserrno = -1;
		isget = JsonHelper::GetJsonWith_errno(outjson, json_object, iserrno);
		if (isget != L"success") {
			ShowUIMessage(false, L"解析json失败:" + FileItem.server_filename);
			return L"error";
		}

		try {
			/*
			{

	"errno": 0,

	"list": [{
		"category": "6",
		"fs_id": "383946898409085",
		"isdir": "0",
		"local_ctime": "1560361280",
		"local_mtime": "1560361280",
		"md5": "8f7a2c9fedae7a135e081742ff0340ac",
		"path": "\/iPlaySoft.com_Feem_v4.3.0_beta_Win.zip",
		"server_ctime": "1560361281",
		"server_filename": "iPlaySoft.com_Feem_v4.3.0_beta_Win.zip",
		"server_mtime": "1560361281",
		"size": "35641663",
		"dlink": ""
	}]
}
			*/
			json_object = json_object["list"];

			size_t len = json_object.size();
			if (len <= 0) {
				SLOGFMTE(L"listlenError outjson=%hs", outjson.c_str());
				ShowUIMessage(false, L"解析文件名失败:" + FileItem.server_filename);
				return L"error";
			}

			Json::Value& current = json_object[0];//这里是百度返回的，fs_id特殊，是字符串
			int64_t fs_id = stoll(current["fs_id"].asString());
			wstring md5 = S2W::StringtoWString(current["md5"].asString(), CP_UTF8);
			wstring path = S2W::StringtoWString(current["path"].asString(), CP_UTF8);
			wstring server_filename = S2W::StringtoWString(current["server_filename"].asString(), CP_UTF8);//其实只有这个文件名是需要的

			if (FileItem.fs_id != fs_id) {
				SLOGFMTE(L"fs_idError outjson=%hs", outjson.c_str());
				ShowUIMessage(false, L"解析文件名失败:" + FileItem.server_filename);
				return L"error";
			}
			FileItem.md5 = md5;
			FileItem.path = path;
			FileItem.server_filename = server_filename;
		}
		catch (...) {
			SLOGFMTE(L"unkownError %d", 2);
			ShowUIMessage(false, L"未知错误:" + FileItem.server_filename);
			return L"error";
		}

		ShowUIMessage(false, L"结束解析文件名:" + FileItem.server_filename);
		return   L"success";
	}
public:
	static void SubmiteLink(std::wstring linkstr, bool isrefresh) {
		ShowUIMessage(false, L"开始解析链接");
		auto link = ServerAPI_Add::AddNewLink(linkstr);//1 从服务器解析出bdlink
		if (link.state != L"success") return ShowUIMessage(true, link.state);
		ShowUIMessage(false, L"成功提交链接到服务器");
		if (isrefresh) link.fileliststate = L"notfound";
		wstring isget;
		if (link.fileliststate == L"notfound" || link.fileliststate == L"") {//需要从百度解析 首页文件列表			
			ShowUIMessage(false, L"从百度解析链接首页包含的文件");
			wstring indexjson = L"";
			isget = BaiDuYun::GetLinkIndexFileList(link.bdlink, link.pwd, link.surl, link.bdclnd, indexjson);
			if (isget != L"success") return ShowUIMessage(true, L"解析失败," + isget);

			ShowUIMessage(false, L"提取码:" + link.bdclnd == L"" ? L"公开分享不需要" : link.bdclnd);

			isget = ServerAPI_Add::PostLinkIndexFileList(link, indexjson);//缓存到服务器
			if (isget != L"success") return ShowUIMessage(true, L"服务器保存链接失败，请稍后重试");
			link.fileliststate = L"index";
		}


		if (link.fileliststate == L"index") {//需要解析全部文件
			ShowUIMessage(false, L"格式化&&解析首页文件列表");
			vector<JsonFileItem> JsonList;
			isget = ServerAPI_Add::GetLinkIndexFileList(link, JsonList);//更新Info的uk shareid
			if (isget != L"success") return ShowUIMessage(true, L"服务器读取链接内文件失败，请稍后重试");
			if (JsonList.size() == 0) return ShowUIMessage(true, L"解析链接失败，链接内不包含任何文件");//正常情况下不可能发生,所以是出错

			ShowUIMessage(false, L"开始递归分析所有文件夹内的文件");
			bool iserror = false;

			if (JsonList.size() == 1 && JsonList[0].isdir == false) {//只有一个文件的特殊
				std::wstring isok = GetLinkOnlyOneFileName(link, JsonList[0]);
				if (isok != L"success") iserror = true;
			}
			else {
				for (auto iter = JsonList.begin(); iter != JsonList.end(); iter++)
				{
					if (iter->isdir) {//递归解析包含的所有文件夹
						std::wstring isok = GetSerDirFileList(link, *iter);
						if (isok != L"success") iserror = true;
					}
				}
			}

			if (iserror) {
				return ShowUIMessage(true, L"结束分析,部分文件解析出错");
			}
			else {
				ShowUIMessage(false, L"结束分析,全部解析成功");
				ServerAPI_Add::PostLinkDirFileList(link.bdlink, JsonList);//缓存到服务器
				link.fileliststate = L"cache";
			}
		}

		//最后了，从服务器读取全部文件（这里才包括share_fsid）
		vector<SerFileItem> FileList;
		ShowUIMessage(false, L"从服务器读取缓存的文件列表");
		isget = ServerAPI_Add::GetLinkDirFileList(link.bdlink, FileList);
		if (isget != L"success") return ShowUIMessage(true, L"服务器读取链接失败，请稍后重试");

		//ShowUIMessage(false, L"模拟网络延时");
		//Sleep(2000);//模拟网络延时
		//把FileList绑定到UI,显示filetree
		ShowUIMessage(false, L"解析完成");
		ShowUIFileTree(FileList);
	}
};