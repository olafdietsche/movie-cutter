#include "demuxer.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

static const char *str_error(int err)
{
	static char errbuf[128];
	if (av_strerror(err, errbuf, sizeof(errbuf)) >= 0)
		return errbuf;

	return strerror(AVUNERROR(err));
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

int64_t demuxer::rescale_timestamp(AVStream *st, int64_t ts)
{
	int64_t start = st->start_time;
	if (start == AV_NOPTS_VALUE)
		start = 0;

	return av_rescale_q(ts, AV_TIME_BASE_Q, st->time_base) + start;
}

int demuxer::open(const char *filename)
{
	int err;

	// open container
	err = avformat_open_input(&fmt_ctx_, filename, NULL, NULL);
	if (err < 0) {
		av_log(NULL, AV_LOG_ERROR, "%s\n", str_error(err));
		return err;
	}

	// look for container and stream types
	err = avformat_find_stream_info(fmt_ctx_, NULL);
	if (err < 0) {
		av_log(fmt_ctx_, AV_LOG_ERROR, "%s\n", str_error(err));
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
			av_log(dec_ctx, AV_LOG_ERROR, "Failed to open %s codec; %s\n",
			       av_get_media_type_string(dec_ctx->codec_type),
			       str_error(err));
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

/*
void demuxer::process_packet_queue()
{
	AVStream *st = fmt_ctx_->streams[pkt.stream_index];
	AVCodecContext *dec_ctx = st->codec;
	AVPacket tmp = pkt;
	do {
		av_log(fmt_ctx_, AV_LOG_DEBUG, "pkt: stream_index=%d, pts=%ld, flags=%x, convergence_duration=%ld\n", pkt.stream_index, pkt.pts, pkt.flags, pkt.convergence_duration);
		if (decode_packet(dec_ctx, &tmp)) {
			done = decoder(dec_ctx, frame_);
			//av_frame_unref(frame_);
			if (done)
				break;
		}
	} while (tmp.size > 0);

	av_free_packet(&pkt);
}
*/

AVFrame *demuxer::seek(int stream_index, int64_t pts)
{
	av_log(fmt_ctx_, AV_LOG_INFO, "seek(%d, %ld)\n", stream_index, pts);
	AVStream *st = fmt_ctx_->streams[stream_index];
	AVCodecContext *dec_ctx = st->codec;
	pts = av_rescale_q(pts, AV_TIME_BASE_Q, st->time_base);
	if (st->start_time != AV_NOPTS_VALUE)
		pts += st->start_time;

	if (pts == 0)
		pts = -st->first_dts;

	av_log(fmt_ctx_, AV_LOG_INFO, "AV_NOPTS_VALUE=%ld, start_time=(%ld, %ld), first_dts=%ld, time_base=%d/%d;%d/%d, pts=%ld\n", AV_NOPTS_VALUE, st->start_time, fmt_ctx_->start_time, st->first_dts, st->time_base.num, st->time_base.den, dec_ctx->time_base.num, dec_ctx->time_base.den, pts);

	int err = avformat_seek_file(fmt_ctx_, stream_index, st->first_dts, pts, pts, 0);
	if (err < 0) {
		av_log(fmt_ctx_, AV_LOG_ERROR, "avformat_seek_file: err=%s\n", str_error(err));
		return NULL;
	}

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	int n_packets = 0;
	bool got_frame = false;
	for (;;) {
		err = av_read_frame(fmt_ctx_, &pkt);
		if (err < 0) {
			av_log(fmt_ctx_, AV_LOG_ERROR, "av_read_frame: err=%s\n", str_error(err));
			return NULL;
		}

		if (pkt.stream_index == stream_index) {
			++n_packets;
			if (pkt.flags & AV_PKT_FLAG_KEY)
				av_log(fmt_ctx_, AV_LOG_INFO, "keyframe@%d, dts=%ld, pts=%ld\n", n_packets, pkt.dts, pkt.pts);

			AVPacket tmp = pkt;
			do {
				got_frame = decode_packet(dec_ctx, &tmp);
			} while (tmp.size > 0);

			if (got_frame && pkt.pts >= pts) {
				av_log(fmt_ctx_, AV_LOG_INFO, "n_packets=%d, dts=%ld, pts=%ld\n", n_packets, pkt.dts, pkt.pts);
				break;
			}
		}

		//av_free_packet(&pkt);
	}

	return got_frame ? frame_ : NULL;
}

AVFrame *demuxer::seek(AVMediaType codec_type, int64_t pts)
{
	int stream_index = av_find_best_stream(fmt_ctx_, codec_type, -1, -1, NULL, 0);
	return seek(stream_index, pts);
}

void demuxer::flush(AVPacket *pkt)
{
	AVStream *st = get_stream(pkt->stream_index);
	pkt->data = NULL;
	pkt->size = 0;
	while (decode_packet(st->codec, pkt))
		;
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
			av_log(fmt_ctx_, AV_LOG_ERROR, "%s\n", str_error(err));
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
	int got_frame;
	int size;
	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		// decode video frame
		size = avcodec_decode_video2(dec_ctx, frame_, &got_frame, pkt);
	} else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		// decode audio frame
		size = avcodec_decode_audio4(dec_ctx, frame_, &got_frame, pkt);
	} else {
		got_frame = 1;
		size = pkt->size;
	}

	if (size < 0) {
		av_log(dec_ctx, AV_LOG_ERROR, "%s\n", str_error(size));
		return false;
	}

	if (pkt->data) {
		pkt->data += size;
		pkt->size -= size;
	}

	return got_frame;
}

int demuxer::read_next_packet(AVPacket *pkt)
{
#if 1
	return av_read_frame(fmt_ctx_, pkt);
#else
	if (pkt_q_.empty())
		return av_read_frame(fmt_ctx_, pkt);

	*pkt = pkt_q_.front();
	pkt_q_.pop_front();
	return 0;
#endif
}

int demuxer::read_next_packet(AVPacket *pkt, int stream_index)
{
	int err;
	while ((err = read_next_packet(pkt)) >= 0) {
		if (pkt->stream_index == stream_index)
			break;

		av_free_packet(pkt);
	}

	return err;
}

void demuxer::push_packet(AVPacket *pkt)
{
	pkt_q_.push_back(*pkt);
}

void demuxer::flush_packet_queue()
{
	for (auto i = pkt_q_.begin(); i != pkt_q_.end(); ++i)
		av_free_packet(&*i);

	pkt_q_.clear();
}

void demuxer::frame_loop(Decoder decoder)
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	bool done = false;
	while (!done && read_next_packet(&pkt) >= 0) {
		AVStream *st = fmt_ctx_->streams[pkt.stream_index];
		AVCodecContext *dec_ctx = st->codec;
		AVPacket tmp = pkt;
		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
			do {
				//av_log(fmt_ctx_, AV_LOG_INFO, "frame_loop; pkt: stream_index=%d, dts=%ld, pts=%ld, flags=%x, convergence_duration=%ld\n", pkt.stream_index, pkt.dts, pkt.pts, pkt.flags, pkt.convergence_duration);
				if (decode_packet(dec_ctx, &tmp)) {
					done = decoder(dec_ctx, frame_);
					//av_frame_unref(frame_);
					if (done)
						break;
				}
			} while (tmp.size > 0);
		}

		av_free_packet(&pkt);
	}

	// flush cached frames
	flush(&pkt);
}
