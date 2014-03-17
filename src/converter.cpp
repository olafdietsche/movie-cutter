#include "converter.h"

converter::converter(AVStream *st, AVPixelFormat to_pix_fmt)
{
	AVCodecContext *codec_ctx_ = st->codec;
	AVRational aspect = st->sample_aspect_ratio;
	if (aspect.num <= 0 || aspect.den <= 0)
		aspect = codec_ctx_->sample_aspect_ratio;

	int width = codec_ctx_->width * aspect.num / aspect.den;
	int height = codec_ctx_->height;
	int size = avpicture_get_size(to_pix_fmt, width, height);
	data_ = (uint8_t*) av_malloc(size);
	dest_ = av_frame_alloc();
	dest_->width = width;
	dest_->height = height;
	avpicture_fill((AVPicture*) dest_, data_, to_pix_fmt, width, height);
	sws_ctx_ = sws_getContext(codec_ctx_->width, codec_ctx_->height,
				  codec_ctx_->pix_fmt,
				  width, height,
				  to_pix_fmt, SWS_BICUBIC,
				  NULL, NULL, NULL);
}

converter::~converter()
{
	av_frame_free(&dest_);
	av_free(data_);
}

AVFrame *converter::convert_frame(AVFrame *src)
{
	sws_scale(sws_ctx_, src->data, src->linesize, 0, src->height,
		  dest_->data, dest_->linesize);
	return dest_;
}
