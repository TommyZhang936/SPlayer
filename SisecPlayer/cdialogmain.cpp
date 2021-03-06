
#include "cdialogmain.h"
#include "qfiledialog.h"
#include "qmessagebox.h"

#include <QDomComment>
#include <QTextStream>
#include <qmenu.h>
#include <qdatetime.h>

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QThread>

#include "cdialogconfig.h"
#include "cdialogconverter.h"
#include "cdialogcut.h"
#include "cdialoghelp.h"
#include "cupdatedlg.h"
#include "cdialogmessagebox.h"
#include <windows.h>
#include <windowsx.h>
#include "cdialogvcamain.h"
#include "public_def.h"

#ifndef POINTER_64
#define	POINTER_64	__ptr64
#endif
//#include "ukey/Sage_USB_API.h"
//#pragma comment(lib,"ukey/Sage_USB_API.lib")
#include "ukey/Verifier.h"
#pragma comment(lib,"ukey/Verifier.lib")


#define FILEPLAYLISTXML "playlist.xml"
#define ROOTTEXT  "playFileList"
#define NOTETEXT "fileName"
#define DX 180

UkeyThread::UkeyThread():m_stopFlag(false)
{

}

UkeyThread::~UkeyThread()
{

}

bool UkeyThread::VerifyUkey()
{	
	return IsPresent();
}

void UkeyThread::run()
{
	while (!m_stopFlag)
	{
		QThread::msleep(3000);//每3秒检测一次
		if (!VerifyUkey())
		{
			emit ukeyDown();			
		}
		else
		{
			
			emit ukeyUp();
		}
	}
}

void UkeyThread::Stop()
{
	m_stopFlag = true;
}

UkeyThread CDialogMain::s_ukeyThread;

GetImageProcessInterfaceFun CDialogMain::s_GetImgProInterfaceFun = NULL;
FreeImageProcessInterfaceFun CDialogMain::s_FreeImgProInterfaceFun = NULL;
ImageProcessFun CDialogMain::s_ImgProcessFun = NULL;
InitProcessFun CDialogMain::s_InitImgProcessFun = NULL;

bool CDialogMain::GetImageProcessFun()
{
	QString strImageProcessPath = QApplication::applicationDirPath() + "/factorys/Standard/Imageprocess.dll";
	QFile file(strImageProcessPath);
	if (file.exists())
	{
		HMODULE hDllLib = ::LoadLibrary(strImageProcessPath.toStdWString().c_str());
		if (hDllLib)
		{
			//获取动态连接库里的函数地址。
			s_GetImgProInterfaceFun = (GetImageProcessInterfaceFun)GetProcAddress(hDllLib, "GetImageProcessInterface");
			s_FreeImgProInterfaceFun = (FreeImageProcessInterfaceFun)GetProcAddress(hDllLib, "FreeImageProcessInterface");
			s_ImgProcessFun = (ImageProcessFun)GetProcAddress(hDllLib, "ImageProcess");
			s_InitImgProcessFun = (InitProcessFun)GetProcAddress(hDllLib, "InitProcess");
			return true;
		}
	
	}
	return false;
}

CDialogMain::CDialogMain(QWidget *parent)
: NoFlameDlg(parent), m_pConvertDlg(NULL), m_pCutDlg(NULL)
{
	ui.setupUi(this);

	InitPlayerFactory();
	GetImageProcessFun();//获取图像处理接口

	for (int i = 0; i< NUM; i++)
	{
		m_PlayWnd[i] = new CWidgetPlayWnd(i,this);
		m_PlayCtrl[i] = new CFormPlayCtrl(i, m_PlayWnd[i], this);
	}

	m_AudioDubCtrl = new CFormPlayCtrl(NUM, NULL,this);
	m_AudioDubCtrl->SetAudioAttrible();
	m_AudioDubCtrl->hide();
	m_PlayCtrl[0]->SetAudioCtrl(m_AudioDubCtrl);

	
	m_iScreenWidth = 0;
	m_iScreenHeigth = 0;
	m_bFullScreen = false;

	m_PlayMode = SINGLE_MODE;

	ReadPlayFileList();//读取播放列表
	CDialogConfig::ReadConfigXml();//读设置信息，包括截图路径等

	m_pVCAMaindlg = new CDialogVCAMain(this);
	m_pVCAMaindlg->hide();

	IniSysMenu();//初始所有菜单
	IniWndMenu();
	IniPlayListMenu();
	
	ui.BtnFullScreen->hide();
	ui.frame_toolBar->raise();
	ui.frameBackGround->raise();
	ui.frameComboBox->raise();
	ui.BtnHideList->raise();

	//ui.splitter->setSizes()
//	ui.splitter->setStretchFactor(0, 1); 
//	ui.splitter->setStretchFactor(1, 0);
	ui.splitter->setCollapsible(0, false);
	ui.splitter->setCollapsible(1, false);

	connect(&s_ukeyThread, SIGNAL(ukeyDown()), this, SLOT(OnUkeyDown()));
	connect(&s_ukeyThread, SIGNAL(ukeyUp()), this, SLOT(OnUkeyUp()));
	connect(&m_ukeyDownMsgdlg, SIGNAL(accepted()), this, SLOT(OnTerminated()));
	connect(m_vcaAct, SIGNAL(triggered()), this, SLOT(OnVCATriggered()));
	connect(m_configAct, SIGNAL(triggered()), this, SLOT(OnConfigTriggered()));
	connect(m_converterAct, SIGNAL(triggered()), this, SLOT(OnConvertTriggered()));
	connect(m_cutAct, SIGNAL(triggered()), this, SLOT(OnCutTriggered()));
	connect(m_updateAct, SIGNAL(triggered()), this, SLOT(OnUpdateTriggered()));
	connect(m_helpAct, SIGNAL(triggered()), this, SLOT(OnHelpTriggered()));
	connect(m_WndActGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnWndModeTriggered(QAction*)));
	connect(m_renderActGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnRenderModeTriggered(QAction*)));
	connect(m_ActGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnPlayModeTriggered(QAction*)));
	connect(ui.BtnMore, SIGNAL(clicked()), this, SLOT(OnBtnMoreClick()));
	connect(ui.BtnFullScreen, SIGNAL(clicked()), this, SLOT(OnBtnFullScreenClick()));
	connect(ui.BtnClose, SIGNAL(clicked()), this, SLOT(OnBtnCloseClick()));
	connect(ui.BtnMax, SIGNAL(clicked()), this, SLOT(OnBtnMaxClick()));
	connect(ui.BtnMin, SIGNAL(clicked()), this, SLOT(OnBtnMinClick()));
//	connect(ui.BtnSingle, SIGNAL(clicked()), this, SLOT(OnBtnSingleClick()));
	connect(ui.BtnDiv4Time, SIGNAL(clicked()), this, SLOT(OnBtnDiv4TimeClick()));
	connect(ui.BtnFileAdd, SIGNAL(clicked()), this, SLOT(OnBtnFileAddClick()));
	connect(ui.comboBox, SIGNAL(activated(int)), this, SLOT(OnActivated(int))); 
	connect(ui.BtnFileOpen, SIGNAL(clicked()), this, SLOT(OnBtnFileOpenClick()));
	connect(ui.BtnFileDel, SIGNAL(clicked()), this, SLOT(OnBtnFileDelClick()));
	connect(ui.BtnFileDelAll, SIGNAL(clicked()), this, SLOT(OnBtnFileDelAllClick()));
	connect(ui.BtnHideList, SIGNAL(clicked()), this, SLOT(OnBtnHideListClick()));
	connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OnListWidgetItemDbClick(QListWidgetItem*)));
	connect(ui.splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(OnSplitterMoved(int, int )));
	//connect(ui.listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(OnListWidgetItemDbClick(QListWidgetItem*)));
