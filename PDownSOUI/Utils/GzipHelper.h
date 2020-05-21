#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "windows.h"
#include "../third-part/zlib/zlib.h"


using namespace std;
class GzipHelper
{
public:
	GzipHelper() {

	}
	~GzipHelper() {

	}

	/*gzip压缩文件。读取文件数据->gzip->返回数据  失败返回NULL*/
	static unsigned char* GzipFileData(wstring filetoread, size_t& gzlen) {
		if (_waccess(filetoread.c_str(), 0) != 0) {//文件不存在
			return nullptr;
		}

		ifstream fin; size_t errComp, file_len_src;
		unsigned long len_tmp;
		unsigned char* buffer_src = nullptr;
		unsigned char* buffer_tmp = nullptr;
		try {
			fin.open(filetoread, ios_base::in | ios_base::binary);

			fin.seekg(0, ios::end);
			streampos pend = fin.tellg();
			fin.seekg(0, ios::beg);
			streampos pbegin = fin.tellg();
			file_len_src = (size_t)(pend - pbegin);

			if (file_len_src > 10240)
			{
				if (file_len_src > 1024 * 1024 * 20) file_len_src = 1024 * 1024 * 20;
				buffer_src = new unsigned char[file_len_src];
				len_tmp = compressBound(file_len_src);
				buffer_tmp = new unsigned char[len_tmp];

				fin.seekg(0, ios::beg);
				fin.read((char*)buffer_src, file_len_src);//读取文件内容
				fin.close();
				errComp = gzcompress(buffer_src, file_len_src, buffer_tmp, &len_tmp);//压缩
				delete[] buffer_src;
				buffer_src = nullptr;
				if (errComp == 0)
				{
					gzlen = len_tmp;
					return buffer_tmp;
				}
			}
			else {
				fin.close();
			}
		}
		catch (...) {
			if (nullptr == buffer_src) delete[] buffer_src;
			if (nullptr == buffer_tmp) delete[] buffer_tmp;
		}
		gzlen = 0;
		return nullptr;
	}
	inline static string U2A(const wstring& str)
	{
		string strDes;
		if (str.empty())
			goto __end;
		int nLen = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
		if (0 == nLen)
			goto __end;
		char* pBuffer = new char[nLen + 1];
		memset(pBuffer, 0, nLen + 1);
		::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen, NULL, NULL);
		pBuffer[nLen] = '\0';
		strDes.append(pBuffer);
		delete[] pBuffer;
	__end:
		return strDes;
	}
	static string GzipString(wstring str) {
		string strRet = "";
		size_t errComp, file_len_src;
		unsigned char* buffer_tmp = NULL;
		unsigned long len_tmp;
		try {
			string u8 = U2A(str);
			file_len_src = u8.length();
			unsigned char* buffer_src = (unsigned char*)u8.c_str();
			len_tmp = compressBound(file_len_src);
			buffer_tmp = new unsigned char[len_tmp];
			errComp = gzcompress(buffer_src, file_len_src, buffer_tmp, &len_tmp);//压缩
			buffer_src = NULL;
			if (errComp == 0)
			{
				strRet.append((const char*)buffer_tmp, len_tmp);
			}
			delete[] buffer_tmp;
			buffer_tmp = NULL;
		}
		catch (...) {
			strRet = "";
			if (buffer_tmp) delete[] buffer_tmp;
			buffer_tmp = NULL;
		}
		return strRet;
	}

	/*解压缩HTTP数据 gzip decompress */
	static int UnGzipHttpData(Byte* zdata, uLong nzdata, Byte** data, uLong* ndata)
	{
		if (zdata == NULL) return 0;
		Byte b4 = zdata[nzdata - 4];
		Byte b3 = zdata[nzdata - 3];
		Byte b2 = zdata[nzdata - 2];
		Byte b1 = zdata[nzdata - 1];

		//uLong data32 = ((uLong)b4 << 24) | ((uLong)b3 << 16) | ((uLong)b2 << 8) | ((uLong)b1);

		uLong gao32 = ((uLong)b1 << 24) | ((uLong)b2 << 16) | ((uLong)b3 << 8) | ((uLong)b4);
		*ndata = gao32;
		Byte* undata = new Byte[gao32];

		int err = 0;
		z_stream d_stream = { 0 }; /* decompression stream */
		static char dummy_head[2] =
		{
			0x8 + 0x7 * 0x10,
			(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
		};
		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;
		d_stream.next_in = zdata;
		d_stream.avail_in = 0;
		d_stream.next_out = undata;
		//if (inflateInit2(&d_stream, -MAX_WBITS) != Z_OK) return -1;
		if (inflateInit2(&d_stream, 47) != Z_OK) return -1;
		while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
			d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
			if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
			if (err != Z_OK)
			{
				if (err == Z_DATA_ERROR)
				{
					d_stream.next_in = (Bytef*)dummy_head;
					d_stream.avail_in = sizeof(dummy_head);
					if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
					{
						return -1;
					}
				}
				else return -1;
			}
		}
		if (inflateEnd(&d_stream) != Z_OK) return -1;
		*ndata = d_stream.total_out;
		*data = undata;

		return 0;
	}
private:
	static int gzcompress(Bytef* data, uLong ndata, Bytef* zdata, uLong* nzdata)
	{

		z_stream c_stream;
		int err = 0;
		if (data && ndata > 0) {

			c_stream.zalloc = NULL;
			c_stream.zfree = NULL;
			c_stream.opaque = NULL;
			//只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
			if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
				MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) return-1;
			c_stream.next_in = data;
			c_stream.avail_in = ndata;
			c_stream.next_out = zdata;
			c_stream.avail_out = *nzdata;
			while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata) {
				if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
			}
			if (c_stream.avail_in != 0) return c_stream.avail_in;

			for (;;) {
				if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
				if (err != Z_OK) return -1;
			}
			if (deflateEnd(&c_stream) != Z_OK) return -1;
			*nzdata = c_stream.total_out;
			return 0;
		}
		return-1;

	}



};