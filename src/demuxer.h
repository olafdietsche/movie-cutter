// -*- mode: c++ -*-
#ifndef __demuxer_h_included__
#define __demuxer_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include <deque>
#include <functional>
#include <string>

class demuxer {
public:
	demuxer();
	~demuxer();

	AVFormatContext *get_format_context() { return fmt_ctx_; }
	const AVFormatContext *get_format_context() const { return fmt_ctx_; }
	int get_stream_index(AVMediaType codec_type) {
		return av_find_best_stream(fmt_ctx_, codec_type, -1, -1, NULL, 0);
	}
	AVStream *get_stream(int i) { return fmt_ctx_->streams[i]; }
	static int64_t normalize_timestamp(AVStream *st, int64_t ts);
	static int64_t start_timestamp(AVStream *st);
	static int64_t rescale_timestamp(AVStream *st, int64_t ts);
	int64_t rescale_timestamp(int stream_index, int64_t ts) {
		AVStream *st = get_stream(stream_index);
		return rescale_timestamp(st, ts);
	}
	static std::string format_timestamp(int64_t ts);
	static int64_t ticks_per_frame(AVStream *st);

	int open_input(const char *filename);
	void close();

	int read_next_packet(AVPacket *pkt);
	int read_next_packet(AVPacket *pkt, int stream_index);
	AVFrame *decode_packet(AVCodecContext *dec_ctx, AVPacket *pkt);
	void flush(int stream_index);

	int seek(int stream_index, int64_t pts);
private:
	demuxer(const demuxer &);
	demuxer &operator=(const demuxer &);

	AVFormatContext *fmt_ctx_;
	AVFrame *frame_;
};

#endif
