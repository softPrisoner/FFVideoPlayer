#pragma once
#include <QtWidgets/QSlider>
#include <QtGui/QMouseEvent>

class MySlider :public QSlider {
	Q_OBJECT
public:
	MySlider(QWidget* p = NULL);

	~MySlider();

	//��갴���¼�
	void mousePressEvent(QMouseEvent* e);
};