//***********************************************//


//***********************************************//
#ifndef _VRPLAYSDK_BELL_H
#define _VRPLAYSDK_BELL_H

#ifdef VRSDK_BELL_EXPORTS
#define VRSDK_BELL_API extern "C" __declspec(dllexport)
#else
#define VRSDK_BELL_API extern "C" __declspec(dllimport)
#endif

typedef enum {YUV420}FrameType;

typedef struct
{
	FrameType Type;
	long Timestamp;
	long FrameRate;
	long FrameId;
	long Width;
	long Height;
	char* Buffer;
	long Size;
}FrameInfo;

//音频解码回调的数据结构，音频数据格式为PCM。
typedef struct
{
	long SamplesPerSec ;//采样率 
	long BitsPerSample ;//每个采样需要的BIT数 
	long Timestamp; //时戳（单位：毫秒） 
	char * Buffer ;//数据缓冲区
	long Size ;//数据长度 
}AudioFrameInfo;


//视频编码或封装格式 
typedef enum  
{
	MPEG_PS,//符合国标GB/T28181-2011的PS流（不含RTP头） 
	H264,   //H.264. 
	MPEG2, //MPEG2. 
	MPEG4,//MPEG4. 
	MJPEG, //Motion JPEG. 
	SVAC_VIDEO  //SVAC视频编码 
}VideoCodeFormat;


//视频编码后帧类型 
typedef enum  
{
	I, //I帧 
	P, //P帧 
	B, //B帧 
	PARAM_SET  //PARAM_SET指H264的SPS、PPS等参数集信息 
}VideoFrameType;

//视频编码数据结构（未解码的编码格式），或GB/T 28181规范要求的PS封装的混合视音频包。 
typedef struct 
{
	char * Buffer;//数据缓冲区 
	long Size ;//数据长度 
	VideoCodeFormat CodeFormat ;//编码类型 
	VideoFrameType Type ;//帧类型 
	long Timestamp ;//时戳（单位：毫秒） 
	long FrameRate ;//图像帧率（单位：帧/秒） 
	long Width ;//图像宽度（单位：像素） 
	long Height ;//图像高度（单位：像素） 

}CodedVideoFrameInfo ;


//音频编码格式 
typedef enum  
{
VISS_G711_PCMA,
//G.711 A-law (常用) 

VISS_G711_PCMU, 
//G.711 µ-law (美国、日本标准) 

VISS_G723_1,
//G.723.1. 

VISS_G729,
//G.729. 

VISS_SVAC_AUDIO,
//SVAC音频编码 

VISS_AMR_NB,
//AMR NB. 

VISS_AAC  
//Advanced Audio Coding. 
}AudioCodeFormat;


//音频编码数据结构（未解码的编码格式）。 
typedef struct  
{
	char * Buffer ;
	//数据缓冲区 

	long Size ;
	//数据长度 

	AudioCodeFormat CodeFormat ;
	//音频编码格式 

	long Timestamp ;
	//时戳（单位：毫秒） 

	long SamplesPerSec ;
	//采样率 

	long BitsPerSample ;
	//每个采样需要的BIT数 
}CodedAudioFrameInfo;



VRSDK_BELL_API LPCTSTR Viss_GetErrorMessage(HRESULT errorCode); 

//VRSDK_BELL_API HRESULT Viss_OpenFile(long* hHandle, LPCTSTR sFileName);
VRSDK_BELL_API HRESULT Viss_OpenFile(long* hHandle, char* sFileName);
VRSDK_BELL_API HRESULT Viss_CloseFile(long hHandle);

VRSDK_BELL_API HRESULT Viss_OpenStream(long* hHandle, char* pFileHeadBuffer, long nSize, long nBufferSize, long nMediaType);
VRSDK_BELL_API HRESULT Viss_CloseStream(long hHandle);

VRSDK_BELL_API HRESULT Viss_InputData(long hHandle, char* pBuffer, long nSize);

VRSDK_BELL_API HRESULT Viss_SetFrameBufferSize4InputData(long hHandle, long nSeconds);
VRSDK_BELL_API HRESULT Viss_ReplayBufferedFrames(long hHandle, long action);

VRSDK_BELL_API HRESULT Viss_RecordStart(long hHandle, LPCTSTR sFileName, long nRecordType);
VRSDK_BELL_API HRESULT Viss_RecordStop(long hHandle);

