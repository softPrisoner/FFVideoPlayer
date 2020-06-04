#include "MyFFmpeg.h"
#include <QtWidgets/QMessageBox>

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"swresample.lib") //重采样

//分子分母转换成Double
static double r2d(AVRational r) {
	return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}
MyFFmpeg::MyFFmpeg() {
	//设置第一个字符为空字符
	m_errbuf[0] = '\0';
	//FFmpeg编程第一句
	av_register_all();
}
//析构函数
MyFFmpeg::~MyFFmpeg() {

}

//打开视频
int MyFFmpeg::Open(const char* path) {
	//关闭上次句柄
	Close();

	m_mutex.lock();
	int nRet = avformat_open_input(&m_afc, path, 0, 0);
	if (nRet != 0) {
		m_mutex.unlock();
		//声明字节数组
		char buf[1024] = { 0 };
		av_strerror(nRet, buf, sizeof(buf));
		return 0;
	}

	//视频的时间，结果是多少毫秒
	m_totalMs = (m_afc->duration / AV_TIME_BASE) * 1000;

	for (int i = 0;i < m_afc->nb_streams;i++) {
		AVCodecContext* acc = m_afc->streams[i]->codec;

		if (acc->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_videoStream = i;
			//FPS (Frame Per Second)每秒传输帧数
			m_fps = r2d(m_afc->streams[i]->avg_frame_rate);
			AVCodec* codec = avcodec_find_decoder(acc->codec_id);
			if (!codec)
			{
				m_mutex.unlock();
				return 0;
			}
			int err = avcodec_open2(acc, codec, NULL);
			if (err != 0) {
				m_mutex.unlock();
				char buf[1024] = { 0 };
				av_strerror(err, buf, sizeof(buf));
				return 0;
			}
		}
		else if (acc->codec_type == AVMEDIA_TYPE_AUDIO) {
			m_audioStream = i;
			AVCodec* codec = avcodec_find_decoder(acc->codec_id);
			if (avcodec_open2(acc, codec, NULL) < 0)
			{
				m_mutex.unlock();
				return false;
			}
			//设置音频的参数:采样频率和通道数
			this->m_sampleRate = acc->sample_rate;
			this->m_sampleSize = acc->channels;
			switch (acc->sample_fmt) {
			case AV_SAMPLE_FMT_S16:
				this->m_sampleSize = 16;
				break;
			case AV_SAMPLE_FMT_S32:
				this->m_sampleSize = 32;
				break;
			default:
				break;
			}
		}

	}
	m_mutex.unlock();
	return m_totalMs;
}

void MyFFmpeg::Close() {
	m_mutex.lock();
	if (m_afc) {
		avformat_close_input(&m_afc);

	}
	if (m_yuv) {
		av_frame_free(&m_yuv);
	}
	//释放资源
	if (m_cCtx)
	{
		sws_freeContext(m_cCtx);
		m_cCtx = NULL;
	}
	if (m_aCtx)
	{	//Check Code
		swr_free(&m_aCtx);
	}
	m_mutex.unlock();
}
string MyFFmpeg::GetError() {
	m_mutex.lock();
	string re = this->m_errbuf;
	m_mutex.unlock();
	return re;
}

AVPacket MyFFmpeg::Read() {
	AVPacket pkt;
	//Set Memory
	memset(&pkt, 0, sizeof(AVPacket));
	m_mutex.lock();
	if (!m_afc)
	{
		m_mutex.unlock();
		return pkt;

	}
	int err = av_read_frame(m_afc, &pkt);
	if (err != 0) {
		av_strerror(err, m_errbuf, sizeof(m_errbuf));
	}
	m_mutex.unlock();
	return pkt;
}

//Decode
int MyFFmpeg::Decode(const AVPacket* pkt) {
	m_mutex.lock();
	if (!m_afc) {
		m_mutex.unlock();
		return NULL;
	}
	//Init yuv
	if (m_yuv == NULL) {
		m_yuv = av_frame_alloc();
	}
	// BUG 这里pcm没有分配内存
	if (m_pcm == NULL)
	{
		m_pcm = av_frame_alloc();
	} 

	AVFrame* frame = m_yuv;
	if (pkt->stream_index == m_audioStream) {
		frame = m_pcm;
	}

	int re = avcodec_send_packet(m_afc->streams[pkt->stream_index]->codec, pkt);

	if (re != 0) {
		m_mutex.unlock();
		return NULL;
	}
	re = avcodec_receive_frame(m_afc->streams[pkt->stream_index]->codec, frame);
	if (re != 0)
	{
		m_mutex.unlock();
		return NULL;
	}
	m_mutex.unlock();
	//Set timebase for point
	int p = (frame->pts * r2d(m_afc->streams[pkt->stream_index]->time_base))*1000;
	if (pkt->stream_index == m_audioStream) {
		this->m_pts = p;
	}
	return m_pts;
}

