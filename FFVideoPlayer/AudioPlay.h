#pragma once
#include <QtMultimedia/QAudioOutput>
#include <QtCore/Qmutex>

class AudioPlay {

public:
	static AudioPlay* Get();

	//定义音频输出
	QAudioOutput* m_AudioOutput = NULL;

	//定义读取设备线程
	QIODevice* io = NULL;

	//互斥锁
	QMutex mutex;

	//设置采样率HZ
	int sampleRate = 48000;
	//采样大小
	int sampleSize = 16;
	//声道数
	int channel = 2;
public:
	//停止播放
	void Stop();

	//开始播放
	bool Start();

	//播放
	void Play(bool isPlay);

	//写数据
	bool Write(const char* data, int datasize);

	int GetFree();
	//设置音量
	void SetVolume(int value);

	//构造函数私有化，防止外部访问
private:
	AudioPlay() {};
};