#include "AudioPlay.h"
#include <QtMultimedia/QAudioOutput>
#include <QtCore/QMutex>

void AudioPlay::Stop() {
	mutex.lock();
	//判断是否有音频输出
	if (m_AudioOutput) {
		//停止播放音频
		m_AudioOutput->stop();
		//清除内存占用
		delete m_AudioOutput;
		m_AudioOutput = NULL;
		io = NULL;
	}
	mutex.unlock();
}

bool AudioPlay::Start() {
	//首先关闭上一个线程 
	Stop();
	mutex.lock();
	//Qt的音频格式采集
	QAudioFormat fmt;
	//设置音频采样率48000
	fmt.setSampleRate(this->sampleRate);
	//设置音频采样大小  16bit
	fmt.setSampleSize(this->sampleSize);
	//设置声道数量
	fmt.setChannelCount(this->channel);
	//设置编码格式为PCM裸流
	fmt.setCodec("audio/pcm");
	//设置字节序
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	//样本的类别
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	//Qt音频输出
	m_AudioOutput = new QAudioOutput(fmt);

	//设置启动IO
	io = m_AudioOutput->start();
	mutex.unlock();
	return true;
}

/*
 播放声音
*/
void AudioPlay::Play(bool isPlay) {
	mutex.lock();
	//
	if (!m_AudioOutput) {
		mutex.unlock();
		return;
	}

	//是否播放
	if (isPlay) {
		m_AudioOutput->resume();
	}
	else {
		//延迟播放
		m_AudioOutput->suspend();
	}
	mutex.unlock();
}

/*
写入事件
*/
bool AudioPlay::Write(const char* data, int datasize) {
	//如果没有数据，数据大小
	if (!data || datasize <= 0) {
		return false;
	}

	mutex.lock();
	if (io) {
		mutex.unlock();
		//通过io写数据
		io->write(data, datasize);
		return true;
	}
	mutex.unlock();
	return false;
}

//获取释放
int AudioPlay::GetFree() {
	mutex.lock();
	//如果音频输出为空
	if (!m_AudioOutput) {
		mutex.unlock();
		return 0;
	}

	//字节释放
	int free = m_AudioOutput->bytesFree();
	mutex.unlock();
	return free;
}

//设置音量
void AudioPlay::SetVolume(int value) {
	mutex.lock();
	//设置音量
	m_AudioOutput->setVolume(value);
	mutex.unlock();
}

AudioPlay* AudioPlay::Get() {
	static AudioPlay ap;
	return &ap;
}