//	connect(ui.treeWidgetFileList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnTreeWidgetItemDbClick(QTreeWidgetItem*, int)));
//	connect(ui.BtnVCA, SIGNAL(clicked()), this, SLOT(OnBtnVCAClick()));
//	ui.treeWidgetFileList->hide();
//	ui.BtnVCA->hide();
	
	installEventFilter(this);

	ui.BtnWndMode->installEventFilter(this);
	ui.BtnDiv4Time->installEventFilter(this);

	ui.BtnHideList->installEventFilter(this);

	ui.BtnMore->installEventFilter(this);
	ui.BtnMin->installEventFilter(this);
	ui.BtnMax->installEventFilter(this);

	setAcceptDrops(true);

	ui.frame_fileList->hide();
	ui.BtnHideList->setToolTip(QStringLiteral("展开播放列表"));
	m_bHide = true;
	

#ifdef UKey
	s_ukeyThread.start();
#endif
}

CDialogMain::~CDialogMain()
{
	for (int i = 0; i< NUM; i++)
	{
		delete m_PlayCtrl[i];
		delete m_PlayWnd[i];
	}

	delete m_pVCAMaindlg;
}

//启动时检测
bool CDialogMain::VerifyUkey()
{
	//if (!s_ukeyThread.VerifyUkey())
	int threadID = (int)QThread::currentThreadId();
	if (!Verifiy(threadID))
	{
		QMessageBox::information(NULL, QStringLiteral("提示"), QStringLiteral("未检测到有效的UKey,无法运行！"));
		return false;
	}
	return true;
}

void CDialogMain::OnTerminated()
{
	OnBtnCloseClick();
	exit(0);//退出应用程序
}

void CDialogMain::OnSplitterMoved(int pos, int index)
{
	SetPlayMode(m_PlayMode);
	SetPlayCtrlPos();
	SetBtnHideListPos();
}

void CDialogMain::OnUkeyDown()
{
	if (this->isEnabled())//
	{
		for (int i = 0; i < NUM; i++)
		{
			if (m_PlayCtrl[i]->IsPlaying())
			{
				m_PlayCtrl[i]->Pause(1);
			}
		}

		if (m_pConvertDlg)
		{
			((CDialogConverter*)m_pConvertDlg)->UkeyDownStop();
		}
		if (m_pCutDlg )
		{
			((CDialogCut*)m_pCutDlg)->UkeyDownStop();
		}

		this->setEnabled(false);
		//m_ukeyDownMsgdlg.setWindowFlags(Qt::FramelessWindowHint);
		m_ukeyDownMsgdlg.setWindowTitle(QStringLiteral("提示"));
		m_ukeyDownMsgdlg.setText(QStringLiteral("未检测到有效的UKey,关闭此对话框会退出应用程序！"));

	//	m_ukeyDownMsgdlg.addButton(QStringLiteral("退出应用程序"), QMessageBox::AcceptRole);//QMessageBox::YesRole
	//	int flag = m_ukeyDownMsgdlg.exec();//flag返回0，是QMessageBox::NoButton，什么原因？
	//	if (QMessageBox::NoButton == m_ukeyDownMsgdlg.exec())
		if (QMessageBox::Ok == m_ukeyDownMsgdlg.exec())
	//	if (QMessageBox::Accepted == m_ukeyDownMsgdlg.exec())//不行，添加的按钮不行
		{
			emit m_ukeyDownMsgdlg.accepted();//需手动加发送信号，要不connect(&m_ukeyDownMsgdlg, SIGNAL(accepted()), this, SLOT(OnBtnCloseClick()))无效
		}
	}

}

void CDialogMain::OnUkeyUp()
{
	if (!this->isEnabled())
	{	
		m_ukeyDownMsgdlg.reject();
		this->setEnabled(true);
		for (int i = 0; i < NUM; i++)
		{
			if (m_PlayCtrl[i]->IsPlaying())
			{
				m_PlayCtrl[i]->Pause(0);
			}
		}
	}
}

void  CDialogMain::IniSysMenu()
{
	QMenu *pMenu = new QMenu(QStringLiteral("menu"), ui.BtnMore);
	m_renderIniAct = new QAction(QStringLiteral("原始比例"), this);
	m_rendeStrechAct = new QAction(QStringLiteral("铺满窗口"), this);
	m_render4_3Act = new QAction(QStringLiteral("4:3"), this);
	m_rendeStrech16_9Act = new QAction(QStringLiteral("16:9"), this);
	m_configAct = new QAction(QIcon(":/Skin/configure.png"), QStringLiteral("设置"), this);
	m_converterAct = new QAction( QStringLiteral("转码"), this);
	m_cutAct = new QAction( QStringLiteral("剪切"), this);
	m_vcaAct = new QAction(QIcon(""), QStringLiteral("视频分析"), this);
	m_updateAct = new QAction(QIcon(":/Skin/Update.png"), QStringLiteral("升级"), this);
	m_helpAct = new QAction(QIcon(":/Skin/Help.png"), QStringLiteral("帮助"), this);
	
	//QMenu *pDiaplayMenu = pMenu->addMenu(QIcon(":/Skin/configure.png"), QStringLiteral("显示比例"));
	QMenu *pDiaplayMenu = pMenu->addMenu( QStringLiteral("显示比例"));
	pDiaplayMenu->addAction(m_renderIniAct);
	pDiaplayMenu->addAction(m_rendeStrechAct);
	pDiaplayMenu->addAction(m_render4_3Act);
	pDiaplayMenu->addAction(m_rendeStrech16_9Act);
	m_renderIniAct->setCheckable(true);
	m_rendeStrechAct->setCheckable(true);
	m_render4_3Act->setCheckable(true);
	m_rendeStrech16_9Act->setCheckable(true);
	m_renderIniAct->setChecked(true);
	m_renderMode = RenderIni;
	m_renderActGroup = new QActionGroup(this);
	m_renderActGroup->addAction(m_renderIniAct);
	m_renderActGroup->addAction(m_rendeStrechAct);
	m_renderActGroup->addAction(m_render4_3Act);
	m_renderActGroup->addAction(m_rendeStrech16_9Act);

	QMenu *pConvertMenu = pMenu->addMenu(QStringLiteral("转码/剪切"));
	pConvertMenu->addAction(m_converterAct);
	pConvertMenu->addAction(m_cutAct);
//#ifndef DISKSN  //播放器任何版本都不打包检索功能
	//pMenu->addAction(m_vcaAct);
//#endif
	pMenu->addAction(m_configAct);
	pMenu->addAction(m_updateAct);
	pMenu->addAction(m_helpAct);
	ui.BtnMore->setMenu(pMenu);
}

