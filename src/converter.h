#ifndef __converter_h_included__
#define __converter_h_included__

extern "C" {
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

class converter {
public:
	converter(AVStream *st, AVPixelFormat to_pix_fmt);
	~converter();
	AVFrame *convert_frame(AVFrame *src);

private:
	struct SwsContext *sws_ctx_;
	uint8_t *data_;
	AVFrame *dest_;

	converter();
	converter(const converter &);
	converter &operator=(const converter &);
};

#endif
