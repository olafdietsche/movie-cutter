#include "converter.h"

converter::converter(AVStream *st, AVPixelFormat to_pix_fmt)
{
	AVCodecContext *codec_ctx = st->codec;
	AVRational aspect = st->sample_aspect_ratio;
	if (aspect.num <= 0 || aspect.den <= 0)
		aspect = codec_ctx->sample_aspect_ratio;

	int width = codec_ctx->width * aspect.num / aspect.den;
	int height = codec_ctx->height;
	initialize(codec_ctx, to_pix_fmt, width, height);
}

converter::converter(AVStream *st, AVPixelFormat to_pix_fmt, int width)
{
	AVCodecContext *codec_ctx = st->codec;
	AVRational aspect = st->sample_aspect_ratio;
	if (aspect.num <= 0 || aspect.den <= 0)
		aspect = codec_ctx->sample_aspect_ratio;

	int height = codec_ctx->height * width / codec_ctx->width;
	height = height * aspect.den / aspect.num;
	initialize(codec_ctx, to_pix_fmt, width, height);
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
	dest_->pts = src->pts;
	dest_->pkt_pts = src->pkt_pts;
	dest_->display_picture_number = src->display_picture_number;
	return dest_;
}

void converter::initialize(AVCodecContext *codec_ctx, AVPixelFormat to_pix_fmt, int width, int height)
{
	int size = avpicture_get_size(to_pix_fmt, width, height);
	data_ = (uint8_t*) av_malloc(size);
	dest_ = av_frame_alloc();
	dest_->width = width;
	dest_->height = height;
	avpicture_fill((AVPicture*) dest_, data_, to_pix_fmt, width, height);
	sws_ctx_ = sws_getContext(codec_ctx->width, codec_ctx->height,
				  codec_ctx->pix_fmt,
				  width, height,
				  to_pix_fmt, SWS_BICUBIC,
				  NULL, NULL, NULL);
}