VRSDK_BELL_API HRESULT Viss_RecordSwitchFile(long hHandle, LPCTSTR sFileName);

VRSDK_BELL_API HRESULT Viss_SetRecordFileReadyCallback(long hHandle, void(CALLBACK* pRecordFileReadyCallback)
													   (long hHandle, LPCTSTR sFileName, void* pUser), void* pUser); //

VRSDK_BELL_API HRESULT Viss_PlaySound(long hHandle);
VRSDK_BELL_API HRESULT Viss_StopSound(long hHandle);

VRSDK_BELL_API HRESULT Viss_SetVolume(long hHandle, long nVolume);
VRSDK_BELL_API HRESULT Viss_GetVolume(long hHandle, long* pVolume);

VRSDK_BELL_API HRESULT Viss_Play(long hHandle, HWND hWnd);
VRSDK_BELL_API HRESULT Viss_Stop(long hHandle);
VRSDK_BELL_API HRESULT Viss_Pause(long hHandle, long nPause);

VRSDK_BELL_API HRESULT Viss_SetPlayPos(long hHandle, long nPos);
VRSDK_BELL_API HRESULT Viss_GetPlayPos(long hHandle, long* pPos);

VRSDK_BELL_API HRESULT Viss_SetPlaySpeed(long hHandle,float fSpeed); //
VRSDK_BELL_API HRESULT Viss_GetPlaySpeed(long hHandle,float* pSpeed); //

VRSDK_BELL_API HRESULT Viss_CapPic(long hHandle, LPCTSTR sFileName);

VRSDK_BELL_API HRESULT Viss_SetFileEndCallback(long hHandle, void(CALLBACK* pFileEndCallback)(long hHandle, void* pUser), void* pUser);

VRSDK_BELL_API HRESULT Viss_RegisterDrawFun(long hHandle,void(CALLBACK* pDrawFun)(long hHandle, HDC hDc,void* pUser),void* pUser);

VRSDK_BELL_API HRESULT Viss_NextFrame(long hHandle);
VRSDK_BELL_API HRESULT Viss_PreviousFrame(long hHandle);

VRSDK_BELL_API HRESULT Viss_SetColor(long hHandle, long nBrightness, long nContrast, long nSaturation, long nHue);
VRSDK_BELL_API HRESULT Viss_GetColor(long hHandle, long* pBrightness, long* pContrast, long* pSaturation, long* pHue);

VRSDK_BELL_API HRESULT Viss_GetFileTotalFrames(long hHandle,long* pFrames);

VRSDK_BELL_API HRESULT Viss_GetPlayedFrames(long hHandle,long* pFrames);

VRSDK_BELL_API HRESULT Viss_SetCurrentFrameNum(long hHandle, long nFrameNum);

VRSDK_BELL_API HRESULT Viss_SetDecodeCallback(long hHandle,void(CALLBACK* pVideoCallback)
											  (long hHandle,FrameInfo* pFrame,void* pUser),void* pUser);

VRSDK_BELL_API HRESULT Viss_SetDecodeCallbackEx  ( long  hHandle,  
  void(CALLBACK *pVideoCallback)(long hHandle, FrameInfo *pVideoFrame, void *pUser)  ,  
  void(CALLBACK *pAudioCallback)(long hHandle, AudioFrameInfo *pAudioFrame, void *pUser)  ,  
  void *  pUser  
 ) ;

VRSDK_BELL_API HRESULT Viss_InitAudio(long* hHandle); //

VRSDK_BELL_API HRESULT Viss_SetAudioInCallback(long hHandle, void(CALLBACK* pAudioInCallback)
											   (long hHandle, char* pBuffer,long nSize,void* pUser),void* pUser);  //

VRSDK_BELL_API HRESULT Viss_InputAudioData(long hHandle, char* pBuffer, long nSize);  //

VRSDK_BELL_API HRESULT Viss_CloseAudio(long hHandle);  //

VRSDK_BELL_API HRESULT Viss_DVR_Login(long* hHandle, LPCTSTR sIp, long nPort, LPCTSTR sUserName, LPCTSTR sPassword);

VRSDK_BELL_API HRESULT Viss_DVR_Logout(long hHandle);

VRSDK_BELL_API HRESULT Viss_DVR_OpenFileByName(long hHandle,long* hFileHandle,LPCTSTR sFileName);

