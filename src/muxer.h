// -*- mode: c++ -*-
#ifndef __muxer_h_included__
#define __muxer_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

class muxer {
public:
	muxer() : fmt_ctx_(0) {}
	muxer(const char *filename, const AVFormatContext *fmt_ctx_in)
		: fmt_ctx_(0) { open_output(filename, fmt_ctx_in); }
	~muxer() { close(); }

	int open_output(const char *filename, const AVFormatContext *fmt_ctx_in);
	void close();
	int write_packet(AVPacket *pkt);
private:
	AVFormatContext *fmt_ctx_;

	muxer(const muxer &);
	muxer &operator=(const muxer &);
};

#endif
