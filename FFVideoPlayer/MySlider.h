#pragma once
#include <QtWidgets/QSlider>
#include <QtGui/QMouseEvent>

class MySlider :public QSlider {
	Q_OBJECT
public:
	MySlider(QWidget* p = NULL);

	~MySlider();

	//鼠标按下事件
	void mousePressEvent(QMouseEvent* e);
};