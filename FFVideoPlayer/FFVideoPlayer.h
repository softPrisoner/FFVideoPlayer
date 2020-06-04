#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FFVideoPlayer.h"
#include <QMediaPlayer>
#include "NetStreamDlg.h"

class FFVideoPlayer : public QMainWindow
{
	Q_OBJECT
public:
	FFVideoPlayer(QWidget* parent = Q_NULLPTR);
	void timerEvent(QTimerEvent* e);
	void closeEvent(QCloseEvent* e);
	void resizeEvent(QResizeEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* ); //双击
	void mousePressEvent(QMouseEvent* event);
private:
	void fullShow();
	void normalShow();
	void initUI();
	void allConnect();
	void setButtonBackImage(QPushButton* button, QString image);


private slots:
	void slotPlay(); //播放与暂停
	void slotSliderPressed();
	void slotSliderReleased();
	void on_btnFullScreen_clicked();
	void setMute(); //静音设置

	//菜单
	void OpenLocalVideo();
	void OpenNetStreamDlg();
	void PopAboutDlg();
	void GetSourceCode();

	//音量调节
	void volumeAdjust();

	//网络流
	void slotPushStream(QString address);
private:
	Ui::FFVideoPlayerClass ui;
	QMediaPlayer* player;
	//视频总视频
	int m_Hour;
	int m_Min;
	int m_Second;

	NetStreamDlg m_NetDlg;
	bool m_isMute;
};
