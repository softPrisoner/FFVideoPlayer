#pragma once
#include <QtMultimedia/QAudioOutput>
#include <QtCore/Qmutex>

class AudioPlay {

public:
	static AudioPlay* Get();

	//������Ƶ���
	QAudioOutput* m_AudioOutput = NULL;

	//�����ȡ�豸�߳�
	QIODevice* io = NULL;

	//������
	QMutex mutex;

	//���ò�����HZ
	int sampleRate = 48000;
	//������С
	int sampleSize = 16;
	//������
	int channel = 2;
public:
	//ֹͣ����
	void Stop();

	//��ʼ����
	bool Start();

	//����
	void Play(bool isPlay);

	//д����
	bool Write(const char* data, int datasize);

	int GetFree();
	//��������
	void SetVolume(int value);

	//���캯��˽�л�����ֹ�ⲿ����
private:
	AudioPlay() {};
};