VRSDK_BELL_API HRESULT Viss_DVR_OpenFileByTime(long hHandle,long* hFileHandle,long nChannel, LPCTSTR sStartTime, LPCTSTR sStopTime);

VRSDK_BELL_API HRESULT Viss_DVR_CloseFile(long hHandle);

VRSDK_BELL_API HRESULT Viss_DVR_GetFileByName(long hHandle, long* hFileHandle, LPCTSTR sDVRFileName, LPCTSTR sSavedFileName);

VRSDK_BELL_API HRESULT Viss_DVR_GetFileByTime(long hHandle, long* hFileHandle, long nChannel, LPCTSTR sStartTime, 
											  LPCTSTR sEndTime, LPCTSTR sSavedFileName);

VRSDK_BELL_API HRESULT Viss_DVR_GetDownloadPos(long hFileHandle, long* nPercent);

VRSDK_BELL_API HRESULT Viss_DVR_StopGetFile(long hFileHandle);

VRSDK_BELL_API HRESULT Viss_SetSvc(long hHandle, long type, HWND hWnd);

VRSDK_BELL_API HRESULT Viss_DVR_LoginEX(long* hHandle, LPCTSTR sIp, long nPort, LPCTSTR sConnectionId, LPCTSTR sUserName, LPCTSTR sPassword);

VRSDK_BELL_API HRESULT Viss_SetNeedDecodePasswordCallbackFun(long hHandle, void(CALLBACK* pNeedDecodePasswordFun)(long hHandle,  long nErrorCode, void* pUser), void* pUser);

VRSDK_BELL_API HRESULT Viss_SetDecodePassword(long hHandle, LPCTSTR sPassword);

VRSDK_BELL_API HRESULT Viss_SetExtensionInfoCallbackFun(long hHandle, void(CALLBACK* pExtensionInfoFun)(long hHandle, LPCTSTR sExtensionInfo, void* pUser), void* pUser);

VRSDK_BELL_API HRESULT Viss_GetBytesPS(long hHandle, long* TotalBytesPS, long* BlBytesPS);

VRSDK_BELL_API HRESULT Viss_SetROI(long hHandle, long type);

VRSDK_BELL_API HRESULT Viss_ShowTime(long hHandle, bool bShow);

VRSDK_BELL_API HRESULT Viss_GetFileTotalTime(long hHandle, long* pTotalTime);

VRSDK_BELL_API HRESULT Viss_SetPlayTimePos(long hHandle, long nTimePos);

VRSDK_BELL_API HRESULT Viss_GetPlayTimePos (long hHandle, long* pTimePos);

VRSDK_BELL_API HRESULT Viss_SetPlayAbsTimePos  (long hHandle, LPCTSTR  sTimePos);

VRSDK_BELL_API HRESULT Viss_GetExtensionInfoFromFile( LPCTSTR  sFileName,
													 void(CALLBACK *pGetExtInfoCallback)(long retCode, 
													 LPCTSTR sBeginTime, 
													 LPCTSTR sEndTime, 
													 LPCTSTR sAlarmExtInfoList,
													 void *pUser),  
													 void* pUser);

VRSDK_BELL_API HRESULT Viss_SetTranscodeGB28181Callback(long  hHandle,  
													 void(CALLBACK *pCallback)(long hHandle, CodedVideoFrameInfo *pFrame, void *pUser),  
													 void *  pUser) ;

VRSDK_BELL_API HRESULT Viss_SetRawCodeDataCallback  ( long  hHandle,  
													 void(CALLBACK *pVideoCallback)(long hHandle, CodedVideoFrameInfo *pVideoFrame, void *pUser)  ,  
													 void(CALLBACK *pAudioCallback)(long hHandle, CodedAudioFrameInfo *pAudioFrame, void *pUser)  ,  
													 void *  pUser  
													 ) ;

VRSDK_BELL_API HRESULT Viss_RecordScissor(long *  hHandle,
							LPCTSTR  sSourceFileName,  
							 LPCTSTR  sDestFileName,  
							 long  iWriteMethod,  
							 long  iBeginTime,  
							 long  iDuration  );

VRSDK_BELL_API HRESULT Viss_RecordSplit(long *  hHandle,
										LPCTSTR  sSourceFileName,
										LPCTSTR  sDestFileName[],
										LONG  nParts,
										LONG  nThresh 
										);
#endif

