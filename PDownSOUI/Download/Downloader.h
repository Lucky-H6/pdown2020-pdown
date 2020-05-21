#pragma once
#include <string>
#include "../Utils/singleton.h"
#include "DownloadOneModel.h"
#include "DownloadFlow.h"	
#include "../UIAdapter/DownItem.h"
#include "../AppEvent.h"
#include <thread>
#include <atomic>
#include "vector"


using namespace std;
/*下载队列控制*/
class Downloader
{
public:
	SINGLETON_DEFINE(Downloader);
	Downloader(void) {}
	~Downloader(void) {}
private:

	/*正在下载中的线程总数*/
	std::atomic<int> DowningCount = 0;
	/*最大同时下载数*/
	int MaxCount = 3;

public:
	vector<std::shared_ptr<DownloadOneModel>> DownList = vector<std::shared_ptr<DownloadOneModel>>();
	std::mutex m;
	/*新增前判断是否已满 立即返回*/
	bool IsCanAdd() {
		return DowningCount < MaxCount;
	}
	bool IsDowning() {
		return DowningCount > 0;
	}
	/*新增，继续下载，重新下载 只有主线程操作 队列满了返回false  立即返回*/
	bool AddOne(DowningItem& Item, int waiting) {
		if (DowningCount >= MaxCount) return false;

		wstring share_fsid = Item.share_fsid;
		//std::lock_guard<std::mutex> lck(m);
		for (auto iter = DownList.begin(); iter != DownList.end(); iter++)
		{
			if ((*iter)->share_fsid == share_fsid) {
				return true;//找到了，正在下载中,直接退出(不应该出现)
			}
		}
		DowningCount++;
		std::shared_ptr<DownloadOneModel> Add = std::make_shared<DownloadOneModel>(Item);
		//new DownloadOneModel(Item);
		DownList.push_back(Add);
		//立即创建新线程，开始下载
		thread t1 = thread(DownloadFlow::RunOneDownload, Add, waiting);
		t1.detach();
		//下载线程内部不需要报告down_state( UI此时显示downing,启动中)  需要报告下载进度信息（UI改成显示下载速度等）
		return true;
	}
	/*暂停  只有主线程操作 返回是否找到 立即返回*/
	void StopOne(wstring share_fsid) {
		bool IsFind = false;
		//std::lock_guard<std::mutex> lck(m);
		for (auto iter = DownList.begin(); iter != DownList.end(); iter++)
		{
			if ((*iter)->share_fsid == share_fsid) {
				//找到了，开始暂停
				(*iter)->Stop();//下载线程会时时检测，自动退出下载结束线程	
				IsFind = true;
			}
		}
		//下载线程内部需要报告   down_state==stop(结束时,让UI显示 禁用-->已暂停 )
		if (IsFind == false) {
			wstring cmd = share_fsid + L",stop,已暂停,0,0,00:00:00";
			AppEvent::SendUI(UICmd::UIDowning_UpdateDownItemState, 0, cmd);
		}
	}

	/*完成 只有主线程操作 立即返回*/
	void FinishOne(wstring share_fsid) {
		//std::lock_guard<std::mutex> lck(m);
		for (auto iter = DownList.begin(); iter != DownList.end(); )
		{
			if ((*iter)->share_fsid == share_fsid) {
				//找到了，开始删除
				iter = DownList.erase(iter);//从下载队列中删除(使用了shared_ptr，不需要delete了)
				DowningCount--;
			}
			else iter++;
		}
	}
	/*删除TD文件 只有主线程操作 立即返回*/
	void DelOne(wstring down_dir_path) {
		DownloadFlow::DelTDFile(down_dir_path);
	}

};