#pragma once
#include <QDialog>
#include "ui_StreamInfoDlg.h"
class StreamInfoDlg :public QDialog {
	Q_OBJECT
public:
	StreamInfoDlg(QWidget* p = Q_NULLPTR);
	~StreamInfoDlg();

private:
	Ui::StreamInfoDlg ui;
};