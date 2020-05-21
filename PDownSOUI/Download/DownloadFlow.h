#pragma once
#include "Downloader.h"
#include "DownloadOneModel.h"
#include "DownloadHttp.h"
#include "DownloadBlink.h"
#include "ServerAPID.h"
#include <thread>
#include "../Utils/OpenFileHelper.h"
#include "../Utils/PathHelper.h"
#include "../Utils/CodeCrc32.h"

#ifndef CheckNull
#define CheckNull(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
}
#define CheckNull_R(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
	return;\
}
#define CheckNull_RF(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
	return false;\
}
#endif

/*一个任务的下载流程*/
class DownloadFlow {
public:
	/*外部调用，开始完整的下载*/
	static void RunOneDownload(std::shared_ptr<DownloadOneModel> One, int waiting) {
		if (waiting > 0) this_thread::sleep_for(std::chrono::milliseconds(waiting));

		CheckNull_R(One);//找不到，已销毁 直接退出
		if (!r1_GetDownLinkList(One)) return;//读取下载地址失败
		if (!r2_CreatDownDir(One)) return;//创建文件夹失败

		if (One->downfile.fs_size <= 0) {//文件体积为0 特殊
			if (r3_1_TryCreatDownFileSize0(One)) {
				//创建成功，更新数据库和UI,退出
				rf_SaveToDBAndUI(One);
				return;
			}
			else {
				return;//创建失败(内部报告错误了)，直接退出
			}
		}

		//文件体积》0

		if (r3_2_CheckHasDowned(One)) {
			//已经下载过，更新数据库和UI,退出
			rf_SaveToDBAndUI(One);
			return;
		}

		//到这里，已经确定了，需要重新下载 开始循环下载(包含出错自动重试)
		if (!r3_3_DownFileList(One)) return;//下载失败、停止，退出
		if (!r4_SaveToFinalFile(One)) return;//保存文件失败，退出
		else {
			rf_SaveToDBAndUI(One);//保存成功，更新数据库和UI，退出
			return;
		}

	}
	/*外部调用，删除一个任务的所有临时文件*/
	static void DelTDFile(wstring down_dir_path) {

		wstring finalfile = PathHelper::PathCleanup(down_dir_path);
		if (finalfile == L"") 	return;
		wstring dirpath = L"", filename = L"";
		bool IsDir = PathHelper::GetDir(finalfile, dirpath, filename);
		if (IsDir == false || _waccess(dirpath.c_str(), 0) != 0) {
			//文件夹不存在
			return;
		}
		DownloadFile downfile = DownloadFile();
		downfile.SetDownPath(finalfile);
		for (int i = 0; i < 10; i++) {
			downfile.SetDownPathTD(i);
			_wremove(downfile.fw_filetd.c_str());
		}
		//全部删除了
	}
private:

