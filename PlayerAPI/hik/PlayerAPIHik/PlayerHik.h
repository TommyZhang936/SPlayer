#pragma once
#include "../../../PlayerFactory/IPlayer.h"
#include<list>

class CPlayerFactoryHik :public IPlayerFactory
{
public:
	CPlayerFactoryHik(){}
	virtual ~CPlayerFactoryHik(){}
public:
	virtual BOOL IsBelongThis(char *pFile);
	virtual char* Name();

	virtual BOOL Init();
	virtual void Release() ;

	virtual IPlayer *CreatePlayer();
};

typedef void (CALLBACK* DisplayCBFun)(long nPort, char * pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, long nReserved);

class CPlayerHik :
	public IPlayer
{
public:
	CPlayerHik();
	virtual ~CPlayerHik();
	BOOL OpenFile(char *PFile, HWND hwnd);
	//BOOL CloseFile() ;

	BOOL Play();
	BOOL Pause(DWORD nPause);
	BOOL Stop();
	BOOL Fast();
	BOOL Slow();
	BOOL OneByOne();
	BOOL OneByOneBack();
	BOOL PlaySoundShare();
	BOOL StopSoundShare();
	BOOL SetVolume(WORD nVolume);
	DWORD GetVolume();

	BOOL  SetPlayPos(float fRelativePos);
	float GetPlayPos();
	DWORD  GetFileTime();
	DWORD  GetPlayedTime();
	BOOL GetPictureSize(LONG *pWidth, LONG *pHeight);

	BOOL  SetFileEndCallback(long nID, FileEndCallback callBack, void *pUser);

	BOOL  CapturePic(char *pSaveFile, int iType);
	BOOL FileCutStart(const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, BOOL bFast);
	BOOL FileCutClose();
	int FileCutProcess();

	BOOL  SavePic(char * pBuf, long nSize, long nWidth, long nHeight, long nType);

	static void  DisplayCBFunBack(long nPort, char * pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, long nReserved);
	static std::list<CPlayerHik*>  m_sPlayerHik;
	char m_saveFile[256];
	int m_iType;
private:
	HWND m_hwnd;
};

