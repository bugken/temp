#include "stdafx.h"
#include "LotteryDB.h"
#include <iostream>
#include <thread>

using namespace std;

MUTEX LotteryMutex;
DRAW_LOTTERY_PERIOD_QUEUE DrawLotteryQueue;
CONDITION_VARIABLE LotteryConditionVariable;

bool ProcessLotteryOrder(UINT64 uiAllBet, UINT64 uiAllBetAsOfLast, UINT64 uiBonusAlready,
		float fWinRateAsOfLast, PLAYER_ORDERS_VEC tagPlayerOrdersVec, 
		PLAYER_ORDERS_VEC tagControlUserOrdersVec, UINT32& iControlType, LOTTERY_RESULT& lotteryResult)
{

	return true;
}

void LotteryProcessWorker()
{
	printf("worker thread id: %d\n", GetCurrentThreadId());
	LotteryDB lotteryDB;
	lotteryDB.DBConnect();
	while (true)
	{
		DRAW_LOTTERY_PERIOD tagDrawLotteryInfo = { 0 };
		{
			std::unique_lock<std::mutex> LotteryLock(LotteryMutex);
			LotteryConditionVariable.wait(LotteryLock, []{return !DrawLotteryQueue.empty();});
			tagDrawLotteryInfo = DrawLotteryQueue.front();
			DrawLotteryQueue.pop();
			LotteryLock.unlock();
		}

		UINT64 uiAllBet = 0;//所有下注
		UINT64 uiAllBetAsOfLast = 0;//截止上期下注
		UINT64 uiBonusAlready = 0;//已经发放的彩金
		float fWinRateAsOfLast = 0;//截止上期赢率
		UINT32 iControlType = 0;//控制类型
		LOTTERY_RESULT tagLotteryResult = {0};
		PLAYER_ORDERS_VEC tagPlayerOrdersVec, tagControlUserOrdersVec;
		bool bResult = lotteryDB.Ex_GetLotteryUserOrders(tagDrawLotteryInfo, uiAllBet, uiAllBetAsOfLast, 
				uiBonusAlready, fWinRateAsOfLast, tagPlayerOrdersVec, tagControlUserOrdersVec);
		if (bResult)
			bResult = ProcessLotteryOrder(uiAllBet, uiAllBetAsOfLast,uiBonusAlready, fWinRateAsOfLast, 
				tagPlayerOrdersVec, tagControlUserOrdersVec, iControlType, tagLotteryResult);
		if (bResult)
			lotteryDB.Ex_UpdateGameResult(tagDrawLotteryInfo.iTypeID, tagDrawLotteryInfo.strCurrentIssueNumber, 
				iControlType, tagLotteryResult);
	}
}

bool IsDrawLotterySecond()
{
	bool bIsDrawing = false;
	struct tm stTempTm;
	char cSecond[20];

	time_t nowTime = time(NULL);
	localtime_s(&stTempTm, &nowTime);
	sprintf_s(cSecond, "%02d", stTempTm.tm_sec);
	if (strcmp("50", cSecond) == 0)
	{
		bIsDrawing = true;
		printf("...................50s...............\n");
	}

	return bIsDrawing;
}

void LoopCheckLottery()
{
	LotteryDB lotteryDB;
	lotteryDB.DBConnect();
	while (true)
	{
		printf("loop check lottery every 100 miliseconds\n");
		//检查是否是开奖的时间，每分钟的第50秒
		if (IsDrawLotterySecond())
		{
			DRAW_LOTTERY_PERIOD_QUEUE tagDrawLotteryQueue;
			if (lotteryDB.Ex_GetDrawLottery(tagDrawLotteryQueue))
			{
				{
					lock_guard<mutex> LotteryLock(LotteryMutex);
					DrawLotteryQueue = tagDrawLotteryQueue;
				}
				//printf("DrawLotteryQueue:%d\n", DrawLotteryQueue.size());
				//while (!DrawLotteryQueue.empty())
				//{
				//	DRAW_LOTTERY_PERIOD tag = DrawLotteryQueue.front();
				//	DrawLotteryQueue.pop();
				//	printf("uiTypeID:%d\n", tag.uiTypeID);
				//	printf("uiUserControled:%d\n", tag.uiUserControled);
				//	printf("uiControlRate:%d\n", tag.uiControlRate);
				//	printf("uiPowerControl:%d\n", tag.uiPowerControl);
				//	printf("strCurrentIssueNumber:%s\n", tag.strCurrentIssueNumber);
				//	printf("strBeginIssueNumber:%s\n", tag.strBeginIssueNumber);
				//	printf("strLastIssueNumber:%s\n", tag.strLastIssueNumber);
				//}
				LotteryConditionVariable.notify_all();
			}
		}
		else
		{
			Sleep(100);
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("main process id: %d\n", GetCurrentProcessId());
	printf("main thread id: %d\n", GetCurrentThreadId());

	//先启动工作线程再循环检查是否开奖
	thread arrProcessWorkerThreads[4];
	for (int i = 0; i < WORKERS_THREAD_NUM; i++)
	{
		arrProcessWorkerThreads[i] = thread(LotteryProcessWorker);
	}
	for (auto &Thread : arrProcessWorkerThreads)
	{
		Thread.detach();
	}

	LoopCheckLottery();

	return 0;
}

