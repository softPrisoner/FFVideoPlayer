#pragma once
#include <iostream>
#include <string>
#include <QtCore/QMutex>

extern "C" {
	//libavformat 
#include <libavformat/avformat.h>
	//libswscale 
#include <libswscale/swscale.h>
	//libswresample
#include <libswresample/swresample.h>
}

#include <Windows.h>
using namespace std;
/*
/*
/*
ffmpeg  打开视频，音视频解码，资源释放等;
*/
class MyFFmpeg {
	//静态单例
public:
	static MyFFmpeg* Get() {
		static MyFFmpeg ff;
		return &ff;
	}

	int Open(const char* path);

	void Close();

	int Decode(const AVPacket* pkt);

	AVPacket Read();

	bool ToRGB(char* out, int outweight, int outheight);

	int ToPCM(char* out);

	bool Seek(float pos);

	int GetPts(const AVPacket* pkt);

	void SetVolume(char* buf, UINT32 size, UINT32 uRepeat, double vol); //设置音量

	string GetError();

	virtual ~MyFFmpeg();

public:
	int m_totalMs;
	int m_videoStream = 0;
	int m_fps = 0;
	int m_pts = 0;
	boolean m_isPlay = false;
	int m_audioStream = 1;

	int m_sampleRate = 48000;
	int m_sampleSize = 16;
	int m_channel = 2;

	double m_VolumeRatio = 1.00; //音量比例

protected:
	MyFFmpeg();

	char m_errbuf[1024];
	QMutex m_mutex;//线程同步
	AVFormatContext* m_afc;
	AVFrame* m_yuv = NULL;
	AVFrame* m_pcm = NULL; //存放解码后的音频
	SwsContext* m_cCtx = NULL; //转换器
	SwrContext* m_aCtx = NULL;	//音频

};

