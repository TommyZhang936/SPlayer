#pragma once
#include "../../../PlayerFactory/IPlayer.h"

class CPlayerFactoryDL :public IPlayerFactory
{
public:
	CPlayerFactoryDL(){}
	virtual ~CPlayerFactoryDL(){}
public:
	virtual BOOL IsBelongThisHead(char *pFile);
	virtual char* Name();

	virtual BOOL Init();
	virtual void Release();

	virtual IPlayer *CreatePlayer();
};

class CPlayerDL :
	public IPlayer
{
public:
	CPlayerDL();
	virtual ~CPlayerDL();
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
	BOOL  GetPictureSize(LONG *pWidth, LONG *pHeight);
	BOOL SetColor(DWORD nRegionNum, int nBrightness, int nContrast, int nSaturation, int nHue);
	BOOL GetColor(DWORD nRegionNum, int *pBrightness, int *pContrast, int *pSaturation, int *pHue);

	BOOL  SetFileEndCallback(long nID, FileEndCallback callBack, void *pUser);
	BOOL  SetDisplayCallback(LONG nID, DisplayCallback displayCallback, void * nUser);

	BOOL  CapturePic(char *pSaveFile, int iType);
	BOOL FileCutStart(const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, BOOL bFast);
	BOOL FileCutClose();
	int FileCutProcess();

	BOOL FileConvertStart(const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, bool bConvert, T_ConverterParameters *pConvertPara);
	BOOL FileConvertClose();
	int FileConvertProcess();
	BOOL FileConvertPause(DWORD nPause);

	static INT CALLBACK ConvertWMVCallBackFun(INT pos, void* cbpara);//ת������
	static void CALLBACK DisplayCBFunBack(LONG lHandle, DWORD /*dwDataType*/dwFrameRate, BYTE *pBuffer,
		DWORD nWidth, DWORD nHeight, int err, DWORD dwUser);
public:
	int m_process;
private:
	HWND m_hwnd;
	unsigned long m_fileTime;
	int m_iSpeed;
	DWORD m_dBeginTime;
	BOOL m_bPause;
};

