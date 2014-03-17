#ifndef __demuxer_h_included__
#define __demuxer_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include <functional>

class demuxer {
public:
	demuxer(const char *filename);
	~demuxer();

	int open(const char *filename);
	void close();
	//AVFrame *get_next_frame();
	typedef std::function<bool(AVStream*, AVPacket*)> Filter;
	typedef std::function<bool(AVCodecContext*, AVFrame*)> Decoder;
	void frame_loop(Filter f, Decoder d);

private:
	demuxer();
	demuxer(const demuxer &);
	demuxer &operator=(const demuxer &);

	bool decode_packet(AVCodecContext *dec_ctx, AVPacket *pkt);

	AVFormatContext *fmt_ctx_;
	AVFrame *frame_;
};

#endif
