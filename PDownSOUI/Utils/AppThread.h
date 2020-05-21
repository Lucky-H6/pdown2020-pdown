#pragma once
#include "ThreadPool.h"
class AppThread {
public:
	static ThreadPool HttpPool;
	static ThreadPool BlinkPool;
	static ThreadPool DBPool;
	static void WaitUntilStop() {
		DBPool.~ThreadPool();
		HttpPool.~ThreadPool();
		BlinkPool.~ThreadPool();
	}

	/*

	DWORD dwCurThreadID = GetCurrentThreadId();
	DWORD dwProcID=GetCurrentProcessId();
	*/
	/*
// 将 闭包 传递到了 UI线程
// 所以 这里 尽量 将 相同类型的 处理 放到一起 执行  而不是分开调用。

// SendMessage [&] 中的 & 是指 fn里调用的变量 都是 引用拷贝的
#define SRUNONUISYNC(fn)		SNotifyCenter::getSingletonPtr()->RunOnUISync([&](){fn})

// PostMessage [=] 中的 等号 是指 fn里调用的变量 都是 值拷贝的
#define SRUNONUI(fn)		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([=](){fn})

	*/

	/*
	提示：
	RunOnUISync RunOnUIAsync


	调用RunOnUISync，把fn包装成event msg，
					 Window响应UM_RUNONUISYNC，
					 触发SNotifyReceiver的MESSAGE_HANDLER_EX(UM_RUNONUISYNC, OnRunOnUISync)，
					 直接执行fn
	就是说通过窗口消息触发直接执行


	调用RunOnUIAsync时,把fn包装成event msg，
					 把msg加入到SNotifyCenter内部的m_asyncFuns队列，
					 Window响应timer（TIMERID_ASYNC），
					 触发SNotifyReceiver的MSG_WM_TIMER(OnTimer)，
					 执行队列中的所有fn
	就是说通过窗口的timer触发直接执行


	最后测试就是，
	都是长时间任务会卡主UI
	RunOnUISync会卡主UI+卡发送任务的子进程
	RunOnUIAsync会卡主UI   不会卡发送任务的子进程
	*/

	/*
	FireEventSync 直接执行event

	FireEventAsync 把event包装成msg，加入到m_ayncEvent队列，通过timer触发执行

	同上面，FireEventSync卡子进程，FireEventAsync不卡
	*/


};