void  CDialogMain::IniWndMenu()
{
	QMenu *pWndMenu = new QMenu(QStringLiteral("WndModeMenu"), ui.BtnPlayList);
/*	m_Wnd1Act = new QAction(QIcon(":/Skin/single.png"), QStringLiteral("1"), this);
	m_Wnd4Act = new QAction(QIcon(":/Skin/grid4.png"), QStringLiteral("4"), this);
	m_Wnd9Act = new QAction(QIcon(":/Skin/grid9.png"), QStringLiteral("9"), this);
	m_Wnd16Act = new QAction(QIcon(":/Skin/grid16.png"), QStringLiteral("16"), this);*/

	m_Wnd1Act = new QAction( QStringLiteral("1"), this);
	m_Wnd4Act = new QAction(QStringLiteral("4"), this);
	m_Wnd9Act = new QAction(QStringLiteral("9"), this);
	m_Wnd16Act = new QAction(QStringLiteral("16"), this);
	m_Wnd1Act->setCheckable(true);
	m_Wnd4Act->setCheckable(true);
	m_Wnd9Act->setCheckable(true);
	m_Wnd16Act->setCheckable(true);

	pWndMenu->addAction(m_Wnd1Act);
	m_Wnd1Act->setChecked(true);
	m_WndMode = WND1;
	pWndMenu->addAction(m_Wnd4Act);
	pWndMenu->addAction(m_Wnd9Act);
	pWndMenu->addAction(m_Wnd16Act);
	ui.BtnWndMode->setMenu(pWndMenu);

	m_WndActGroup = new QActionGroup(this);
	m_WndActGroup->addAction(m_Wnd1Act);
	m_WndActGroup->addAction(m_Wnd4Act);
	m_WndActGroup->addAction(m_Wnd9Act);
	m_WndActGroup->addAction(m_Wnd16Act);
}

void  CDialogMain::IniPlayListMenu()
{
	QMenu *pPlayListMenu = new QMenu(QStringLiteral("PlayModeMenu"), ui.BtnPlayList);
	m_singlePlayAct = new QAction(QStringLiteral("单个播放"), this);
	m_listPlayAct = new QAction(QStringLiteral("顺序播放"), this);
	m_singleRepeatAct = new QAction(QStringLiteral("单个循环"), this);
	m_listRepeatAct = new QAction(QStringLiteral("顺序循环"), this);
	m_singlePlayAct->setCheckable(true);
	m_listPlayAct->setCheckable(true);
	m_singleRepeatAct->setCheckable(true);
	m_listRepeatAct->setCheckable(true);

	pPlayListMenu->addAction(m_listPlayAct);
	m_listPlayAct->setChecked(true);
	m_playListMode = LIST;
	pPlayListMenu->addAction(m_singlePlayAct);
	pPlayListMenu->addAction(m_singleRepeatAct);
	pPlayListMenu->addAction(m_listRepeatAct);
	ui.BtnPlayList->setMenu(pPlayListMenu);

	m_ActGroup = new QActionGroup(this);
	m_ActGroup->addAction(m_singlePlayAct);
	m_ActGroup->addAction(m_listPlayAct);
	m_ActGroup->addAction(m_singleRepeatAct);
	m_ActGroup->addAction(m_listRepeatAct);
}

void  CDialogMain::OnConfigTriggered()
{
	CDialogConfig dlg(this);
	dlg.exec();
}

void  CDialogMain::OnConvertTriggered()
{
	CDialogConverter dlg(this);
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();
	QRect rc = dlg.geometry();
	dlg.move((iScreenWidth - rc.width()) / 2, (iScreenHeigth - rc.height()) / 2);
	m_pConvertDlg = &dlg;
	dlg.exec();
	m_pConvertDlg = NULL;
}

void  CDialogMain::OnCutTriggered()
{
	CDialogCut dlg(this);
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();
	QRect rc = dlg.geometry();
	dlg.move((iScreenWidth - rc.width()) / 2, (iScreenHeigth - rc.height()) / 2);
	m_pCutDlg = &dlg;
	dlg.exec();
	m_pCutDlg = NULL;
}


void  CDialogMain::OnUpdateTriggered()
{
	CUpdateDlg dlg(this);
	dlg.CheckNewVersion();
	dlg.exec();
}


void  CDialogMain::OnHelpTriggered()
{
	CDialogHelp dlg(this);
	dlg.exec();
}

 void  CDialogMain::OnWndModeTriggered(QAction*act)
{
	if (m_PlayMode == TIME_MODE)
	{
		bool bPlaying = false;
		for (int i = 0; i < m_curWndNum; i++)
		{
			if (m_PlayCtrl[i]->IsPlaying())
			{
				bPlaying = true;
			}
		}
		if (bPlaying)
		{
			if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("提示"),
				QStringLiteral("是否退出当前“分段播放”？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
			{
				return;
			}
			StopAll();
		}
	}

	ui.BtnDiv4Time->setStyleSheet("QPushButton{border-image: url(:/Skin/divPlay.png)0 0 0 0 repeat}QPushButton:hover{border-image: url(:/Skin/divPlay.png)28 0 0 0 repeat}QPushButton:pressed{border-image: url(:/Skin/divPlay.png)56 0 0 0 repeat}QPushButton:disabled{border-image: url(: / Skin/divPlay.png)84 0 0 0 repeat}");
	act->setChecked(true);
	if (act == m_Wnd1Act)
	{
		m_WndMode = WND1;
	}
	else if (act == m_Wnd4Act)
	{
		m_WndMode = WND4;
	}
	else if (act == m_Wnd9Act)
	{
		m_WndMode = WND9;
	}
	else if (act == m_Wnd16Act)
	{
		m_WndMode = WND16;
	}

	SetPlayMode(SINGLE_MODE);	
}

void  CDialogMain::OnRenderModeTriggered(QAction*act)
{
	act->setChecked(true);
	if (act == m_renderIniAct)
	{
		m_renderMode = RenderIni;
	}
	else if (act == m_rendeStrechAct)
	{
		m_renderMode = RenderStrech;
	}
	else if (act == m_render4_3Act)
	{
		m_renderMode = Render4_3;
	}
	else if (act == m_rendeStrech16_9Act)
	{
		m_renderMode = Render16_9;
	}

	for (int i = 0; i < NUM; i++)
	{
		m_PlayCtrl[i]->SetRenderMode(m_renderMode);
	}
}

void  CDialogMain::OnPlayModeTriggered(QAction*act)
{
	act->setChecked(true);
	if (act == m_singlePlayAct)
	{
		m_playListMode = SINGLE;
	}
	else if(act == m_listPlayAct)
	{
		m_playListMode = LIST;
	}
	else if (act == m_singleRepeatAct)
	{
		m_playListMode = SINGLE_REPEAT;
	}
	else if (act == m_listRepeatAct)
	{
		m_playListMode = LIST_REPEAT;
	}
}

void CDialogMain::OnBtnMaxClick()
{
	m_bMax = !m_bMax;
	if (m_bMax)
	{
		this->showFullScreen();
	
		//top ..left
		
		ui.BtnMax->setStyleSheet("QPushButton{border-image: url(:/Skin/max.png)0 0 0 30 repeat}QPushButton:hover{border-image: url(:/Skin/max.png)30 0 0 30 repeat}QPushButton:pressed{border-image: url(:/Skin/max.png)60 0 0 30 repeat}QPushButton:disabled{border-image: url(:/Skin/max.png)90 0 0 30 repeat}");
		ui.BtnMax->repaint();

	}
	else
	{
		this->showNormal();

		ui.frame_toolBar->raise();
		ui.BtnMax->setStyleSheet("QPushButton{border-image: url(:/Skin/max.png)0 0 0 0 repeat}QPushButton:hover{border-image: url(:/Skin/max.png)30 0 0 0 repeat}QPushButton:pressed{border-image: url(:/Skin/max.png)60 0 0 0 repeat}QPushButton:disabled{border-image: url(:/Skin/max.png)90 0 0 0 repeat}");
		ui.BtnMax->repaint();
		
	}

}

void CDialogMain::OnBtnMinClick()
{
	this->showMinimized();
}

void CDialogMain::OnBtnMoreClick()
{
	ui.BtnClose->showMenu();
}

void  CDialogMain::ReadPlayFileList()
{
	QString strPlayListXml=QApplication::applicationDirPath() + "/" + FILEPLAYLISTXML;
	QFile file(strPlayListXml);
	QDomDocument doc;
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}

	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	//读文件列表
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		file.close();
		return;
	}

	QDomElement  note = root.firstChildElement();
	while (!note.isNull())
	{
		QString strPathName = note.text();
		int index=strPathName.lastIndexOf("/");
		QString strName = strPathName.mid(index+1);
		
		QListWidgetItem *item = new QListWidgetItem(strName);
		item->setToolTip(strPathName);
		ui.listWidget->addItem(item);

		m_strPlayFileList.append(strPathName);

		note = note.nextSiblingElement();
	}

	file.close();
	
}
void  CDialogMain::WritePlayFileList()
{
	//写播放列表
	QDomDocument doc;

	QDomNode instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	QDomElement root = doc.createElement(ROOTTEXT);
	QDomText nodeText = doc.createTextNode("");
	root.appendChild(nodeText);
	doc.appendChild(root);

	int size = m_strPlayFileList.size();
	for (int i = 0; i < size; i++)
	{
		QDomElement note = doc.createElement(NOTETEXT);
		QDomText nodeText = doc.createTextNode(m_strPlayFileList.at(i));
		note.appendChild(nodeText);
		root.appendChild(note);
	}


	QString strPlayListXml = QApplication::applicationDirPath() + "/" + FILEPLAYLISTXML;
	QFile file(strPlayListXml);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4, QDomNode::EncodingFromTextStream);

	file.close();
}

