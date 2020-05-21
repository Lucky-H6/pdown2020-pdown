#pragma once
#include "../stdafx.h"
#include "../Utils/singleton.h"
#include "vector"
#include "sqlite3.h"
#include "../utils/StringHelper.h"
#include "../Utils/PathHelper.h"
#include "../UIAdapter/DownItem.h"



class DBHelper
{
public:
	SINGLETON_DEFINE(DBHelper);
	DBHelper(void) {
		OpenDBFile();
	}
	~DBHelper(void) {
		CloseDBFile();
	}
private:
	static int BusyHandler(void* ptr, int retry_times) {
		sqlite3_sleep(10);
		// 如果返回零则不会继续等待，则外部的执行返回SQLITE_BUSY。
		// 如果返回非零则继续循环，等待其他应用释放DB锁。
		return 1;
	}
	std::wstring _dbfile = L"";
	std::wstring _config_userdata = L"";
public:
	sqlite3* db_ = nullptr;
	/*程序启动时，初始化数据库*/
	bool OpenDBInit(wstring dbfile) {
		_dbfile = dbfile;
		sqlite3_config(SQLITE_CONFIG_SERIALIZED);
		int result = sqlite3_open16(dbfile.c_str(), &db_);

		if (result == SQLITE_OK)
		{
			std::string creatsql = "CREATE TABLE IF NOT EXISTS AppConfig(key TEXT PRIMARY KEY, value TEXT);";
			creatsql += "CREATE TABLE IF NOT EXISTS DownInfo_Downing(share_fsid TEXT PRIMARY KEY, down_time BIGINT, down_timestr TEXT, down_dir TEXT, down_prog INTEGER, down_state TEXT, local_mtime INTEGER,path TEXT,server_filename TEXT,size BIGINT,sizestr TEXT);";
			creatsql += "CREATE TABLE IF NOT EXISTS DownInfo_Downed(    down_id TEXT PRIMARY KEY, down_time BIGINT, down_timestr TEXT, down_dir TEXT, down_file TEXT, local_mtime INTEGER,path TEXT,server_filename TEXT,size BIGINT,sizestr TEXT);";
			int dbresult = sqlite3_exec(db_, creatsql.c_str(), nullptr, 0, nullptr);
			return (dbresult == SQLITE_OK);

		}
		return false;
	}

	bool OpenDBFile()
	{
		if (db_)	return true;
		sqlite3_config(SQLITE_CONFIG_SERIALIZED);
		int result = sqlite3_open16(_dbfile.c_str(), &db_);
		return (result == SQLITE_OK);
	}
	void CloseDBFile() {
		if (db_)
		{
			sqlite3_close_v2(db_);
			db_ = nullptr;
		}
	}

