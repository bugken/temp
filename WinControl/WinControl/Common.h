#pragma  once

#include "stdafx.h"
#include <windows.h>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

#define ISSUE_NUMBER_LEN	20                         
#define COLOR_LEN			10 
#define NUMBER_LEN			5
#define WORKERS_THREAD_NUM	4//�����߳�����

#define CONDITION_VARIABLE condition_variable
#define MUTEX mutex

class CTicker
{
public:
	CTicker(string name)
	{
		mName = name;
		mBeginTick = ::GetTickCount();
	}
	~CTicker()
	{
		unsigned int endTick = ::GetTickCount();
		//ִ�д洢����ʱ�䳬��100���룬��¼�澯������500����¼������־
		unsigned int usedms = endTick - mBeginTick;
		printf("%s, The execution of stored procedures[%s] last %u milliseconds \n", __FUNCTION__, mName.c_str(), usedms);
	}
private:
	unsigned int mBeginTick;
	string mName;
};

//��ǰ�ں� ��ʼ������ں� ��һ�ں� TypeID ��ɱUserID Ӯ�� ɱ������
typedef struct _DRAWLOTTERY_INFO
{
	UINT32  uiTypeID;								//��Ʊ����
	UINT32  uiUserControled;						//��ɱUserID
	UINT32  uiControlRate;							//ɱ��
	UINT32  uiPowerControl;							//��ɱ����
	char	cCurrentIssueNumber[ISSUE_NUMBER_LEN];	//��ǰ�ں�
	char	cBeginIssueNumber[ISSUE_NUMBER_LEN];	//���俪ʼ�ں�
	char	cLastIssueNumber[ISSUE_NUMBER_LEN];		//��һ���ں�
}DRAWLOTTERY_INFO;
typedef std::queue<DRAWLOTTERY_INFO> DRAWLOTTERY_INFO_QUEUE;

typedef struct _ORDER_INFO
{
	UINT32  uiUserID;
	char	cIssueNumber[ISSUE_NUMBER_LEN];
	UINT32  uiTypeID;
	char	cSelectType[COLOR_LEN];
	float   fRealAmount;
}ORDER_INFO;
typedef std::vector<ORDER_INFO>  ORDER_INFO_VEC;

typedef struct  _LOTTERY_ORDER_INFO
{

}LOTTERY_ORDER_INFO;

typedef struct _Finally_Result
{
	char	cLotteryNumber[NUMBER_LEN];
	char	cLotteryColor[COLOR_LEN];
}Finally_Result;