void  CDialogMain::OnBtnHideListClick()
{
	m_bHide = !m_bHide;

	if (m_bHide)
	{	
		ui.frame_fileList->hide();
		ui.BtnHideList->setToolTip(QStringLiteral("展开播放列表"));
	}
	else
	{
		ui.frame_fileList->show();
		ui.BtnHideList->setToolTip(QStringLiteral("隐藏播放列表"));
	}
	ui.splitter->refresh();//即时刷新窗口大小
	SetPlayMode(m_PlayMode);	
	SetPlayCtrlPos();
	SetBtnHideListPos();
}

void  CDialogMain::SetPlayMode(PLAY_MODE eMode)
{
	m_PlayMode = eMode;
	int left = 4;
	int top = 38;
	
	//int iScreenWidth = QApplication::desktop()->width();
	//int iScreenHeigth = QApplication::desktop()->height();		
	for (int i = 0; i < NUM; i++)
	{
		m_PlayWnd[i]->hide();
		m_PlayCtrl[i]->hide();
	}

	QRect rc = ui.frameLeft->geometry();
	int twidth = rc.width()-6 ;
	int theight = rc.height()-75 ;
	if (m_PlayMode == SINGLE_MODE)
	{	
		if (m_WndMode == WND1)
		{
			m_PlayWnd[0]->setGeometry(left, top, twidth, theight);
			m_PlayWnd[0]->show();
			m_PlayCtrl[0]->SetRenderMode(m_renderMode);
			ui.frameComboBox->move(left + twidth / 2 - 65, top + theight / 2 + 80);
			ui.frameBackGround->move(left + (twidth - 500) / 2, top + (theight - 500) / 2);
			if (!m_PlayCtrl[0]->IsPlaying())
			{
				ui.frameBackGround->show();
				ui.frameBackGround->raise();
				ui.frameComboBox->show();
				ui.frameComboBox->raise();
			}
			m_curWndNum = 1;
		}
		else if (m_WndMode == WND4)
		{
			SetWndGrid(top, left, twidth, theight, 4,2);
		}
		else if(m_WndMode == WND9)
		{
			SetWndGrid(top, left, twidth, theight, 9,3);

		}
		else if (m_WndMode == WND16)
		{
			SetWndGrid(top, left, twidth, theight, 16,4);
		}
		m_PlayCtrl[0]->show();
		m_iCurFocus = 0;
		repaint();
	}
	else if (m_PlayMode == TIME_MODE)
	{
		SetWndGrid(top, left, twidth, theight, 4,2);

		m_PlayCtrl[0]->show();
		repaint();
	}
}

void  CDialogMain::SetWndGrid(int &top, int &left, int &twidth, int &theight, int num,int grid)
{
	int wndDivLine = 2;
	int width = twidth / grid - wndDivLine;
	int height = theight / grid - wndDivLine;
	for (int i = 0; i < num; i++)
	{
		int row = i / grid;//行数
		int col = i % grid;//列数

		m_PlayWnd[i]->setGeometry(left + width*col + wndDivLine*col, top + height*row + wndDivLine*row, width, height);
		m_PlayWnd[i]->show();
		m_PlayCtrl[i]->SetRenderMode(m_renderMode);
	}
	m_curWndNum = num;
	ui.frameComboBox->hide();
	ui.frameBackGround->hide();
}

//进度条设置 10, 630, 700, 100);与m_playMode无关
void  CDialogMain::SetPlayCtrlPos()
{
	int ctrlleft = 0;
	int ctrlheight = 70;

	QRect rc = ui.frameLeft->geometry();
	int ctrltop = 35+rc.height() - 72;
	int ctrlwidth = rc.width();

	for (int i = 0; i < NUM; i++)
	{
		m_PlayCtrl[i]->setGeometry(ctrlleft, ctrltop, ctrlwidth, ctrlheight);
		ui.frame_CtrlBar->setGeometry(ctrlleft, ctrltop, ctrlwidth, ctrlheight);
	}
}

void  CDialogMain::SetBtnHideListPos()
{
	QRect rc=this->geometry();

	int	left1 = rc.width() - ui.frame_fileList->geometry().width()-35;
	int	top1 = rc.height() - 40;
	if (m_bHide)
	{
		left1 = rc.width()-35;
	}
	ui.BtnHideList->move(left1,top1);
}

