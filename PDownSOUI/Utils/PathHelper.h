#pragma once
#include <Shlobj.h>  
#include <windows.h>
#include "StringHelper.h"
class PathHelper {

public:

	/*C:\Users\vmpc\AppData\Local\*/
	static std::wstring GetLocalAppDataDir()
	{
		std::wstring temp_path;
		PWSTR pszPath = NULL;
		if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,
			0, NULL, &pszPath))) {
			return L"";
		}
		temp_path = pszPath;
		::CoTaskMemFree(pszPath);

		if (!temp_path.empty())
			if (temp_path.back() != L'\\')
				temp_path.push_back(L'\\');
		return temp_path;
	}
	/*返回并创建目录 C:\Users\vmpc\AppData\Local\app_name\*/
	static std::wstring GetLocalAppDataDir(std::wstring app_name) {
		std::wstring dir = GetLocalAppDataDir();
		dir.append(app_name);
		dir.append(L"\\");
		if (CreatDir(dir)) return dir;
		else return L"";
	}
	/*返回并创建目录 C:\Users\vmpc\AppData\Local\app_name\app_name.exe*/
	static std::wstring GetAppStartDir() {
		wchar_t szCurrentDir[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, szCurrentDir, sizeof(szCurrentDir));
		return std::wstring(szCurrentDir);
	}
	//<,>,|,*,?,,/
	/*清理路径里的非法字符 :*?\"<>| 并替换/为\,并删除.\  ..\ 并确保长度<250*/
	static std::wstring PathCleanup(std::wstring dirfile) {
		//c:\DownDir\sss.pdf
		wstring outstr = L"";

		wchar_t safe[500];
		const wchar_t* ddd = dirfile.c_str();
		size_t len = dirfile.length();
		if (len > 500) return outstr;

		safe[0] = ddd[0];
		if (ddd[1] != L':') return outstr;
		safe[1] = ddd[1];
		if (ddd[2] != L'/' && ddd[2] != L'\\') return outstr;
		safe[2] = L'\\';
		//c:\


		size_t s = 3;
		wchar_t p = L' ', old = L'\\';
		wstring del = L":*?\"<>|";
		for (size_t i = 3; i < len; i++) {
			p = ddd[i];
			if (del.find(p) != wstring::npos) continue;
			if (p == L'/') p = L'\\';

			if (p == L'\\') {
				if (old == L'\\') continue;
				while (old == L'.') {
					s--; old = safe[s - 1];
				} //如果出现.\或者..\ s--就会删掉前面的.  old = safe[s - 1];会删除前面第二个.
			}
			old = p;
			safe[s] = p;
			s++;
		}
		safe[s] = L'\0';
		outstr = wstring(safe, s);
		size_t pos = outstr.find_last_of('\\');//最小是==2

		wstring dirpath = outstr.substr(0, pos);
		wstring filename = outstr.substr(pos + 1);
		size_t fnlen = filename.length();
		if (fnlen > 221) {//文件名太长，截取掉
			filename = filename.substr(0, 220);
			fnlen = filename.length();
		}

		size_t dirlen = MAX_PATH - fnlen - 10;//目录最长  10 是为了富裕,因为文件名最长220，所以目录名最短是30
		while (dirpath.length() > dirlen) {
			dirpath = dirpath.substr(0, dirpath.find_last_of('\\'));
		}
		outstr = dirpath + L"\\" + filename;//30+1+220

		return outstr;
	}
	/*创建文件夹（多层）*/
	static bool CreatDir(std::wstring full_dir) {
		if (_waccess(full_dir.c_str(), 0) == 0) return true;//文件夹存在
		HRESULT result = ::SHCreateDirectory(NULL, full_dir.c_str());
		if (result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS)
			return true;
		return false;
	}

	static bool GetDir(std::wstring full_file, std::wstring& dirpath, std::wstring& filename) {
		size_t pos = full_file.find_last_of('\\');
		if (pos != wstring::npos) {
			dirpath = full_file.substr(0, pos + 1);
			filename = full_file.substr(pos + 1);
			return true;
		}
		return false;
	}

	static bool OpenFileAutoCreat(std::wstring full_file, FILE*& fp, uint64_t& filepos) {
		//errno_t err = _wsopen_s(&fp, fileSave.c_str(), _O_APPEND | _O_BINARY | _O_CREAT | _O_RDWR, _SH_DENYWR, _S_IREAD | _S_IWRITE);
		if (_waccess(full_file.c_str(), 0) != 0)//文件不存在
		{
			std::wstring filePath = full_file.substr(0, full_file.find_last_of(L'\\'));
			CreatDir(filePath);//创建目录
			fp = _wfsopen(full_file.c_str(), L"w+b", _SH_DENYWR);//bin形式，重新创建文件 读写
		}
		else {
			fp = _wfsopen(full_file.c_str(), L"r+b", _SH_DENYWR);//bin形式，打开已存在的文件 读写
		}
		if (fp != nullptr) {//成功打开	
			_fseeki64(fp, 0, SEEK_END);
			filepos = _ftelli64(fp);
			return true;
		}
		else {
			filepos = 0;
			return false;
		}
	}

	static bool OpenFile(std::wstring full_file, FILE*& fp, uint64_t& filesize) {
		//errno_t err = _wsopen_s(&fp, fileSave.c_str(), _O_APPEND | _O_BINARY | _O_CREAT | _O_RDWR, _SH_DENYWR, _S_IREAD | _S_IWRITE);
		if (_waccess(full_file.c_str(), 0) != 0)//文件不存在
		{
			filesize = 0;
			return false;
		}

		fp = _wfsopen(full_file.c_str(), L"r+b", _SH_DENYWR);//bin形式，追加写入  _SH_DENYWR拒绝其他写入
		if (fp != nullptr) {//成功打开	
			_fseeki64(fp, 0, SEEK_END);
			filesize = _ftelli64(fp);
			_fseeki64(fp, 0, SEEK_SET);
			return true;
		}
		else {
			filesize = 0;
			return false;
		}
	}



};