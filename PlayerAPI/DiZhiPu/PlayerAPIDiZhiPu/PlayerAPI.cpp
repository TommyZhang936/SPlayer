// PlayerAPI.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerDiZhiPu.h"

IPlayerFactory * GetPlayerFactory()
{
	CPlayerFactoryDiZhiPu *pPlayerFacory = new CPlayerFactoryDiZhiPu();
	return pPlayerFacory;
}