bool  CDialogMain::OpenAndPlayFile(CFormPlayCtrl *pPlayCtrl, QString strFileName)
{
	bool bRet = false;
	int index = pPlayCtrl->Index();
	if (m_PlayMode == SINGLE_MODE)
	{
		if (pPlayCtrl->IsPlaying())
		{
			pPlayCtrl->Stop();
		}
		pPlayCtrl->SetPlayMode(m_PlayMode);
		
		bRet = pPlayCtrl->OpenAndPlayFile(strFileName, (HWND)m_PlayWnd[index]->GetPlayWndID());
		if (bRet&&m_WndMode == WND1)
		{
			ui.frameComboBox->hide();
			ui.frameBackGround->hide();
		}

		m_PlayWnd[index]->SetTitleFileName(strFileName);
		m_PlayCtrl[index]->SetRenderMode(m_renderMode);
	}
	else if (m_PlayMode == TIME_MODE)
	{
		StopAll();
		index = 0;
		m_PlayCtrl[0]->SetPlayMode(m_PlayMode);
		bRet = m_PlayCtrl[0]->OpenAndPlayFile(strFileName, (HWND)m_PlayWnd[0]->GetPlayWndID());
		m_PlayWnd[0]->SetTitleFileName(strFileName);
		m_PlayCtrl[0]->SetRenderMode(m_renderMode);
	}
	if (!bRet)
	{
		CDialogMessageBox dlg;
		QFile file(strFileName);
		if (!file.exists())
		{
			dlg.SetText1(QStringLiteral("文件路径不存在！"));

		}
		else
		{
			dlg.SetText1(QStringLiteral("无法播放！文件可能被损坏！"));		
		}
		dlg.SetText2(strFileName);
		dlg.exec();
		
		if (m_playListMode == LIST || m_playListMode == SINGLE_REPEAT || m_playListMode == LIST_REPEAT)
		{
			if (m_PlayMode == SINGLE_MODE)
			{
				QApplication::postEvent(this, new CMyEvent(CUSTOM_AutoStop_EVENT, pPlayCtrl));
			}
			else if (m_PlayMode == TIME_MODE)
			{
				QApplication::postEvent(this, new CMyEvent(CUSTOM_AutoStop_EVENT, m_PlayCtrl[0]));
			}
		}
		
	}
	return bRet;
}

void CDialogMain::ShellExe2()
{
	if (!this->isEnabled())//ukey不在线
	{
		return;
	}
	QString strShellFileXml = QApplication::applicationDirPath() + "/" + "shellFile.xml";
	QFile file(strShellFileXml);
	QDomDocument doc;
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}

	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	//读文件列表
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		file.close();
		return;
	}

	QDomElement  note = root.firstChildElement();
	QString strPathName = "";
	while (!note.isNull())
	{
		strPathName = note.text();	
		break;
	}

	file.close();
	ShellExe(strPathName);
}

void CDialogMain::ShellExe(QString &strFile)
{
	if (strFile=="")
	{
		return;
	}
	AddToPlayListWidget(strFile);

	CFormPlayCtrl *pPlayerCtrl = NULL;
	if (m_WndMode == WND1)
	{
		pPlayerCtrl = m_PlayCtrl[0];
	}
	for (int i = 0; i < m_curWndNum; i++)
	{
		if (!m_PlayCtrl[i]->IsPlaying())
		{
			pPlayerCtrl = m_PlayCtrl[i];
			break;
		}
	}

	if (pPlayerCtrl != NULL)
	{
		OpenAndPlayFile(pPlayerCtrl,strFile);
		SetCurPlayListIndex(strFile);
	}
}

void  CDialogMain::StopAll()//分段播放切换到分屏播放，或分屏播放切换到分段播放
{
	for (int i = 0; i < NUM; i++)//这里不能用m_curWndNum了，因为16屏切换到9屏的时候，有些窗口还在播放，只是没显示
	{
		if (m_PlayCtrl[i]->IsPlaying())
		{
			m_PlayCtrl[i]->Stop();
		}
	}
}

void CDialogMain::OnBtnCloseClick()
{	
	if (!m_pVCAMaindlg->Close())
	{
		return;
	}

	s_ukeyThread.Stop();
	s_ukeyThread.wait();

	StopAll();
	ReleasePlayerFactory();
	WritePlayFileList();//写播放列表

	CDialogConfig::WriteConfigXml();
	this->close();
}

void CDialogMain::OnBtnDiv4TimeClick()
{
	if (m_PlayMode == TIME_MODE)
	{
		OnWndModeTriggered(m_Wnd1Act);
		return;
	}

	bool bPlaying = false;
	for (int i = 0; i < NUM; i++)
	{
		if (m_PlayCtrl[i]->IsPlaying())
		{
			bPlaying = true;
		}
	}
	if (bPlaying)
	{
		if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("提示"),
			QStringLiteral("是否退出当前所有播放？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
		{
			return;
		}
		StopAll();
	}
	SetPlayMode(TIME_MODE);
	ui.BtnDiv4Time->setStyleSheet("QPushButton{border-image: url(:/Skin/divPlay.png)0 0 0 28 repeat}QPushButton:hover{border-image: url(:/Skin/divPlay.png)28 0 0 28 repeat}QPushButton:pressed{border-image: url(:/Skin/divPlay.png)56 0 0 28 repeat}QPushButton:disabled{border-image: url(: / Skin/divPlay.png)84 0 0 28 repeat}");
}

void CDialogMain::OnBtnFileDelClick()
{
	int row = ui.listWidget->currentRow();
	if (row < 0)
	{
		return;
	}
	QString strFile = m_strPlayFileList.at(row);
	for (int i = 0; i < NUM; i++)
	{
		if (m_PlayCtrl[i]->IsPlaying())
		{
			if (strFile == m_PlayCtrl[i]->GetCurPlayFileName())
			{
				m_PlayCtrl[i]->Stop();
			}
		}
		
	}
	QListWidgetItem *item = ui.listWidget->takeItem(row);
	delete item;
	m_strPlayFileList.removeAt(row);
}

void CDialogMain::OnBtnFileDelAllClick()
{
	int size = m_strPlayFileList.size();
	if (size == 0)
	{
		return;
	}
	if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("提示"),
		QStringLiteral("确定要清空所有文件，并停止播放？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
	{
		return;
	}
	StopAll();

	ui.listWidget->clear();
	m_strPlayFileList.clear();
}


void CDialogMain::OnActivated(int index)
{
	//CDialogDubbing dlg(this);
	if (QDialog::Accepted == m_DubDlg.exec())
	{
		QString videoFile = m_DubDlg.GetVideoFileName();
		QString AudioFile = m_DubDlg.GetAudioFileName();
		OpenAndPlayFile(m_PlayCtrl[0], videoFile);
		m_PlayCtrl[0]->OpenAudioFile(AudioFile);
	}
}

void CDialogMain::AddToPlayListWidget(QString strPathName)
{
	strPathName.replace("\\", "/");
	int index = strPathName.lastIndexOf("/");

	QString strName = strPathName.mid(index + 1);

	int size = m_strPlayFileList.size();
	if (size != 0)
	{
		for (int i = 0; i < size; i++)
		{
			if (strPathName == m_strPlayFileList.at(i))
			{
				return;
			}
		}
	}

	QListWidgetItem *item = new QListWidgetItem(strName);
	item->setToolTip(strPathName);
	ui.listWidget->addItem(item);

	m_strPlayFileList.append(strPathName);
}

void CDialogMain::OnBtnFileAddClick()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this,
		QStringLiteral("请选择文件"), "", FileFilter);
	int size = fileList.size();
	if (size != 0)
	{
		for (int i = 0; i < size; i++)
		{
			AddToPlayListWidget(fileList.at(i));

		}//只增加到列表，不播放
		SetCurPlayListIndex(fileList.at(0));
	}
}

