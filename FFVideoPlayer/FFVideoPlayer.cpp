#include "FFVideoPlayer.h"
#include <QFileDialog>
#include "MyFFmpeg.h"
#include <QMessageBox>
#include "AudioPlay.h"
#include <string>
#include <iostream>
#include "VideoThread.h"
#include <QCursor>
#include "AboutDlg.h"
#include "NetStreamThread.h"
#include "MyLog.h"
#include <QDesktopServices>
#include <qevent.h>

using namespace std;
//全局变量只在该cpp中有效
static bool ispressSlider = false;
//是否播放
static bool g_isPlay = true;

//网络流
int g_NetStream;

FFVideoPlayer::FFVideoPlayer(QWidget* parent)
	: QMainWindow(parent)
{
	//设置主窗口
	ui.setupUi(this);
	//初始化时间标记
	m_Hour = 0;
	//设置分钟
	m_Min = 0;
	//设置秒数
	m_Second = 0;
	//是否静音，默认外放声音
	m_isMute = false;
	//最大化显示
	showMaximized();
	//设置窗口最小尺寸
	setMinimumSize(QSize(800, 512));

	LOG4CPLUS_INFO(MyLog::getInstance()->logger, "Program Start,init UI");
	initUI();

#ifdef _DEBUG
	setWindowTitle("FFVideoPlayer_D");
#else
	setWindowTitle("FFVideoPlayer");
#endif // _DEBUG
	startTimer(40);
	allConnect();
}

void FFVideoPlayer::initUI() {
	//MainWindow背景色
	setStyleSheet("background-color:rgb(53,53,53);");
	//菜单颜色
	ui.menuBar->setStyleSheet("background-color:rgb(53,53,53);border:1px solid gray; color:white;padding:1px;");

	//设置播放时间
	ui.label_Playtime->clear();
	//设置播放时间初始显示
	ui.label_Playtime->setText("00:00:00/00:00:00");

	//按钮贴图
	setButtonBackImage(ui.btnPlayVideo, "./Resources/play.png");
	//设置音量
	setButtonBackImage(ui.btnSetVolume, "./Resources/volume.png");
	//设置背景图片
	setButtonBackImage(ui.btnFullScreen, "./Resources/fullscreen.png");
	//设置重置图片
	setButtonBackImage(ui.btnCutImage, "./Resources/cutimage.png");
}

void FFVideoPlayer::allConnect() {
	//设置播放按钮事件
	connect(ui.btnPlayVideo, SIGNAL(clicked()), this, SLOT(slotPlay()));
	//设置时间滑动 按下事件
	connect(ui.timerSlider, SIGNAL(sliderPressed()), this, SLOT(slotSliderPressed()));
	//设置时间滑动 释放事件
	connect(ui.timerSlider, SIGNAL(sliderReleased()), this, SLOT(slotSliderReleased()));

	//打开本地文件事件
	connect(ui.action_OpenLocalFiles, SIGNAL(triggered()), this, SLOT(OpenLocalVideo()));
	//打开网络流事件
	connect(ui.action_OpenNetStream, SIGNAL(triggered()), this, SLOT(OpenNetStreamDlg()));
	//设置关于软件事件
	connect(ui.action_About, SIGNAL(triggered()), this, SLOT(PopAboutDlg()));
	//设置获取源代码事件
	connect(ui.action_GetSourceCode, SIGNAL(triggered()), this, SLOT(GetSourceCode()));

	//音量调节
	connect(ui.sliderVolume, SIGNAL(sliderReleased()), this, SLOT(volumeAdjust()));
	//设置静音
	connect(ui.btnSetVolume, SIGNAL(clicked()), this, SLOT(setMute()));

	connect(&m_NetDlg, SIGNAL(PushStream(QString)), this, SLOT(slotPushStream(QString)));
}

