/*

*/
#pragma once
#include <QThread>
#include <QImage>

class NetStreamThread :public QThread {
	Q_OBJECT
public:
	//单例方法
	static NetStreamThread* getInstance();
	~NetStreamThread();
	void startPlay(QString url);
	//运行线程
	void run();

signals:
	//获取视频中的一帧
	void sig_GetOneFrame(QImage);
private:
	NetStreamThread();
	//内部类 进行垃圾回收
	class GC {
	public:
		~GC() {
			if (m_Net != NULL) {
				NetStreamThread::getInstance()->terminate();
				delete m_Net;
				m_Net = NULL;
			}
		}

	};
private:
	QString m_Url;
	static GC m_gc;
	static NetStreamThread* m_Net;
};