	/*校验文件的crc32是否合法
nofile  文件不存在，success 验证成功
crcerror 文件存在&&验证失败 sizeerror 文件存在&&文件长度不一致
*/
	static wstring r23_CheckCrc32(wstring filefull, uint64_t ck_filesize, uint32_t ck_filecrc32) {
		FILE* fileptr = nullptr;
		uint64_t filesize = 0;
		bool IsOpen = PathHelper::OpenFile(filefull, fileptr, filesize);
		if (IsOpen == false) return L"nofile";

		//本地是空文件，尝试删除
		if (ck_filesize != filesize) {
			fclose(fileptr);
			fileptr = nullptr;
			if (filesize == 0) {//本地是个空文件
				if (_wremove(filefull.c_str()) == 0) {
					return L"nofile";//先尝试删除，删除了就返回nofile，重新下载。
				}
				else return L"sizeerror";//如果删除不了，那就返回crcerror，让One重命名
			}
			else return L"sizeerror";
		}
		else if (ck_filesize <= 0) {//现在ck_filesize===filesize===0
			fclose(fileptr);
			fileptr = nullptr;
			return L"success";//要下载的文件本身为0 正确
		}


		//文件存在，大小一致，计算文件crc,失败尝试删除
		CodeCrc32 crc = CodeCrc32();
		uint32_t filecrc32 = crc.getFileCrc32(fileptr);
		fclose(fileptr);
		fileptr = nullptr;

		if (ck_filecrc32 == filecrc32) {
			return L"success";
		}
		else {//文件大小一样的时候
			if (_wremove(filefull.c_str()) == 0) {
				return L"nofile";//先尝试删除，删除了就返回nofile，重新下载。
			}
			else return L"crcerror";//如果删除不了，那就返回crcerror，让One重命名
		}
	}

private:
	/*[内部报告]从服务器读取下载地址列表
	1 获取联网失败 ReportError并返回false
	2 拉取中 ReportWaiting并返回false
	3 成功，返回true*/
	static bool r1_GetDownLinkList(std::shared_ptr<DownloadOneModel> One) {
		//bdlink,fs_id,md5,path,server_filename,size
		wstring ret = ServerAPID::GetDownFileList(One);
		CheckNull_RF(One);
		if (ret != L"success") {
			One->downprog.ReportError(ret);
			return false;
		}
		else if (One->downfile.fs_state == L"waiting") {
			One->downprog.ReportWaiting();//服务器拉取中，直接退出
			return false;
		}
		else {
			//227 210 130

			int64_t total = 0;
			vector<SerDownFileItem>& DownFileList = One->downfile.DownFileList;
			for (auto iter = DownFileList.begin(); iter != DownFileList.end(); iter++)
			{
				total += (*iter).pt_size;
			}
			if (One->downfile.fs_size < total) {
				SLOGFMTE(L"Error fs_size share_fsid=%ws fs_size=%I64u total=%I64u", One->share_fsid.c_str(), One->downfile.fs_size, total);
				One->downfile.fs_size = total;//更新正确的文件大小，后面计算进度会用到
			}

			return true;//成功读取到下载地址了
		}
	}

	/*[内部报告]下载前先创建文件夹
	1 创建成功返回true，路径保存到fw_file0
	2 创建失败 ReportError并返回false*/
	static bool r2_CreatDownDir(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		wstring ff = One->downitem.down_dir + One->downitem.path;
		wstring finalfile = PathHelper::PathCleanup(ff);
		if (finalfile == L"") {
			SLOGFMTE(L"Error Save Path ff=%ws", ff.c_str());
			One->downprog.ReportError(L"下载文件保存路径错误");
			return false;
		}
		wstring dirpath = L"", filename = L"";
		bool IsDir = PathHelper::GetDir(finalfile, dirpath, filename);
		if (IsDir == false || PathHelper::CreatDir(dirpath) == false) {
			SLOGFMTE(L"Error Save Path dirpath=%ws", dirpath.c_str());
			One->downprog.ReportError(L"创建文件夹失败");
			return false;
		}
		One->downfile.SetDownPath(finalfile);
		return true;
	}

	/*[内部报告]针对文件大小为0的文件，直接创建,
	1 创建成功了，返回true，文件路径保存在downfile.fw_file
	2 创建失败，ReportError并返回false，fw_file没用*/
	static bool r3_1_TryCreatDownFileSize0(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		FILE* fileptr = nullptr;
		uint64_t filesize = 0;
		bool IsOpen = false;
		auto& downfile = One->downfile;

		for (int i = 0; i < 10; i++) {
			downfile.SetDownPath(i);
			IsOpen = PathHelper::OpenFileAutoCreat(downfile.fw_file, fileptr, filesize);
			if (IsOpen) {
				fclose(fileptr);
				fileptr = nullptr;
				if (filesize == 0) return true;
			}
		}
		One->downprog.ReportError(L"创建文件失败0");
		return false;
	}

	/*[不需要报告]检查之前是否已下载过
	1 找到了，返回true,文件路径保存在downfile.fw_file
	2 找不到，返回false,fw_file没用*/
	static bool r3_2_CheckHasDowned(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		auto& downfile = One->downfile;

		for (int i = 0; i < 10; i++) {
			downfile.SetDownPath(i);
			wstring IsCrc = r23_CheckCrc32(downfile.fw_file, downfile.fs_size, downfile.fs_crc32);
			if (IsCrc == L"success") return true;//之前已经下载过，直接返回true			
			else if (IsCrc == L"crcerror" || IsCrc == L"sizeerror") continue;//只要不是成功就继续检查
			else if (IsCrc == L"nofile") continue;//只要不是成功就继续检查
		}
		return false;
	}


