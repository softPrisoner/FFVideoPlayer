/*
关于软件对话框
*/

#include "AboutDlg.h"

AboutDlg::AboutDlg(QWidget* parent) :QDialog(parent) {
	ui.setupUi(this);
	//设置窗口大小
	setFixedSize(QWidget::width(), QWidget::height());
}

AboutDlg::~AboutDlg() {

}