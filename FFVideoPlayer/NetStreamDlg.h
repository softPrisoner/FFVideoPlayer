#pragma once
#include <QDialog>
#include  <ui_NetStreamDlg.h>

class NetStreamDlg : public QDialog {
	Q_OBJECT
public:
	//没有构造的函数报错，需要参数初始化
	//NetStreamDlg();
	NetStreamDlg(QWidget* p = Q_NULLPTR);
	~NetStreamDlg();
	void closeEvent(QCloseEvent* e);
signals:
	void PushStream(QString address);
private:
	Ui::NetStreamDlg ui;
	QString m_iniPath;
	QString m_iniKey;
};