	/*[内部报告]循环下载全部的分片
	1 td打开失败 ReportError并返回false
	2 StopDownload ReportStop并返回false
	3 多次尝试后下载失败 ReportError并返回false
	4 所有分片都下载成功 ReportDowning并返回true*/
	static bool r3_3_DownFileList(std::shared_ptr<DownloadOneModel> One) {
		if (!r3_3_1_TryOpenTDFile(One)) {
			One->downprog.ReportError(L"读写临时文件失败");
			return false;//打开TD文件失败
		}

		vector<SerDownFileItem>& DownFileList = One->downfile.DownFileList;
		int parti = 0;
		//按顺序循环下载所有分片
		for (auto iter = DownFileList.begin(); iter != DownFileList.end(); iter++)
		{
			bool IsDown = false;
			parti++;
			for (int i = 0; i < 10; i++) {
				CheckNull_RF(One);
				//SLOGFMTI("r2_DownFileList parti=%d for=%d %ws", parti, i, One->share_fsid.c_str());
				if (r3_3_2_DownAndCheckOneFile(One, parti, *(iter))) {
					//SLOGFMTI("r2_DownFileList 下载成功 parti=%d i=%d", parti, i);
					IsDown = true;//成功下载
					break;
				}
				//下载失败或者用户主动要求停止
				//SLOGFMTI("r2_DownFileList 下载失败 parti=%d i=%d", parti, i);
				CheckNull_RF(One);
				CheckNull_RF(One->downresp);
				if (One->downresp->StopDownload) {
					One->downprog.ReportStop();
					return false;//如果是用户主动要求停止，直接退出
				}
			}
			if (IsDown == false) {
				CheckNull_RF(One);
				One->downprog.ReportError(L"下载失败，请重试");
				return false;//多次尝试仍然错误,直接退出，不继续下载后面的文件
			}
			CheckNull_RF(One);
		}
		//到这里就说明，所有分片都成功下载了
		CheckNull_RF(One);
		One->downprog.ReportDowning(L"0B/s", One->downfile.fs_size, One->downfile.fs_size);
		One->downprog.ReportDowning(L"保存文件中");
		return true;
	}

	/*[不需要报告]尝试打开TD文件，开始写入
	1 成功打开、创建，返回true，文件路径保存到downfile.fw_filetd,文件句柄One->downresp->fw_tdptr
	2 打开失败，返回false*/
	static bool r3_3_1_TryOpenTDFile(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		auto& downfile = One->downfile;
		FILE* tdfileptr = nullptr;
		uint64_t tdfilesize = 0;
		bool IsOpen = false;
		for (int i = 0; i < 10; i++) {
			downfile.SetDownPathTD(i);
			IsOpen = PathHelper::OpenFileAutoCreat(downfile.fw_filetd, tdfileptr, tdfilesize);
			if (IsOpen) break;//成功打开了,直接退出循环
			else {
				//td文件被占用，改名重试
				SLOGFMTE(L"WriteToFileError=%ws", downfile.fw_filetd.c_str());
			}
		}
		if (IsOpen) {
			One->downresp->SetFilePtr(tdfileptr, tdfilesize, One->downfile.fs_size);
			//OpenFileHelper::OpenDir(L"", downfile.fw_filetd); //调试时自动打开下载目录
			return true;
		}
		else {
			return false;
		}
	}