void CDialogMain::OnBtnFileOpenClick()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this,
		QStringLiteral("请选择文件"), "", FileFilter);
	int size = fileList.size();
	if (size != 0)
	{
		QString strFirstName;
		for (int i = 0; i < size; i++)
		{
			QString strPathName = fileList.at(i);
			if (i == 0)
			{
				strFirstName = strPathName;
			}
			AddToPlayListWidget(strPathName);

		}//
		OpenAndPlayFile(m_PlayCtrl[0],strFirstName);
		SetCurPlayListIndex(strFirstName);
	}
}

void CDialogMain::SetCurPlayListIndex(QString strPathName)
{
	int count = m_strPlayFileList.size();
	for (int i = 0; i < count; i++)
	{
		if (strPathName == m_strPlayFileList.at(i))
		{
			ui.listWidget->setCurrentRow(i);
			break;
		}
	}
}

void CDialogMain::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}
/*
void CDialogMain::dragMoveEvent(QDragMoveEvent *event)
{
//	event->acceptProposedAction();
}
*/
void CDialogMain::dropEvent(QDropEvent * event)
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls())
	{
		QList<QUrl> urlList = mimeData->urls();
		QString strFirstName;
		for (int i = 0; i < urlList.size(); ++i) 
		{
			QString url = urlList.at(i).path();
			QString strPathName = url.mid(1);
			if (i == 0)
			{
				strFirstName = strPathName;
			}
			AddToPlayListWidget(strPathName);
			SetCurPlayListIndex(strPathName);
			
		}
		QPoint pos = event->pos();
		for (int i = 0; i < m_curWndNum; i++)
		{
			QRect rc = m_PlayWnd[i]->geometry();

			if (rc.contains(pos))
			{
				OpenAndPlayFile(m_PlayCtrl[i],strFirstName);
				break;
			}
		}

	}
	
	event->acceptProposedAction();
}

void CDialogMain::OnListWidgetItemDbClick(QListWidgetItem * item)
{
	//int index = ui.listWidget->row(item);
	//OpenAndPlayFile(m_PlayCtrl[m_iCurFocus],m_strPlayFileList.at(index));
	QApplication::postEvent(this, new CMyEvent(CUSTOM_ListItemDbClick_EVENT, (QObject*)item));
}
/*
void CDialogMain::keyPressEvent(QKeyEvent * keyEvent)
{
 	if (keyEvent->key() == Qt::Key_Escape) {
		if (m_bFullScreen)
		{
			SetFullScreen(m_PlayWnd[m_iCurFocus]);
		}
		keyEvent->accept();
		return ;
	}
	else if (keyEvent->key() == Qt::Key_Enter) {
		return ;
	} 
}

void CDialogMain::keyReleaseEvent(QKeyEvent * keyEvent)
{
	keyEvent->accept();
	if (keyEvent->key() == Qt::Key_Escape) {
		if (m_bFullScreen)
		{
			SetFullScreen(m_PlayWnd[m_iCurFocus]);
		}
		return;
	}
	else if (keyEvent->key() == Qt::Key_Enter) {
		return;
	}
}
*/

void  CDialogMain::customEvent(QEvent * e)
{
	if(e->type() == CUSTOM_ListItemDbClick_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		QListWidgetItem *pItem = (QListWidgetItem*)myEvent->obj;
		int index = ui.listWidget->row(pItem);
		OpenAndPlayFile(m_PlayCtrl[m_iCurFocus], m_strPlayFileList.at(index));
		return;
	}
	else if (e->type() == CUSTOM_PLAYWNDPRESSED_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;
		if (pPlayWnd)
		{
			m_iCurFocus = pPlayWnd->Index();
			if (m_PlayCtrl[m_iCurFocus]->IsPlaying())
			{
				QString strFile = m_PlayCtrl[m_iCurFocus]->GetCurPlayFileName();
				SetCurPlayListIndex(strFile);
			}

			for (int i = 0; i<m_curWndNum; i++)
			{
				m_PlayCtrl[i]->hide();
				if (i == m_iCurFocus){
					m_PlayCtrl[i]->show();
					this->repaint();

				}
			}
		}
		return;
	}
	else if (e->type() == CUSTOM_PIC_EVENT)
	{
		CDialogConfig::GetPicParam(m_picPath, m_picType, m_picRepeatCount);
		//判断路径是否存在，不存在则创建，创建不成功返回

		if (!CheckPicPath(m_picPath))
		{
			return ;
		}
		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;
		int index = pPlayWnd->Index();
		QString strName = m_PlayCtrl[index]->GetCurPlayFileName();
		int pos1 = strName.lastIndexOf('/');
		int pos2 = strName.lastIndexOf('.');
		strName = strName.mid(pos1 + 1, pos2 - pos1 - 1);
		QDateTime dateTime = QDateTime::currentDateTime();// m_CapPicRepeatCount
		QString strFilePathName = "";
		CDialogConfig::GetPicParam(m_picPath, m_picType, m_picRepeatCount);
		QString strPicType = ".jpg";
		if (m_picType == 0)
		{
			strPicType = ".bmp";
		}
		strFilePathName = strFilePathName.append(m_picPath).append("/").append(strName).append(dateTime.toString("_yyyyMMddhhmmss").append(strPicType));
		bool bRet = m_PlayCtrl[index]->CapturePic(strFilePathName, m_picType);
		if (bRet)
		{
			CDialogMessageBox dlg(this);
			dlg.SetText1(QStringLiteral("保存成功！"));
			dlg.SetText2(strFilePathName);
			dlg.exec();
		}
		return ;
	}
	else if (e->type() == CUSTOM_PICREPEAT_EVENT)
	{

		CDialogConfig::GetPicParam(m_picPath, m_picType, m_picRepeatCount);
		//判断路径是否存在，不存在则创建，创建不成功返回

		if (!CheckPicPath(m_picPath))
		{
			return ;
		}

		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;

		int index = pPlayWnd->Index();
		QString strName = m_PlayCtrl[index]->GetCurPlayFileName();
		int pos1 = strName.lastIndexOf('/');
		int pos2 = strName.lastIndexOf('.');
		strName = strName.mid(pos1 + 1, pos2 - pos1 - 1);
		QDateTime dateTime = QDateTime::currentDateTime();// 
		QString strTime = dateTime.toString("_yyyyMMddhhmmss");
		QString strPicType = ".jpg";
		if (m_picType == 0)
		{
			strPicType = ".bmp";
		}
		bool bRet = false;
		QString strFileList;
		for (int i = 0; i < m_picRepeatCount; i++)
		{
			QString strFilePathName = "";
			strFilePathName = strFilePathName.append(m_picPath).append("/").append(strName).append(strTime).append("_(%1_%2)").append(strPicType).arg(m_picRepeatCount).arg(i + 1);
			bRet = m_PlayCtrl[index]->CapturePic(strFilePathName, m_picType);
			strFileList.append(strFilePathName);
			strFileList.append("\n");
		}

		if (bRet)
		{
			CDialogMessageBox dlg(this);
			dlg.SetText1(QStringLiteral("保存成功！"));
			dlg.SetText2(strFileList);
			dlg.exec();
		}
		return ;
	}
	else if (e->type() == CUSTOM_PLAYWNDDBCLICK_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;

		SetFullScreen(pPlayWnd);
		return ;
	}
	else if (e->type() == CUSTOM_AutoStop_EVENT || e->type() == CUSTOM_ManualStop_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CFormPlayCtrl *pPlayerCtrl = (CFormPlayCtrl*)myEvent->obj;
		int index = pPlayerCtrl->Index();
		if (m_bFullScreen&&index == m_iCurFocus)
		{
			SetFullScreen(m_PlayWnd[index]);
		}
		if (pPlayerCtrl->IsPlaying())
		{
			pPlayerCtrl->Stop();
		}

		if (m_PlayMode == SINGLE_MODE)
		{
			if (m_WndMode == WND1)
			{
				ui.frameBackGround->show();
				ui.frameBackGround->raise();
				ui.frameComboBox->show();
				ui.frameComboBox->raise();
			}

		}
		else if (m_PlayMode == TIME_MODE)
		{
			bool bPlaying = false;
			for (int i = 0; i<4; i++){
				if (m_PlayCtrl[i]->IsPlaying()){
					bPlaying = true;//分段播放其它还在播放，返回；否则继续下一个播放
					return ;
				}
			}
		}

		if (e->type() == CUSTOM_ManualStop_EVENT)//手动停止不进行下一个播放
		{
			return ;
		}


		/*	if (m_playListMode == SINGLE)//单个播放
		{

		}*/
		if (m_playListMode == LIST)
		{
			PlayIndex(pPlayerCtrl, true);
		}
		else if (m_playListMode == SINGLE_REPEAT)
		{
			QString strFileName = pPlayerCtrl->GetCurPlayFileName();
			OpenAndPlayFile(pPlayerCtrl, strFileName);
		}
		else if (m_playListMode == LIST_REPEAT)
		{
			PlayIndex(pPlayerCtrl, true, true);
		}

		return ;
	}
	else if (e->type() == CUSTOM_PREVIOUS_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CFormPlayCtrl *pPlayerCtrl = (CFormPlayCtrl*)myEvent->obj;
		PlayIndex(pPlayerCtrl, false, false);
		return ;
	}
	else if (e->type() == CUSTOM_NEXT_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CFormPlayCtrl *pPlayerCtrl = (CFormPlayCtrl*)myEvent->obj;
		PlayIndex(pPlayerCtrl, true);
		return ;
	}
	else if (e->type() == CUSTOM_NOTDIV_EVENT)
	{
		QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("文件过短，不支持分段播放！"));
		return ;
	}
	else if (e->type() == CUSTOM_DIV_EVENT)
	{
		CDivEvent *divEvent = (CDivEvent *)e;
		QString strFileName = divEvent->strFileName;
		QString strDivFile = strFileName;
		m_PlayWnd[0]->SetTitleFileName(strDivFile.append(QStringLiteral("_分段%1")).arg(1));
		for (int i = 1; i < 4; i++)//NUM
		{
			m_PlayCtrl[i]->SetPlayMode(m_PlayMode);
			bool ret = m_PlayCtrl[i]->OpenAndPlayFile(strFileName, (HWND)m_PlayWnd[i]->GetPlayWndID());

			strDivFile = strFileName;
			m_PlayWnd[i]->SetTitleFileName(strDivFile.append(QStringLiteral("_分段%1")).arg(i + 1));
			m_PlayCtrl[i]->SetRenderMode(m_renderMode);
		}
		return ;
	}
	QDialog::customEvent(e);
}