/*
*打开本地视频
*/
void FFVideoPlayer::OpenLocalVideo() {
	LOG4CPLUS_DEBUG(MyLog::getInstance()->logger, "Open Local Video File");
	QString filename = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择视频文件"));
	//LOG4CPLUS_DEBUG(MyLog::getInstance()->logger, "filename:" + filename);
	//文件名称是否为空
	if (filename.isEmpty()) {
		return;
	}

	//视频路径
	string videoPath = string((const char*)filename.toLocal8Bit());//QString 转换成8 Bit String
	int n1 = videoPath.find_last_of('/');
	int n2 = videoPath.find_last_of('\0');
	//截取视频名称
	string sname = videoPath.substr(n1 + 1, n2);

	QString qstr = QString(QString::fromLocal8Bit(sname.c_str()));//string转QString
	this->setWindowTitle(qstr);

	//总时间
	int totalMs = MyFFmpeg::Get()->Open(filename.toLocal8Bit());
	if (totalMs <= 0) {
		QMessageBox::information(this, "err", "file open failed!");
		LOG4CPLUS_INFO(MyLog::getInstance()->logger, "Open Local Video File Failed");
		return;
	}

	AudioPlay::Get()->sampleRate = MyFFmpeg::Get()->m_sampleRate;
	AudioPlay::Get()->channel = MyFFmpeg::Get()->m_channel;
	AudioPlay::Get()->sampleSize = 16;

	AudioPlay::Get()->Start();

	char buf[1024] = { 0 };
	int seconds = totalMs / 1000;
	m_Hour = seconds / 3600;
	m_Min = (seconds - m_Hour * 3600) / 60;
	m_Second = seconds - m_Hour * 3600 - m_Min * 60;

	g_isPlay = false;
	slotPlay();
}

/*
*@brief 定时器事件
*@param e
*/
void FFVideoPlayer::timerEvent(QTimerEvent* e) {
	int playseconds = MyFFmpeg::Get()->m_pts / 1000;
	int hour = playseconds / 3600;
	int min = (playseconds - hour * 3600) / 60;
	int second = playseconds - hour * 3600 - min * 60;

	char buf[1024] = { 0 };
	sprintf(buf, "%02d:%02d:%02d/%02d:%02d:%02d", hour, min, second, m_Hour, m_Min, m_Second);
	this->ui.label_Playtime->setText(buf);
	if (MyFFmpeg::Get()->m_totalMs > 0)
	{
		float rate = MyFFmpeg::Get()->m_pts / (float)MyFFmpeg::Get()->m_totalMs;
		//只有按下了，才显示进度条
		if (!ispressSlider) {
			this->ui.timerSlider->setValue(rate * 1000);
		}

		/*	if (ui.timerSlider->geometry().contains(this->mapFromGlobal(QCursor::pos())))
			{
				ui.timerSlider->show();
				ui.timerSlider->setValue(rate * 1000);

			}
			else {
				ui.timerSlider->hide();
			}*/

	}
}

/*
*@brief 响应进度按下的信号通知
*/
void FFVideoPlayer::slotSliderPressed() {
	ispressSlider = true;
}

/*
 *@breif 响应鼠标松开进度条的信号
 */
void FFVideoPlayer::slotSliderReleased() {
	ispressSlider = false;
	float pos = 0;
	pos = this->ui.timerSlider->value() / static_cast<float>(ui.timerSlider->maximum() + 1);//从0开始的，不能让分母为0

	MyFFmpeg::Get()->Seek(pos);
}

/*
*@brief 播放视频
*/
void FFVideoPlayer::slotPlay() {
	g_isPlay = !g_isPlay;
	MyFFmpeg::Get()->m_isPlay = g_isPlay;

	if (g_isPlay) {
		setButtonBackImage(ui.btnPlayVideo, "./Resources/stop.png");
	}
	else {
		setButtonBackImage(ui.btnPlayVideo, "./Resources/play.png");
	}
}

/*
*@brief 窗口关闭时的操作
@param e
*/
void FFVideoPlayer::closeEvent(QCloseEvent* e) {
	//结束线程
	VideoThread::Get()->terminate();
}


