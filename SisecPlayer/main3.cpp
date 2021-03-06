#include "sisecplayer.h"
#include <QtWidgets/QApplication>
#include "cdialogmain.h"
#include "singleapllication.h"

#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

#include <QtWidgets/QApplication>
#include <qtextcodec.h>
#include <QProcess>
#include <QSqlDatabase>
#include <QTranslator>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <qdesktopwidget.h>
#include <QSharedMemory>

#include "TempAuthDialog.h"
#include "md5.h"
#include "dialog.h"
#include "GetMac.h"
//#include "Verify.h"
#include "settings.h"

void execDlg(CDialogMain& dlg)
{
	// 	__try{
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();
	QRect rc = dlg.geometry();
	dlg.move((iScreenWidth - rc.width()) / 2, (iScreenHeigth - rc.height()) / 2);
	dlg.exec();
	// 	}
	// 	__except (EXCEPTION_EXECUTE_HANDLER)
	// 	{
	// 		std::cout << "execDlg unkonw error!" << std::endl;
	// 		//Log::add("execDlg unkonw error!", fatal);
	// 		exit(-1);
	// 	}
}
void showMainDlg(SingleApplication& a, QString &strFile)
{
	CDialogMain logd;
	a.setMainWidget(&logd);
	logd.ShellExe(strFile);
	execDlg(logd);
}


void showMainDlgNoExcept(SingleApplication& a, QString &strFile)
{
	// 	__try{
	showMainDlg(a, strFile);
	// 	}
	// 	__except (EXCEPTION_EXECUTE_HANDLER)
	// 	{
	// 		std::cout << "showMainDlgNoExcept unkonw error!" << std::endl;
	// 		exit(-1);
	// 	}
}

LONG
WINAPI
MyUnhandledExceptionFilter(
_In_ struct _EXCEPTION_POINTERS * ExceptionInfo
)
{
	//Verify::uninit();
	char szFileName[MAX_PATH];
	ZeroMemory(szFileName, sizeof(szFileName));
	GetModuleFileNameA(NULL, szFileName, MAX_PATH);
	string strFileName = szFileName;
	int iIndex = strFileName.rfind('\\');
	if (iIndex == string::npos)
	{
		return FALSE;
	}
	strFileName = strFileName.substr(0, iIndex + 1);
	strFileName += "crashReport.dmp";

	HANDLE hFile = ::CreateFileA(strFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
	//	Log::instance().AddLog(QString("MyUnhandledExceptionFilter"));

		MINIDUMP_EXCEPTION_INFORMATION einfo;
		einfo.ThreadId = ::GetCurrentThreadId();
		einfo.ExceptionPointers = ExceptionInfo;
		einfo.ClientPointers = FALSE;

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, &einfo, NULL, NULL);
		::CloseHandle(hFile);

		return TRUE;
	}
	return FALSE;
}
/*
void checkDirectory(){

	QString sDir(Verify::downloadDir());

	QDir d(sDir);
	if (!d.exists())
	{
		d.mkpath(sDir);
	}
}*/
void initDb()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName("fast_video.db");
	db.setUserName("fast_video");
	db.setPassword("fast_video!@#");
	if (!db.open())
	{
		return;
	}

	Settings::createTable();
	//Logdata::createTable();
	return;
}
int main(int argc, char *argv[])
{
	SetErrorMode(SEM_NOGPFAULTERRORBOX);
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	QApplication a(argc, argv);

	//SingleApplication a(argc, argv);
	//if (a.isRunning()) {

		//return 0;
	//}
	QSharedMemory *shareMem = new QSharedMemory(QString("SingleInstanceIdentify"));

	/* if the sharedmemory has not been created, it returns false, otherwise true.
	* But if the application exit unexpectedly, the sharedmemory will not detach.
	* So, we try twice.
	*/
	volatile short i = 2;
	while (i--)
	{
		if (shareMem->attach(QSharedMemory::ReadOnly)) /* no need to lock, bcs it's read only */
		{
			shareMem->detach();
		}
	}

	if (shareMem->create(1))


	{
		CDialogMain w;
		w.show();
		a.exec();

		if (shareMem->isAttached())
			shareMem->detach();
		delete shareMem;
	}

	return 0;
	
	char *pFile = NULL;
	QString strFile = "";
	if (argc >= 2)
	{
		pFile = argv[1];
		QTextCodec *textcode = QTextCodec::codecForName("GB18030");
		strFile = textcode->toUnicode(pFile);
	}
//	showMainDlgNoExcept(a, strFile);
	return 1;
/*	QString appDir = QApplication::applicationDirPath() + "/";
	if (argc <= 1 || strcmp(argv[1], "-s") != 0)
	{

		QProcess::startDetached(appDir + Verify::startExe());
		return 0;

	}*/
	initDb();
	MD5 md5_toApply;//用于生成申请码

	unsigned char address[1024];
	if (getLocalMac(address) > 0)
	{
		md5_toApply.update((const char *)address);//生成申请码
	}
	else
	{

	}

	if (!Dialog::hasKey()/* && !TempAuthDialog::instance().hasAuthority()*/)
	{
		Dialog w;
		w.show();
		a.exec();
		exit(0);
	}
//	showMainDlgNoExcept(a);
	/*
	if (Verify::init())
	{
		checkDirectory();
		qDebug() << "showMainDlgNoExcept";
		showMainDlgNoExcept(a);
		Verify::uninit();
	}*/
	//CDialogMain w;
	//w.show();
	//return a.exec();
}
