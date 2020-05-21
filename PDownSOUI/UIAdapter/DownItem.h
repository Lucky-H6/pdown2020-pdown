#pragma once
#include <string>
using namespace std;
struct DowningItem
{

	wstring share_fsid;
	/*百度本地文件最后修改时间*/
	size_t local_mtime = 0;
	
	/*百度文件路径*/
	wstring path;
	/*UI 百度文件名*/
	wstring server_filename;
	
	/*百度文件体积int*/
	int64_t size = 0;
	/*UI 文件大小xxMB*/
	wstring sizestr;
	
	/*下载 加入下载队列的时间，用来排序*/
	int64_t down_time = 0;
	/*UI 下载时间2020-01-01 03:03:00 或者是 文件最后修改时间2020-01-01,当downing时是临时显示为00:00:00剩余时间*/
	wstring down_timestr;
	/*下载 文件保存目录*/
	wstring down_dir;
	/*UI 下载 进度%*/
	size_t down_prog = 0;
	
	/*UI 下载 状态 downing,waiting,stop,error,downed,disable*/
	wstring down_state;

	/*UI 临时 下载状态的说明文字*/
	wstring down_statestr = L"";

	/*临时 下载等待时间(服务器拉取中，需要等到xx时间，才能开始下载，一般是现在时间+30秒)*/
	size_t down_timewait = 0;

};

struct DownedItem
{
	
	/*百度文件ID*/
	wstring down_id;
	/*百度本地文件最后修改时间*/
	size_t local_mtime = 0;
	
	/*百度文件路径*/
	wstring path;
	/*UI 百度文件名*/
	wstring server_filename;
	
	/*百度文件体积int*/
	int64_t size = 0;
	/*UI 文件大小xxMB*/
	wstring sizestr;

	/*下载 加入下载队列的时间，用来排序*/
	int64_t down_time = 0;
	/*UI 下载时间2020-01-01 03:03:00 或者是 文件最后修改时间2020-01-01,当downing时是临时显示为00:00:00剩余时间*/
	wstring down_timestr;
	/*下载 文件保存目录*/
	wstring down_dir;
	/*下载 文件保存完整路径*/
	wstring down_file;
	
};