#include "AudioPlay.h"
#include <QtMultimedia/QAudioOutput>
#include <QtCore/QMutex>

void AudioPlay::Stop() {
	mutex.lock();
	//�ж��Ƿ�����Ƶ���
	if (m_AudioOutput) {
		//ֹͣ������Ƶ
		m_AudioOutput->stop();
		//����ڴ�ռ��
		delete m_AudioOutput;
		m_AudioOutput = NULL;
		io = NULL;
	}
	mutex.unlock();
}

bool AudioPlay::Start() {
	//���ȹر���һ���߳� 
	Stop();
	mutex.lock();
	//Qt����Ƶ��ʽ�ɼ�
	QAudioFormat fmt;
	//������Ƶ������48000
	fmt.setSampleRate(this->sampleRate);
	//������Ƶ������С  16bit
	fmt.setSampleSize(this->sampleSize);
	//������������
	fmt.setChannelCount(this->channel);
	//���ñ����ʽΪPCM����
	fmt.setCodec("audio/pcm");
	//�����ֽ���
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	//���������
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	//Qt��Ƶ���
	m_AudioOutput = new QAudioOutput(fmt);

	//��������IO
	io = m_AudioOutput->start();
	mutex.unlock();
	return true;
}

/*
 ��������
*/
void AudioPlay::Play(bool isPlay) {
	mutex.lock();
	//
	if (!m_AudioOutput) {
		mutex.unlock();
		return;
	}

	//�Ƿ񲥷�
	if (isPlay) {
		m_AudioOutput->resume();
	}
	else {
		//�ӳٲ���
		m_AudioOutput->suspend();
	}
	mutex.unlock();
}

/*
д���¼�
*/
bool AudioPlay::Write(const char* data, int datasize) {
	//���û�����ݣ����ݴ�С
	if (!data || datasize <= 0) {
		return false;
	}

	mutex.lock();
	if (io) {
		mutex.unlock();
		//ͨ��ioд����
		io->write(data, datasize);
		return true;
	}
	mutex.unlock();
	return false;
}

//��ȡ�ͷ�
int AudioPlay::GetFree() {
	mutex.lock();
	//�����Ƶ���Ϊ��
	if (!m_AudioOutput) {
		mutex.unlock();
		return 0;
	}

	//�ֽ��ͷ�
	int free = m_AudioOutput->bytesFree();
	mutex.unlock();
	return free;
}

//��������
void AudioPlay::SetVolume(int value) {
	mutex.lock();
	//��������
	m_AudioOutput->setVolume(value);
	mutex.unlock();
}

AudioPlay* AudioPlay::Get() {
	static AudioPlay ap;
	return &ap;
}