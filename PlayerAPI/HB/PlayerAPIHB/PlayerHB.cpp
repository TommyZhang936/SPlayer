#include "stdafx.h"
#include "PlayerHB.h"

#include <stdio.h>


char* CPlayerFactoryHB::Name()
{
	return "����߿�";
}

BOOL CPlayerFactoryHB::IsBelongThis(char *pFile)
{
	FILE *pfd = NULL;
	int ret = fopen_s(&pfd, pFile, "rb");
	if (pfd)//���ļ��ɹ�
	{
		char buf[8];
		memset(buf, 0, 8);
		fread(buf, 8, 1, pfd);

		if (strncmp(buf, "HBGK", 4) == 0)
		{
			return true;
		}
		fclose(pfd);
	}


	return false;
}

BOOL CPlayerFactoryHB::Init()
{
	return true;
}

void CPlayerFactoryHB::Release()
{

}

IPlayer *CPlayerFactoryHB::CreatePlayer()
{
	return new CPlayerHB();
}

////////

CPlayerHB::CPlayerHB() : m_fSpeed(1.0)
{
}


CPlayerHB::~CPlayerHB()
{
}


BOOL CPlayerHB::OpenFile(char *PFile, HWND hwnd)
{
	m_hwnd = hwnd;

	HRESULT  result = HB_PLAY2_OpenFileA(&m_hPlay, PFile);
	if (result != HBPLAY2_OK)
	{
		return false;
	}
	HBPLAY2_VIEWPORT viewPort;
	viewPort.dwSize = sizeof(HBPLAY2_VIEWPORT);
	viewPort.dwID = 1;
	viewPort.hDestWnd = hwnd;
	result = HB_PLAY2_AddViewport(m_hPlay, &viewPort);
	if (result != HBPLAY2_OK)
	{
		return false;
	}
	return true;
}

BOOL CPlayerHB::Play()
{
	HB_PLAY2_Play(m_hPlay);
	return true ;
}

BOOL CPlayerHB::Pause(DWORD nPause)
{
	if ( nPause)
	{
		HB_PLAY2_Pause(m_hPlay);
	}
	else
	{
		HB_PLAY2_Play(m_hPlay);
	}
	return true;
}

BOOL CPlayerHB::Stop()
{
	HB_PLAY2_Stop(m_hPlay);
	HB_PLAY2_Close(m_hPlay);
	return true;
}

BOOL CPlayerHB::Fast()
{
	m_fSpeed *= 2.0;
	if (m_fSpeed>64.0)
	{
		m_fSpeed = 64.0;
	}
	HRESULT hRet = HB_PLAY2_SetSpeed(m_hPlay, m_fSpeed);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CPlayerHB::Slow()
{
	m_fSpeed /= 2.0;
	if (m_fSpeed<(1.0 / 64.0))
	{
		m_fSpeed = 1.0 / 64.0;
	}
	HRESULT hRet = HB_PLAY2_SetSpeed(m_hPlay, m_fSpeed);
	 if (HBPLAY2_OK == hRet)
	 {
		 return true;
	 }
	 else
	 {
		 return false;
	 }
}


BOOL CPlayerHB::OneByOne()
{
	HRESULT hRet = HB_PLAY2_SeekNextIndex(m_hPlay);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CPlayerHB::OneByOneBack()
{
	HRESULT hRet = HB_PLAY2_SeekPreviousIndex(m_hPlay);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CPlayerHB::PlaySoundShare()
{
	DWORD flags;
	HB_PLAY2_GetEnableFlag(m_hPlay, &flags);
	HRESULT hRet = HB_PLAY2_SetEnableFlag(m_hPlay, flags | HBPLAY2_ENABLE_AUDIO_CODEC | HBPLAY2_ENABLE_SOUND);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL  CPlayerHB::StopSoundShare()
{
	DWORD flags;
	HB_PLAY2_GetEnableFlag(m_hPlay, &flags);
	HRESULT hRet = HB_PLAY2_SetEnableFlag(m_hPlay, flags | ~HBPLAY2_ENABLE_AUDIO_CODEC |~ HBPLAY2_ENABLE_SOUND);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CPlayerHB::SetVolume(WORD nVolume)
{
	HRESULT hRet = HB_PLAY2_SetVolume(m_hPlay, nVolume);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

DWORD CPlayerHB::GetVolume()
{
	DWORD dVolume=0;
	HRESULT hRet = HB_PLAY2_GetVolume(m_hPlay, &dVolume);
	return dVolume;
}

BOOL  CPlayerHB::SetPlayPos(float fRelativePos)
{
	HRESULT hRet = HB_PLAY2_SeekByRatio(m_hPlay, fRelativePos);
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float CPlayerHB::GetPlayPos()
{
	DWORD dCurTime = 0;
	DWORD dFileTime = 0;
	HB_PLAY2_GetPlayDuration(m_hPlay, &dCurTime);
	HB_PLAY2_GetFileDuration(m_hPlay, &dFileTime);
	return (float)dCurTime / (float)dFileTime;
}

DWORD CPlayerHB::GetFileTime()
{
	DWORD dTime;
	HB_PLAY2_GetFileDuration(m_hPlay, &dTime);
	return dTime/1000;
}

DWORD CPlayerHB::GetPlayedTime()
{
	DWORD dTime;
	HB_PLAY2_GetPlayDuration(m_hPlay, &dTime);
	return dTime/1000;
}

BOOL  CPlayerHB::GetPictureSize(LONG *pWidth, LONG *pHeight)
{
	return HB_PLAY2_GetPictureSize(m_hPlay, (int*)pWidth, (int*)pHeight);
}

BOOL  CPlayerHB::SetFileEndCallback(long nID, FileEndCallback callBack, void *pUser)
{
	HB_PLAY2_RegisterFileEndedCallback(m_hPlay,(PHB_PLAY2_FILE_ENDED_PROC)callBack, pUser);
	return true;
}

BOOL  CPlayerHB::CapturePic(char *pSaveFile, int iType)
{
	HRESULT hRet = 0;
	if (iType == 0)
	{
		hRet = HB_PLAY2_SnapshotToBmpFileA(m_hPlay, pSaveFile);
	}
	else
	{
		hRet = HB_PLAY2_SnapshotToJpegFileA(m_hPlay, pSaveFile, 100);
	}
	if (HBPLAY2_OK == hRet)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CPlayerHB::FileCutStart(const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, BOOL bFast)
{
	return false;
}

BOOL CPlayerHB::FileCutClose()
{
	return false;
}

int CPlayerHB::FileCutProcess()
{
	return 0;
}