//窗口大小发生变化时，各控件大小的位置及大小的设置
void FFVideoPlayer::resizeEvent(QResizeEvent* e) {
	/*if (isFullScreen()) {
		ui.openGLWidget->move(0, 0);
		ui.openGLWidget->resize(this->width(), this->height());
	}
	else {
		ui.openGLWidget->move(0, 0);
		ui.openGLWidget->resize(this->width(), this->height() - 62);

		ui.timerSlider->move(0, this->height() - 80);
		ui.timerSlider->resize(this->width(), 20);
		ui.label_Playtime->move(5, this->height() - 60);
		ui.btnPlayVideo->move(this->width() / 2 - 15, this->height() - 60);
		ui.btnSetVolume->move(this->width() * 4 / 5, this->height() - 60);
		ui.sliderVolume->move(this->width() * 4 / 5 + 31, this->height() - 60);
		ui.btnFullScreen->move(this->width() - 40, this->height() - 60);
		ui.btnCutImage->move(this->width() * 4 / 5 - 40, this->height() - 60);
	}*/
}

/*
*@brief FFVideoPlayer::on_fullScreenBtn_clicked
*全屏按钮
*/
void FFVideoPlayer::on_btnFullScreen_clicked() {
	if (!isFullScreen()) {
		fullShow();
	}
}

/*
*@brief FFVideoPlayer::mouseDoubleClickEvent
*/
void FFVideoPlayer::mouseDoubleClickEvent(QMouseEvent*) {
	if (this->isFullScreen()) {
		normalShow();
	}
	else {
		fullShow();
	}
}

/*
*@brief 全屏时各个按钮隐藏
*/
void FFVideoPlayer::fullShow() {
	this->showFullScreen();

	ui.menuBar->hide();
	ui.timerSlider->hide();
	ui.label_Playtime->hide();
	ui.btnPlayVideo->hide();
	ui.btnSetVolume->hide();
	ui.sliderVolume->hide();
	ui.btnFullScreen->hide();
	ui.btnCutImage->hide();
}

/*
*@breif 正常显示时各个控件均显示
*/
void FFVideoPlayer::normalShow() {
	this->showNormal();
	ui.menuBar->show();
	ui.timerSlider->show();
	ui.label_Playtime->show();
	ui.btnPlayVideo->show();
	ui.btnSetVolume->show();
	ui.sliderVolume->show();
	ui.btnFullScreen->show();
	ui.btnCutImage->show();
}

//菜单--打开网络流
void FFVideoPlayer::OpenNetStreamDlg() {
	//NetStreamDlg nDlg;
	m_NetDlg.exec();
}

void FFVideoPlayer::PopAboutDlg() {
	AboutDlg dlg;
	dlg.exec();
}

//调节音量
void FFVideoPlayer::volumeAdjust() {
	MyFFmpeg::Get()->m_VolumeRatio = (ui.sliderVolume->value()) * 1.00 / 100;
}

//静音设置
void FFVideoPlayer::setMute() {
	m_isMute = !m_isMute;
	if (m_isMute) {
		MyFFmpeg::Get()->m_VolumeRatio = 0;
		setButtonBackImage(ui.btnSetVolume, "./Resources/mute.png");
	}
	else {
		MyFFmpeg::Get()->m_VolumeRatio = 0.5;
		setButtonBackImage(ui.btnSetVolume, "./Resources/volume.png");
	}
}

//设置按钮图标，按钮的默认大小 30*30
void FFVideoPlayer::setButtonBackImage(QPushButton* button, QString image) {
	QPixmap pixmap(image);
	QPixmap fitpixmap = pixmap
		.scaled(30, 30)
		.scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	button->setIcon(QIcon(fitpixmap));
	button->setIconSize(QSize(30, 30));
	button->setFlat(true); //按钮透明
	button->setStyleSheet("border:0px"); //消除边框
}

//设置推流
void FFVideoPlayer::slotPushStream(QString address) {
	LOG4CPLUS_INFO(MyLog::getInstance()->logger, "Push Net Stream");
	g_NetStream = 1;
	setWindowTitle(address);
	NetStreamThread::getInstance()->startPlay(address);
}

//获取源代码
void FFVideoPlayer::GetSourceCode() {
	QDesktopServices::openUrl(QUrl(QString("http://www.baidu.com")));
}

//鼠标右键事件
void FFVideoPlayer::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::RightButton) {
		QMenu menu;
		QAction* ac1 = menu.addAction(QString::fromLocal8Bit("打开本地视频"));
		QAction* ac2 = menu.addAction(QString::fromLocal8Bit("查看视频信息"));
		connect(ac1, SIGNAL(triggered()), this, SLOT(OpenLocalVideo()));
		menu.exec(QCursor::pos());
	}
}