bool CDialogMain::eventFilter(QObject *obj, QEvent *e)
{
	//qDebug("e->type()；%d", e->type());
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
		if (keyEvent->key() == Qt::Key_Escape) {//Key_Escape如果在KeyRelease响应的话会直接退出了
			if (m_bFullScreen)
			{
				SetFullScreen(m_PlayWnd[m_iCurFocus]);
			}
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Space) //按钮有下拉菜单栏的，比如右上角扩展功能按钮，按下空格键，只产生 QEvent::KeyPress，菜单栏被打开。屏蔽掉KeyPress后，会产生KeyRelease
		{
			return true;
		}

	//	屏蔽slider对上下左右键的按下的反应，使slider位置不改变，通过SetPlaySliderPos改变.但是
		
		else if (keyEvent->key() == Qt::Key_Left) {
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Right) {
			return true;
		}

		if (!ui.listWidget->hasFocus())
		{
			if (keyEvent->key() == Qt::Key_Up) //
			{
				return true;
			}
			else if (keyEvent->key() == Qt::Key_Down) {
				return true;
			}
		}
	}
	else if (e->type() == QEvent::KeyRelease)//方向键为什么没有QEvent::KeyPress事件，所以要用QEvent::KeyRelease
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
		//	else if (keyEvent->key() == Qt::Key_Enter) {
		//	return true;
		//	}
		 if (keyEvent->key() == Qt::Key_Space) {
			m_PlayCtrl[m_iCurFocus]->OnBtnPauseClick();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Left) {
			m_PlayCtrl[m_iCurFocus]->SetPlaySliderPos(-1);
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Right) {
			m_PlayCtrl[m_iCurFocus]->SetPlaySliderPos(1);
			return true;
		}
		if (!ui.listWidget->hasFocus())
		{
			if (keyEvent->key() == Qt::Key_Up) {
				m_PlayCtrl[m_iCurFocus]->SetVolumeSliderPos(0xffff * 0.1);
				return true;
			}
			else if (keyEvent->key() == Qt::Key_Down) {
				m_PlayCtrl[m_iCurFocus]->SetVolumeSliderPos(-0xffff * 0.1);
				return true;
			}
		}
	}
	else if (e->type() == QEvent::MouseMove)
	{
		if (m_bFullScreen)
		{
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonDblClick)
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
		QRect rc = ui.frameSysBar->geometry();
		QPoint mousePos = ui.frameSysBar->mapFromGlobal(QCursor::pos());

		if (rc.contains(mousePos)){
			OnBtnMaxClick();
			return true;
		}
	}

	return QWidget::eventFilter(obj, e);
}

bool CDialogMain::CheckPicPath(QString picPath)
{
	QDir dir(picPath);
	bool bRet=dir.exists();
	if (bRet)
	{
		return true;
	}
	return dir.mkpath(picPath);
}

void CDialogMain::PlayIndex(CFormPlayCtrl *pPlayerCtrl, bool bNext, bool bListRepeat)
{
	QString strFileName = pPlayerCtrl->GetCurPlayFileName();
	bool bFind = false;
	int index = 0;
	int size = m_strPlayFileList.size();
	if (size != 0)
	{
		for (int i = 0; i < size; i++)
		{
			if (strFileName.compare(m_strPlayFileList.at(i)) == 0)
			{
				bFind = true;
				if (bNext)
				{
					index = i + 1;
				}
				else
				{
					index = i - 1;
				}
				break;
			}
		}
	}
	if (bListRepeat)
	{
		if (index >= size)
		{
			index = 0;
		}
	}
	if (bFind&&index >=0 && index < size)
	{
		OpenAndPlayFile(pPlayerCtrl, m_strPlayFileList.at(index));
		ui.listWidget->setCurrentRow(index);
	}
	else
	{
		//if (m_bPlay)
		//{
		//	Stop();
		//}

	//	ui.labelFileName->setText("");
	//	m_strCurPlayFileName = "";
	}
	
}

