#pragma once
#include <string>
#include <vector>
using namespace std;

/*一个分片的下载地址(jsoncmd 可以包含更多特殊操作)*/
struct SerDownFileDownUrl {
public:
	//命令类型 downurl
	wstring jsoncmd;
	//下载请求的url
	wstring d_url;
	//下载请求的header
	wstring d_header;
	//下载请求的method
	wstring d_method;

	SerDownFileDownUrl() {}
	SerDownFileDownUrl(wstring jsoncmd, wstring d_method, wstring d_url, wstring d_header)
		:jsoncmd(std::move(jsoncmd)), d_url(std::move(d_url)), d_header(std::move(d_header)), d_method(std::move(d_method)) {
	}
};
/*一个分片的文件信息（包含下载地址）*/
struct SerDownFileItem
{
public:
	//分片序号
	int pt_index;
	//分片位置
	int64_t pt_pos;
	//分片体积（因为分片不会太大，int就可以了）
	int64_t pt_size;
	//分片的crc32
	uint32_t pt_crc32;

	vector<SerDownFileDownUrl> downurls;

	SerDownFileItem() {}
	SerDownFileItem(int pt_index, int64_t pt_size, uint32_t pt_crc32, vector<SerDownFileDownUrl>& downurls)
		: pt_index(pt_index), pt_size(pt_size), pt_crc32(pt_crc32), downurls(downurls)
	{
	}
};