	/*[内部报告]下载和校验 一个文件  报告进度
	1 这个分片已经下载过(crc校验通过) ReportDowning并返回true
	2 分片下载结束 ReportDowning并返回true/false (crc校验结果)
	3 网络出错导致退出 ReportDowning并返回false
	4 用户主动退出 ReportDowning并返回false
	只有这个特殊，不论true/false 都ReportDowning*/
	static bool r3_3_2_DownAndCheckOneFile(std::shared_ptr<DownloadOneModel> One, int index, SerDownFileItem DownFile) {
		CheckNull_RF(One);
		DownloadResp* downresp = One->downresp;
		CheckNull_RF(downresp);
		int64_t pt_start = 0;
		//校验td文件是否已下载这个分片
		if (downresp->fw_tdsize >= (DownFile.pt_pos + DownFile.pt_size)) {
			//已经下载过了，校验crc
			CodeCrc32 crc = CodeCrc32();
			uint32_t filecrc32 = crc.getFileCrc32(downresp->fw_tdptr, DownFile.pt_pos, DownFile.pt_size);
			if (filecrc32 == DownFile.pt_crc32) {
				//一致，不需要下载了,直接退出接着下载下一段
				//SLOGFMTI("DownedBefor&&CheckCRC32pass filecrc32=%I32u", filecrc32);
				pt_start = DownFile.pt_size;
				One->downprog.ReportDowning(L"0B/s", DownFile.pt_pos + DownFile.pt_size, One->downfile.fs_size);
				return true;
			}
			else {
				//不一致，从头重新下载
				//SLOGFMTI("DownedBeforButCheckCRC32Error=%I32u pt_crc32=%I32u ", filecrc32, DownFile.pt_crc32);
				pt_start = 0;
				One->downprog.ReportDowning(L"0B/s", DownFile.pt_pos, One->downfile.fs_size);
			}
		}
		else if (downresp->fw_tdsize > DownFile.pt_pos) {//pt_pos << fw_tdsize << pt_pos + pt_size  说明需要断点续传
			pt_start = downresp->fw_tdsize - DownFile.pt_pos;//续传
			One->downprog.ReportDowning(L"0B/s", DownFile.pt_pos + pt_start, One->downfile.fs_size);
			//SLOGFMTI("RangeDownload pt_start=%I64d pt_pos=%I64d ", pt_start, DownFile.pt_pos);
		}

		//到这里已经明确了，需要下载pt_start----(pt_pos + pt_size)这一段(Rang:pt_start-)就可以了

		bool IsDownSuccess = false;
		for (size_t i = 0; i < DownFile.downurls.size(); i++) {
			auto& downurl = DownFile.downurls[i];
			if (downurl.jsoncmd == L"downurl") {
				CheckNull(One);
				CheckNull_RF(downresp);
				//SLOGFMTI("StartToDownurls d_url=%ls pt_start=%I64d pt_pos=%I64d pt_size=%I64d ", downurl.d_url.c_str(), DownFile.pt_pos, pt_start, DownFile.pt_size);
				downresp->SetDownPos(DownFile.pt_pos, pt_start, DownFile.pt_size);
				IsDownSuccess = DownloadHttp::DownFile(downurl.d_method, downurl.d_url, downurl.d_header, downresp);
			}
			else if (downurl.jsoncmd == L"http") {
				AppThread::BlinkPool.enqueue([](wstring url, wstring useragent) {
					DownloadBlink::RunHttpCommand(url, useragent);
				}, downurl.d_url, downurl.d_header);
			}
		}
		//SLOGFMTI("FinishDown index=%d IsDownSuccess=%ws", index, IsDownSuccess ? L"true" : L"false");
		CheckNull_RF(One);
		//下载结束
		if (IsDownSuccess) {//下载成功，校验crc
			One->downprog.ReportDowning(L"CheckPartCRC " + to_wstring(index));
			CodeCrc32 crc = CodeCrc32();
			uint32_t filecrc32 = crc.getFileCrc32(downresp->fw_tdptr, DownFile.pt_pos, DownFile.pt_size);
			bool IsCrc32 = (filecrc32 == DownFile.pt_crc32);//返回是否成功
			if (IsCrc32) {
				//SLOGFMTI("PartCrcOK,RunNextPart %ws", L"");
				//One->downprog.ReportDowning(L"等待网络");
			}
			else {
				//SLOGFMTI("PartCrcError,Redownload filecrc32=%I32d pt_crc32=%I32d", filecrc32, DownFile.pt_crc32);
				downresp->fw_tdsize = DownFile.pt_pos;
				One->downprog.ReportDowning(L"校验失败准备重下");
			}
			return IsCrc32;
		}
		else if (One->downresp->StopDownload) {
			//SLOGFMTI("下载结束 用户主动取消 %d", index);
			//用户主动取消&&没有下载结束
			return false;
		}
		else {
			//纯粹是网络错误导致下载中断退出
			wstring msg = One->downresp->errormsg;
			//SLOGFMTI("DownFinaly Error index=%d msg=%ws", index, msg.c_str());
			One->downprog.ReportDowning(msg == L"" ? L"网路错误" : msg);//分片下载中发生的网络错误，并能上报为错误时间，会导致One销毁
			//One->downprog.ReportError(One->downresp->errormsg);
		}
		return false;//下载失败，会立即重新尝试
	}

