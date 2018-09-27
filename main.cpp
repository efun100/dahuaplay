#include <unistd.h>
#include <iostream>
#include "include/dhnetsdk.h"
#include "include/dhconfigsdk.h"
#include "include/dhplay.h"

using namespace std;

// PLAYSDK的空闲通道号
LONG gPlayPort = 0;

//断线回调
void CALL_METHOD Disconnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort,
			    LDWORD dwUser)
{
	cout << "Receive disconnect message, where ip:" << pchDVRIP <<
	    " and port:" << nDVRPort << " and login handle:" << lLoginID <<
	    endl;
}

// netsdk 实时回调函数
void CALL_METHOD fRealDataCB(LLONG lRealHandle, DWORD dwDataType,
			     BYTE * pBuffer, DWORD dwBufSize, LDWORD dwUser)
{
	// 把大华实时码流数据送到playsdk中
	PLAY_InputData(gPlayPort, pBuffer, dwBufSize);
	return;
}

int gnIndex = 0;
// playsdk 回调 yuv数据
void CALL_METHOD fDisplayCB(LONG nPort, char *pBuf, LONG nSize, LONG nWidth,
			    LONG nHeight, LONG nStamp, LONG nType,
			    void *pReserved)
{
	//pBuf是数据指针 nSize是buf大小，通过这两个数据，可以取到yuv数据了

	gnIndex++;
	if (gnIndex == 30) {
		gnIndex = 0;
		cout << "YUV" << endl;
	}

	return;
}

int main()
{
	LLONG lLoginHandle = 0;
	char szIpAddr[32] = { 0 };
	char szUser[32] = { 0 };
	char szPwd[32] = { 0 };
	int nPort = 37777;

	//初始化SDK资源,设置断线回调函数
	CLIENT_Init(Disconnect, NULL);

	//获取播放库空闲端口
	PLAY_GetFreePort(&gPlayPort);
	PLAY_SetStreamOpenMode(gPlayPort, STREAME_REALTIME);
	PLAY_OpenStream(gPlayPort, NULL, 0, 900 * 1024);
	PLAY_SetDisplayCallBack(gPlayPort, fDisplayCB, NULL);
	PLAY_Play(gPlayPort, NULL);

	cout << "Input device ip address:" << endl;
	cin >> szIpAddr;

	cout << "Input device port:" << endl;
	cin >> nPort;

	cout << "Input username:" << endl;
	cin >> szUser;

	cout << "Input password:" << endl;
	cin >> szPwd;

	//登陆
	NET_DEVICEINFO_Ex stLoginInfo = { 0 };
	int nErrcode = 0;

	lLoginHandle =
	    CLIENT_LoginEx2(szIpAddr, nPort, szUser, szPwd,
			    (EM_LOGIN_SPAC_CAP_TYPE) 0, NULL, &stLoginInfo,
			    &nErrcode);
	if (0 == lLoginHandle) {
		cout << "Login device failed" << endl;
		cin >> szIpAddr;
		return -1;
	} else {
		cout << "Login device success" << endl;
	}

	//拉流
	LLONG lRealHandle = CLIENT_RealPlayEx(lLoginHandle, 0, NULL);
	if (0 == lRealHandle) {
		cout << "CLIENT_RealPlayEx fail!" << endl;
		sleep(100000);
		return -1;
	}
	cout << "CLIENT_RealPlayEx success!" << endl;

	//设置拉流回调
	CLIENT_SetRealDataCallBack(lRealHandle, fRealDataCB, NULL);

	while (1) {
		sleep(100);
	}

	PLAY_Stop(gPlayPort);
	PLAY_CloseStream(gPlayPort);
	PLAY_ReleasePort(gPlayPort);

	//登出设备
	CLIENT_Logout(lLoginHandle);

	//清理SDK资源
	CLIENT_Cleanup();
	return 0;
}