	bool SaveConfigData(const std::string key, const std::wstring value)
	{
		if (key.length() == 0) return true;
		//ZLOGFI("InsertConfigDataTask: key: %s, value: %s, ThreadID: %d", key.c_str(), value.c_str(), nbase::Thread::CurrentId());
		if (OpenDBFile()) {
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			std::string sql = "INSERT OR REPLACE into AppConfig (key, value) values (?, ?)";
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC); //绑定数据
			sqlite3_bind_text16(stmt, 2, value.c_str(), -1, SQLITE_STATIC); //绑定数据
			int r = sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			if (r == SQLITE_DONE) {
				return true;
			}
			SLOGFMTE("error: InsertConfigData for key: %s, reason: %d", key.c_str(), result);
		}
		return false;
	}

	std::wstring GetConfigData(const std::string& key)
	{
		if (key.length() == 0) return L"";
		std::wstring value = L"";
		if (OpenDBFile()) {
			std::string sql = "SELECT value FROM AppConfig WHERE key='" + key + "'";//SQL语句		
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			if (result == SQLITE_OK) {
				// 每调一次sqlite3_step()函数，stmt语句句柄就会指向下一条记录
				try {
					while (sqlite3_step(stmt) == SQLITE_ROW) {
						wchar_t* Data = (wchar_t*)sqlite3_column_text16(stmt, 0);
						if (nullptr == Data) Data = L"";
						value = std::wstring(Data);
						break;
					}
				}
				catch (...) {
					value = L"";
				}
			}
			//清理语句句柄，准备执行下一个语句
			sqlite3_finalize(stmt);
		}
		//SLOGFMTI(L"GetConfigData: key: %hs, value: %s", key.c_str(), value.c_str());
		return value;
	}

	std::wstring GetUserData() {
		if (_config_userdata == L"") _config_userdata = GetConfigData("UserData");
		return _config_userdata;
	}
	void SaveUserData(std::wstring value) {
		_config_userdata = value;
		SaveConfigData("UserData", value);
	}

	void ClearConfig()
	{
		if (OpenDBFile()) {
			std::string sql = "delete from AppConfig";//SQL语句		
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			sqlite3_exec(db_, sql.c_str(), nullptr, 0, nullptr);
		}
	}

	bool NoUseAddDowning(DowningItem& model) {
		if (model.share_fsid.length() == 0) return false;
		if (OpenDBFile()) {
			std::string sql = "INSERT OR REPLACE into DownInfo_Downing values(?,?,?,?,?,?,?,?,?,?,?)";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

			sqlite3_bind_text16(stmt, 1, model.share_fsid.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int64(stmt, 2, model.down_time);
			sqlite3_bind_text16(stmt, 3, model.down_timestr.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text16(stmt, 4, model.down_dir.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 5, model.down_prog);
			sqlite3_bind_text16(stmt, 6, model.down_state.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 7, model.local_mtime);
			sqlite3_bind_text16(stmt, 8, model.path.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text16(stmt, 9, model.server_filename.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int64(stmt, 10, model.size);
			sqlite3_bind_text16(stmt, 11, model.sizestr.c_str(), -1, SQLITE_STATIC);

			int r = sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return (r == SQLITE_DONE);
		}
		return false;
	}
	bool AddDowningList(std::vector<DowningItem>& List) {
		bool iserror = false;
		if (OpenDBFile()) {
			std::string sql = "INSERT OR REPLACE into DownInfo_Downing values(?,?,?,?,?,?,?,?,?,?,?)";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			sqlite3_exec(db_, "begin;", 0, 0, 0);
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			for (auto iter = List.begin(); iter != List.end(); iter++)
			{
				DowningItem& model = (*iter);
				if (model.share_fsid.length() == 0) continue;

				sqlite3_bind_text16(stmt, 1, model.share_fsid.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_int64(stmt, 2, model.down_time);
				sqlite3_bind_text16(stmt, 3, model.down_timestr.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_text16(stmt, 4, model.down_dir.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_int(stmt, 5, model.down_prog);
				sqlite3_bind_text16(stmt, 6, model.down_state.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_int(stmt, 7, model.local_mtime);
				sqlite3_bind_text16(stmt, 8, model.path.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_text16(stmt, 9, model.server_filename.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_int64(stmt, 10, model.size);
				sqlite3_bind_text16(stmt, 11, model.sizestr.c_str(), -1, SQLITE_STATIC);

				int r = sqlite3_step(stmt);
				if (r != SQLITE_DONE) { iserror = true; }
				sqlite3_reset(stmt);
			}
			sqlite3_finalize(stmt);
			sqlite3_exec(db_, "commit;", 0, 0, 0);
		}

		CloseDBFile();//为了释放内存
		OpenDBFile();

		return iserror;
	}
	/*更新下载进度waiting，或者下载状态stop*/
	bool UpdateDowning(std::wstring& share_fsid, int down_prog, wstring down_state) {
		if (share_fsid.length() == 0) return true;
		if (down_state != L"waiting" && down_state != L"stop") return false;
		if (OpenDBFile()) {
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			if (down_state == L"waiting") {
				std::string sql = "update DownInfo_Downing set down_prog=?,down_state=? where share_fsid=?";
				int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
				sqlite3_bind_int(stmt, 1, down_prog);
				sqlite3_bind_text16(stmt, 2, down_state.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_text16(stmt, 3, share_fsid.c_str(), -1, SQLITE_STATIC);
			}
			else if (down_state == L"stop") {
				std::string sql = "update DownInfo_Downing set down_state=? where share_fsid=?";
				int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
				sqlite3_bind_text16(stmt, 1, down_state.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_text16(stmt, 2, share_fsid.c_str(), -1, SQLITE_STATIC);
			}
			int r = sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return (r == SQLITE_DONE);
		}
		return false;
	}

	bool UpdateDowning(vector<wstring>& List, wstring down_state) {
		if (down_state != L"waiting" && down_state != L"stop") return false;

		bool iserror = false;
		if (OpenDBFile()) {
			std::string sql = "update DownInfo_Downing set down_state=? where share_fsid=?";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			sqlite3_exec(db_, "begin;", 0, 0, 0);
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			for (auto iter = List.begin(); iter != List.end(); iter++)
			{
				wstring& share_fsid = (*iter);
				if (share_fsid.length() == 0) continue;
				sqlite3_bind_text16(stmt, 1, down_state.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_text16(stmt, 2, share_fsid.c_str(), -1, SQLITE_STATIC);
				int r = sqlite3_step(stmt);
				if (r != SQLITE_DONE) { iserror = true; }
				sqlite3_reset(stmt);
			}
			sqlite3_finalize(stmt);
			sqlite3_exec(db_, "commit;", 0, 0, 0);
		}
		return iserror;
	}

	bool DelDowning(wstring share_fsid) {
		if (share_fsid.length() == 0) return true;
		if (OpenDBFile()) {
			std::string sql = "delete from DownInfo_Downing where share_fsid=?";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			sqlite3_bind_text16(stmt, 1, share_fsid.c_str(), -1, SQLITE_STATIC);
			int r = sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return (r == SQLITE_DONE);
		}
		return false;
	}
	bool DelDowning(vector<wstring>& List) {
		bool iserror = false;
		if (OpenDBFile()) {
			std::string sql = "delete from DownInfo_Downing where share_fsid=?";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			sqlite3_exec(db_, "begin;", 0, 0, 0);
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			for (auto iter = List.begin(); iter != List.end(); iter++)
			{
				wstring& share_fsid = (*iter);
				if (share_fsid.length() == 0) continue;
				sqlite3_bind_text16(stmt, 1, share_fsid.c_str(), -1, SQLITE_STATIC);
				int r = sqlite3_step(stmt);
				if (r != SQLITE_DONE) { iserror = true; }
				sqlite3_reset(stmt);
			}
			sqlite3_finalize(stmt);
			sqlite3_exec(db_, "commit;", 0, 0, 0);
		}
		CloseDBFile();//为了释放内存
		OpenDBFile();
		return iserror;

	}

	void GetDowningList(std::vector<DowningItem>& List) {
		List.clear();
		List.reserve(2000);

		if (OpenDBFile()) {
			std::string sql = "SELECT * FROM DownInfo_Downing order by down_time asc";//SQL语句		
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			if (result == SQLITE_OK) {
				try {
					while (sqlite3_step(stmt) == SQLITE_ROW) {
						DowningItem model;

						wchar_t* Data = (wchar_t*)sqlite3_column_text16(stmt, 0);
						model.share_fsid = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.down_time = sqlite3_column_int64(stmt, 1);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 2);
						model.down_timestr = ((nullptr == Data) ? L"" : std::wstring(Data));
						Data = (wchar_t*)sqlite3_column_text16(stmt, 3);
						model.down_dir = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.down_prog = sqlite3_column_int(stmt, 4);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 5);
						model.down_state = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.local_mtime = sqlite3_column_int(stmt, 6);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 7);
						model.path = ((nullptr == Data) ? L"" : std::wstring(Data));
						Data = (wchar_t*)sqlite3_column_text16(stmt, 8);
						model.server_filename = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.size = sqlite3_column_int64(stmt, 9);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 10);
						model.sizestr = ((nullptr == Data) ? L"" : std::wstring(Data));

						Data = nullptr;

						if (model.down_state == L"downing" || model.down_state == L"downed") model.down_state = L"waiting";//不可能发生
						List.emplace_back(model);
					}
				}
				catch (...) {}
			}
			else {
				SLOGFMTE("sql=%s", sql.c_str());
			}
			//清理语句句柄，准备执行下一个语句
			sqlite3_finalize(stmt);
		}
		CloseDBFile();//为了释放内存
		OpenDBFile();
		List.shrink_to_fit();
	}
	/*保存到Downed，并从Downing删除*/
	bool AddDowned(DownedItem& model) {
		if (model.down_id.length() == 0) return true;
		if (OpenDBFile()) {
			std::string sql = "INSERT OR REPLACE into DownInfo_Downed values(?,?,?,?,?,?,?,?,?,?)";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

			sqlite3_bind_text16(stmt, 1, model.down_id.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int64(stmt, 2, model.down_time);
			sqlite3_bind_text16(stmt, 3, model.down_timestr.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text16(stmt, 4, model.down_dir.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text16(stmt, 5, model.down_file.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 6, model.local_mtime);
			sqlite3_bind_text16(stmt, 7, model.path.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text16(stmt, 8, model.server_filename.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int64(stmt, 9, model.size);
			sqlite3_bind_text16(stmt, 10, model.sizestr.c_str(), -1, SQLITE_STATIC);

			int r = sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return (r == SQLITE_DONE);
		}
		CloseDBFile();//为了释放内存
		OpenDBFile();
		return false;
	}

	bool GetDowned(wstring down_id, DownedItem& model) {
		if (down_id.length() == 0) return false;
		bool IsGet = false;
		if (OpenDBFile()) {
			std::string sql = "SELECT * FROM DownInfo_Downed  where down_id=?";//SQL语句		
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			sqlite3_bind_text16(stmt, 1, down_id.c_str(), -1, SQLITE_STATIC);
			int row = sqlite3_step(stmt);
			if (row == SQLITE_ROW) {
				try {
					wchar_t* Data = (wchar_t*)sqlite3_column_text16(stmt, 0);
					model.down_id = ((nullptr == Data) ? L"" : std::wstring(Data));
					model.down_time = sqlite3_column_int64(stmt, 1);
					Data = (wchar_t*)sqlite3_column_text16(stmt, 2);
					model.down_timestr = ((nullptr == Data) ? L"" : std::wstring(Data));
					Data = (wchar_t*)sqlite3_column_text16(stmt, 3);
					model.down_dir = ((nullptr == Data) ? L"" : std::wstring(Data));
					Data = (wchar_t*)sqlite3_column_text16(stmt, 4);
					model.down_file = ((nullptr == Data) ? L"" : std::wstring(Data));
					model.local_mtime = sqlite3_column_int(stmt, 5);
					Data = (wchar_t*)sqlite3_column_text16(stmt, 6);
					model.path = ((nullptr == Data) ? L"" : std::wstring(Data));
					Data = (wchar_t*)sqlite3_column_text16(stmt, 7);
					model.server_filename = ((nullptr == Data) ? L"" : std::wstring(Data));
					model.size = sqlite3_column_int64(stmt, 8);
					Data = (wchar_t*)sqlite3_column_text16(stmt, 9);
					model.sizestr = ((nullptr == Data) ? L"" : std::wstring(Data));

					Data = nullptr;
					IsGet = true;
				}
				catch (...) {}
			}
			else {
				SLOGFMTE("sql=%s", sql.c_str());
			}
			//清理语句句柄，准备执行下一个语句
			sqlite3_finalize(stmt);
		}
		return IsGet;
	}

	void GetDownedList(std::vector<DownedItem>& List) {
		List.clear();
		List.reserve(2000);

		if (OpenDBFile()) {
			std::string sql = "SELECT * FROM DownInfo_Downed order by down_time asc";//SQL语句		
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			if (result == SQLITE_OK) {
				try {
					while (sqlite3_step(stmt) == SQLITE_ROW) {
						DownedItem model;

						wchar_t* Data = (wchar_t*)sqlite3_column_text16(stmt, 0);
						model.down_id = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.down_time = sqlite3_column_int64(stmt, 1);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 2);
						model.down_timestr = ((nullptr == Data) ? L"" : std::wstring(Data));
						Data = (wchar_t*)sqlite3_column_text16(stmt, 3);
						model.down_dir = ((nullptr == Data) ? L"" : std::wstring(Data));
						Data = (wchar_t*)sqlite3_column_text16(stmt, 4);
						model.down_file = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.local_mtime = sqlite3_column_int(stmt, 5);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 6);
						model.path = ((nullptr == Data) ? L"" : std::wstring(Data));
						Data = (wchar_t*)sqlite3_column_text16(stmt, 7);
						model.server_filename = ((nullptr == Data) ? L"" : std::wstring(Data));
						model.size = sqlite3_column_int64(stmt, 8);
						Data = (wchar_t*)sqlite3_column_text16(stmt, 9);
						model.sizestr = ((nullptr == Data) ? L"" : std::wstring(Data));

						Data = nullptr;

						List.emplace_back(model);
					}
				}
				catch (...) {}
			}
			else {
				SLOGFMTE("sql=%s", sql.c_str());
			}
			//清理语句句柄，准备执行下一个语句
			sqlite3_finalize(stmt);
		}
		CloseDBFile();//为了释放内存
		OpenDBFile();
		List.shrink_to_fit();
	}

	bool DelDowned(wstring down_id) {
		if (down_id.length() == 0) return true;
		if (OpenDBFile()) {
			std::string sql = "delete from DownInfo_Downed where down_id=?";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
			sqlite3_bind_text16(stmt, 1, down_id.c_str(), -1, SQLITE_STATIC);
			int r = sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			return (r == SQLITE_DONE);
		}
		return false;
	}
	bool DelDownedAll() {
		if (OpenDBFile()) {
			std::string sql = "delete from DownInfo_Downed";
			sqlite3_stmt* stmt = nullptr;// stmt语句句柄
			sqlite3_busy_handler(db_, BusyHandler, nullptr);//会一直等待，知道返回锁，可以继续操作
			sqlite3_exec(db_, sql.c_str(), nullptr, 0, nullptr);
		}
		return false;
	}
};