	/*[内部报告]全部下载结束，保存到最终的文件
	1 解密失败 ReportError并返回false
	2 校验失败 ReportError并返回false
	3 重命名失败 ReportError并返回false
	4 保存成功返回true*/
	static bool r4_SaveToFinalFile(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		//1 解密
		if (!r4_1_DecodeFile(One)) {
			//删除文件
			One->downresp->CloseFilePtr();
			_wremove(One->downfile.fw_filetd.c_str());
			One->downprog.ReportError(L"保存文件读写失败");
			return false;
		}
		//2 校验
		CodeCrc32 crc = CodeCrc32();
		uint32_t filecrc32 = crc.getFileCrc32(One->downresp->fw_tdptr);
		if (filecrc32 != One->downfile.fs_crc32) {
			//校验失败，删除文件
			One->downresp->CloseFilePtr();
			_wremove(One->downfile.fw_filetd.c_str());
			One->downprog.ReportError(L"文件校验失败");
			return false;
		}

		//3  重命名td-->final
		if (!r4_3_TrySaveFile(One)) {
			One->downprog.ReportError(L"保存文件失败");
			return false;
		}

		return true;
	}
	/*[不需要报告]解密
	1 文件读写失败 返回false
	2 解密成功 返回true*/
	static bool r4_1_DecodeFile(std::shared_ptr<DownloadOneModel> One) {
		
		return true;//解密成功
	}
	/*[不需要报告]把td文件保存为最终文件
	1 保存成功 返回true
	2 保存失败 返回false*/
	static bool r4_3_TrySaveFile(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		auto& downfile = One->downfile;
		One->downresp->CloseFilePtr();

		for (int i = 0; i < 15; i++) {
			downfile.SetDownPath(i);
			wstring IsCrc = r23_CheckCrc32(downfile.fw_file, downfile.fs_size, downfile.fs_crc32);
			if (IsCrc == L"success") {
				//已经有了，直接删除td文件
				_wremove(downfile.fw_filetd.c_str());
				return true;
			}
			else if (IsCrc == L"nofile") {
				//移动td文件,成功就直接退出，移动失败改名重试
				if (0 == _wrename(downfile.fw_filetd.c_str(), downfile.fw_file.c_str())) return true;
			}
			else if (IsCrc == L"crcerror" || IsCrc == L"sizeerror") continue;//只要不是成功就改名继续检查
		}
		return false;
	}
	/*从Downing删除，保存到Downed，刷新UI*/
	static void rf_SaveToDBAndUI(std::shared_ptr<DownloadOneModel> One) {
		DownedItem downed = DownedItem();
		int64_t down_time = StringHelper::GetTimeNowWeiMiao();
		auto& downitem = One->downitem;
		downed.down_id = downitem.share_fsid + L"_" + to_wstring(down_time);
		downed.local_mtime = downitem.local_mtime;
		downed.path = downitem.path;
		downed.server_filename = downitem.server_filename;
		downed.size = One->downfile.fs_size;
		downed.sizestr = StringHelper::FormatFileSize(downed.size);
		wstring down_timestr = S2W::StringtoWString(StringHelper::FormatJsTimeStr(StringHelper::GetTimeNow(), false), CP_UTF8);

		downed.down_time = down_time;
		downed.down_timestr = down_timestr;
		downed.down_dir = downitem.down_dir;
		downed.down_file = One->downfile.fw_file;
		One->downprog.ReportDowned();

		SLOGFMTE(L"down success %ws %ws", downitem.share_fsid.c_str(), downed.down_file.c_str());
		//保存到数据库
		AppThread::DBPool.enqueue([](wstring share_fsid, DownedItem downed) {
			DBHelper::GetI()->AddDowned(downed);
			DBHelper::GetI()->DelDowning(share_fsid);
			AppEvent::SendUI(UICmd::UIDowned_AddDownItem, 0, downed.down_id);
			}, downitem.share_fsid, downed);
		AppEvent::SendUI(UICmd::UIDowning_DeleteDownItem, 0, downitem.share_fsid);
	}

};