#include "demuxer.h"
extern "C" {
#include "libavformat/avformat.h"
}

static void print_error(void *ctx, int err)
{
	char errbuf[128];
	const char *errbuf_ptr = errbuf;

	if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
		errbuf_ptr = strerror(AVUNERROR(err));

	av_log(ctx, AV_LOG_ERROR, "%s\n", errbuf_ptr);
}

demuxer::demuxer(const char *filename)
	: fmt_ctx_(0),
	  frame_(av_frame_alloc())
{
	open(filename);
}

demuxer::~demuxer()
{
	close();
	av_frame_free(&frame_);
}

int demuxer::open(const char *filename)
{
	int err;

	// open container
	err = avformat_open_input(&fmt_ctx_, filename, NULL, NULL);
	if (err < 0) {
		print_error(NULL, err);
		return err;
	}

	// look for container and stream types
	err = avformat_find_stream_info(fmt_ctx_, NULL);
	if (err < 0) {
		print_error(fmt_ctx_, err);
		return err;
	}

	// initialize needed codecs
	for (unsigned i = 0; i < fmt_ctx_->nb_streams; ++i) {
		AVStream *st = fmt_ctx_->streams[i];
		AVCodecContext *dec_ctx = st->codec;
		AVCodec *dec = avcodec_find_decoder(dec_ctx->codec_id);
		if (!dec) {
			av_log(dec_ctx, AV_LOG_ERROR, "Failed to find %s codec\n",
			       av_get_media_type_string(dec_ctx->codec_type));
			return -1;
		}

		err = avcodec_open2(dec_ctx, dec, NULL);
		if (err < 0) {
			print_error(dec_ctx, err);
			av_log(dec_ctx, AV_LOG_ERROR, "Failed to open %s codec\n",
			       av_get_media_type_string(dec_ctx->codec_type));
			return err;
		}
	}

	return 0;
}

void demuxer::close()
{
	// close container
	avformat_close_input(&fmt_ctx_);
}

#if 0
AVFrame *demuxer::get_next_frame()
{
	// read next packet if neccessary
	if (pkt_.size == 0) {
		if (orig_pkt_.data)
			av_free_packet(&orig_pkt_);

		int err = av_read_frame(fmt_ctx_, &pkt_);
		if (err < 0) {
			print_error(fmt_ctx_, err);
			return NULL;
		}

		// save pkt.data for later av_free_packet()
		orig_pkt_ = pkt_;
	}

	av_frame_unref(frame_);
	
	return frame_;
}
#endif

bool demuxer::decode_packet(AVCodecContext *dec_ctx, AVPacket *pkt)
{
	int got_frame = 0;
	int size = pkt->size;
	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		// decode video frame
		size = avcodec_decode_video2(dec_ctx, frame_, &got_frame, pkt);
	} else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		// decode audio frame
		size = avcodec_decode_audio4(dec_ctx, frame_, &got_frame, pkt);
	}

	if (size < 0) {
		print_error(dec_ctx, size);
		return false;
	}

	if (pkt->data) {
		pkt->data += size;
		pkt->size -= size;
	}

	return got_frame;
}

void demuxer::frame_loop(Filter filter, Decoder decoder)
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	bool done = false;
	while (!done && av_read_frame(fmt_ctx_, &pkt) >= 0) {
		AVStream *st = fmt_ctx_->streams[pkt.stream_index];
		if (filter(st, &pkt)) {
			AVCodecContext *dec_ctx = st->codec;
			AVPacket tmp = pkt;
			do {
				if (decode_packet(dec_ctx, &tmp)) {
					done = decoder(dec_ctx, frame_);
					if (done)
						break;
				}
			} while (tmp.size > 0);
		}

		av_free_packet(&pkt);
	}

	// flush cached frames
	AVStream *st = fmt_ctx_->streams[pkt.stream_index];
	AVCodecContext *dec_ctx = st->codec;
	pkt.data = NULL;
	pkt.size = 0;
	while (decode_packet(dec_ctx, &pkt))
		;
}
