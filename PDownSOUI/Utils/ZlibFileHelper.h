#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "windows.h"
#include "../third-part/zlib/zlib.h"
#include "PathHelper.h"

#define CHUNK 16384
class ZlibFileHelper {
public:
	ZlibFileHelper() {}
	~ZlibFileHelper() {}
private:
	FILE* file_zlib = nullptr;
	z_stream strm;
public:
	/*创建zlib文件打开写入，成功返回true*/
	bool CreatZlibFile(std::wstring savefile) {
		uint64_t filelen = 0;
		if (PathHelper::OpenFileAutoCreat(savefile, file_zlib, filelen))
		{
			return true;
		}
		return false;
	}

	bool PackOneFile(std::wstring file)
	{
		FILE* source = nullptr;
		source = _wfsopen(file.c_str(), L"rb", _SH_DENYWR);//bin形式，打开已存在的文件 读写
		if (!source) return false;
		bool IsEndOfFile = false;
		bool IsSuccess = true;
		unsigned have;
		unsigned char in[CHUNK];
		_fseeki64(source, 0, SEEK_SET);
		try {
			/* compress until end of file */
			do {
				have = fread(in, 1, CHUNK, source);//读取部分文件
				if (have <= 0) break;
				fwrite(in, 1, have, file_zlib);//写入文件
			} while (true);
		}
		catch (...) {
			IsSuccess = false;
		}
		fclose(source);
		in[0] = '\r';
		in[1] = '\n';
		fwrite(in, 1, 2, file_zlib);//写入文件
		source = nullptr;
		return IsSuccess;
	}

	void CloseZlibFile() {

		if (file_zlib != nullptr) {
			fclose(file_zlib);
			file_zlib = nullptr;
		}
	}
};