bool MyFFmpeg::ToRGB(char* out, int outweight, int outheight) {
	m_mutex.lock();
	//Check AVFrameContext|YUV
	if (!m_afc | !m_yuv) {
		m_mutex.unlock();
		return false;
	}

	AVCodecContext* videoCtx = m_afc->streams[this->m_videoStream]->codec;
	m_cCtx = sws_getCachedContext(m_cCtx, videoCtx->width, videoCtx->height,
		videoCtx->pix_fmt,
		outweight, outheight,
		AV_PIX_FMT_BGRA, //Output Format
		SWS_BICUBIC,
		NULL, NULL, NULL
	);
	if (!m_cCtx)
	{
		m_mutex.unlock();
		return false;
	}
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	data[0] = (uint8_t*)out;
	int linesize[AV_NUM_DATA_POINTERS] = { 0 };
	//Every Transfer Code Line Size
	linesize[0] = outweight * 4;
	int h = sws_scale(m_cCtx, m_yuv->data, m_yuv->linesize, 0, videoCtx->height, data, linesize);
	m_mutex.unlock();
	return true;
}
int MyFFmpeg::ToPCM(char* out) {
	m_mutex.lock();
	if (!m_afc || !m_pcm || !out) {
		m_mutex.unlock();
		return 0;
	}
	AVCodecContext* ctx = m_afc->streams[m_audioStream]->codec;
	if (m_aCtx == NULL) {
		m_aCtx = swr_alloc();
		//可能会出现问题，16位音频
		swr_alloc_set_opts(m_aCtx, ctx->channel_layout,
			AV_SAMPLE_FMT_S16,
			ctx->sample_rate,
			ctx->channels,
			ctx->sample_fmt,
			ctx->sample_rate,
			0, 0
		);
		swr_init(m_aCtx);
	}

	uint8_t* data[1];
	data[0] = (uint8_t*)out;
	int len = swr_convert(m_aCtx, data, 10000, (const uint8_t**)m_pcm->data, m_pcm->nb_samples);
	if (len <= 0)
	{
		m_mutex.unlock();
		return 0;
	}
	int outsize = av_samples_get_buffer_size(NULL, ctx->channels, m_pcm->nb_samples, AV_SAMPLE_FMT_S16, 0);

	//音量调节
	SetVolume(out, outsize, 1, m_VolumeRatio);

	m_mutex.unlock();

	return outsize;

}

bool MyFFmpeg::Seek(float pos) {
	m_mutex.lock();
	if (!m_afc)
	{
		m_mutex.unlock();
		return false;
	}
	int64_t stamp = 0;
	stamp = pos * m_afc->streams[m_videoStream]->duration;

	//向后|关键帧
	int re = av_seek_frame(m_afc, m_videoStream, stamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

	//清除之前的解码缓冲
	avcodec_flush_buffers(m_afc->streams[m_videoStream]->codec);
	m_mutex.unlock();

	if (re >= 0)
	{
		return true;
	}

	return false;
}

int MyFFmpeg::GetPts(const AVPacket* pkt) {
	m_mutex.lock();
	if (!m_afc) {
		m_mutex.unlock();
		return -1;
	}
	int pts = (pkt->pts * r2d(m_afc->streams[pkt->stream_index]->time_base)) * 1000;//毫秒数
	m_mutex.unlock();
	return pts;
}
/*
*@Func SetVolume
*@brief 音量调节
*@author Tony Li
*@[in] :buf 需要调节音量的音频数据的首地址指针
*@[in] uRepeat为重复次数，通常设置为1
*@[in] vol为增益倍数，可以小于1
*/
void MyFFmpeg::SetVolume(char* buf, UINT32 size, UINT32 uRepeat, double vol) {
	if (!size) {
		return;
	}
	for (int i = 0;i < size;i += 2) {
		short wData;
		wData = MAKEWORD(buf[i], buf[i + 1]);
		long dwData = wData;

		for (int j = 0;j < uRepeat;j++) {
			dwData = dwData * vol;
			if (dwData < -0x8000)
			{
				dwData = -0x8000;
			}
			else if (dwData > 0x7FFF) {
				dwData = 0x7FFF;
			}
		}

		wData = LOWORD(dwData);
		buf[i] = LOBYTE(dwData);
		buf[i + 1] = HIBYTE(wData);
	}
}