void CDialogMain::SetPlayCtrlVisible(bool bShow)
{
	if (!m_bFullScreen)
	{
		return;
	}

	if (bShow)
	{
		ui.frame_CtrlBar->raise();
		m_PlayCtrl[m_iCurFocus]->raise();
		m_PlayCtrl[m_iCurFocus]->show();
		ui.frame_CtrlBar->show();
	}
	else
	{
		m_PlayCtrl[m_iCurFocus]->hide();
		ui.frame_CtrlBar->hide();

	}
}

bool CDialogMain::IsPlayCtrlVisible(int x,int y)
{
	if (!m_bFullScreen)
	{
		return false;
	}
	QRect rc = ui.frame_CtrlBar->geometry();
	
	if (rc.contains(x,y))
	{
		SetPlayCtrlVisible(true);
		return true;
	}
	else
	{
		SetPlayCtrlVisible(false);
		return false;

	}
}

void CDialogMain::SetFullScreen(QWidget *pPlayWnd)
{
	int index = ((CWidgetPlayWnd*)pPlayWnd)->Index();
	if (!m_PlayCtrl[index]->IsPlaying())
	{
		return;
	}
	m_bFullScreen = !m_bFullScreen;
	if (m_bFullScreen)
	{
		m_rcPlayWndNormal = pPlayWnd->geometry();
		m_rcPlayCtrlNormal = m_PlayCtrl[index]->geometry();

		int width = QApplication::desktop()->width();
		int height = QApplication::desktop()->height();
		this->showFullScreen();
		pPlayWnd->setGeometry(0, 0, width, height);
		
		ui.frameSysBar->hide();
	
		ui.frame_toolBar->hide();
		ui.BtnHideList->hide();
		ui.frameBackGround->hide();
		ui.frameComboBox->hide();
	
		ui.frame_fileList->lower();
		pPlayWnd->raise();
		
		m_PlayCtrl[index]->setGeometry(0, height - 70, width, 70);
		ui.frame_CtrlBar->setGeometry(0, height - 70, width, 70);
	//	ui.frame_CtrlBar->raise();
	//	m_PlayCtrl[index]->raise();
		m_PlayCtrl[index]->hide();
		ui.frame_CtrlBar->hide();


	}
	else
	{
		if (m_bMax)
		{
			this->showFullScreen();
		}
		else
		{
			this->showNormal();
		}

		pPlayWnd->setGeometry(m_rcPlayWndNormal);
		m_PlayCtrl[index]->setGeometry(m_rcPlayCtrlNormal);
		ui.frame_CtrlBar->setGeometry(m_rcPlayCtrlNormal);
		m_PlayCtrl[index]->lower();
		ui.frame_CtrlBar->lower();
		
		ui.frameSysBar->show();
		ui.frame_CtrlBar->show();
		ui.frame_toolBar->show();
		ui.BtnHideList->show();

		m_PlayCtrl[index]->show();

	}

	m_PlayCtrl[index]->SetRenderMode(m_renderMode);
}

void CDialogMain::resizeEvent(QResizeEvent  *event)
{
	QWidget::resizeEvent(event);

	QRect rc = this->geometry();
	int windowWidth = rc.width();
	int windowHeight = rc.height();
	ui.frameSysBar->setGeometry(2, 2, windowWidth-4, 32);
	ui.frame_toolBar->move(10, windowHeight - 50);

	ui.splitter->setGeometry(2,32 + 4, windowWidth - 4, windowHeight - 38);
	ui.splitter->refresh();

	SetPlayMode(m_PlayMode);
	SetPlayCtrlPos();
	SetBtnHideListPos();

}

void CDialogMain::OnBtnFullScreenClick()
{
	SetFullScreen(m_PlayWnd[m_iCurFocus]);
}

void CDialogMain::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	//painter.fillRect(this->rect(), QColor(0, 0, 0));
	if (m_PlayMode == SINGLE_MODE&&m_WndMode == WND1)
	{
			QRect rc = m_PlayWnd[0]->geometry();
			rc.adjust(-1, -1, 1, 1);
			painter.setPen(QColor(50, 50, 20));
			painter.drawRect(rc);
	}
	else 
	{
		PaintWndRect(painter, m_curWndNum);
	}
}

void CDialogMain::PaintWndRect(QPainter &painter, int num)
{
	for (int i = 0; i < num; i++)
	{
		QRect rc = m_PlayWnd[i]->geometry();
		rc.adjust(-1, -1, 1, 1);
		painter.setPen(QColor(50, 50, 20));
		painter.drawRect(rc);
	}

	if (m_iCurFocus < NUM&&m_iCurFocus>-1)
	{
		QRect rc = m_PlayWnd[m_iCurFocus]->geometry();
		rc.adjust(-1, -1, 1, 1);
		painter.setPen(QColor(255, 255, 0));
		painter.drawRect(rc);
	}
}

bool CDialogMain::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if (m_bMax)
	{
		return false;
	}
	MSG* msg = (MSG*)message;
	switch (msg->message) {
		
	case WM_NCHITTEST:
	{
		int xPos = GET_X_LPARAM(msg->lParam) - this->frameGeometry().x();
		int yPos = GET_Y_LPARAM(msg->lParam) - this->frameGeometry().y();
		if (this->childAt(xPos, yPos) == 0)
		{
			*result = HTCAPTION;
		}
	//	else{
	//		return false;
	//	}
		*result = HTCLIENT;
		int border = 0;
		int borderWidth = 4;
		if (xPos > 0 && xPos < borderWidth)
			*result = HTLEFT;
		if (xPos >(this->width() - borderWidth) && xPos < (this->width() - border))
			*result = HTRIGHT;
		if (yPos > border && yPos < borderWidth)
			*result = HTTOP;
		if (yPos >(this->height() - borderWidth) && yPos < (this->height() - border))
			*result = HTBOTTOM;
		if (xPos > border && xPos < borderWidth && yPos > border && yPos < borderWidth)
			*result = HTTOPLEFT;
		if (xPos >(this->width() - borderWidth) && xPos < (this->width() - border) && yPos > border && yPos < borderWidth)
			*result = HTTOPRIGHT;
		if (xPos > border && xPos < borderWidth && yPos >(this->height() - borderWidth) && yPos < (this->height() - border))
			*result = HTBOTTOMLEFT;
		if (xPos >(this->width() - borderWidth) && xPos < (this->width() - border) && yPos >(this->height() - borderWidth) && yPos < (this->height() - border))
			*result = HTBOTTOMRIGHT;

	//	
		return true;
	}
	break;
/*	case WM_NCCALCSIZE:
		if (msg->wParam)
		{
			NCCALCSIZE_PARAMS *lpncsp = (NCCALCSIZE_PARAMS*)msg->lParam;
		lpncsp->rgrc[0].left = lpncsp->rgrc[0].left +4;
		lpncsp->rgrc[0].right = lpncsp->rgrc[0].right - 4;
		lpncsp->rgrc[0].top = lpncsp->rgrc[0].top +4;
		lpncsp->rgrc[0].bottom = lpncsp->rgrc[0].bottom - 4;
		lpncsp->rgrc[1] = lpncsp->rgrc[0];
		return true;
	}

	*/
	}
	return false;
}


void  CDialogMain::OnVCATriggered()
{
	m_pVCAMaindlg->StartShow();

	QRect rc = m_pVCAMaindlg->geometry();
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();
	m_pVCAMaindlg->move((iScreenWidth - rc.width()) / 2, (iScreenHeigth - rc.height()) / 2);
}