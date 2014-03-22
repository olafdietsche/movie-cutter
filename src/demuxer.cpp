#include "demuxer.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
}

#include <iomanip>
#include <sstream>

demuxer::demuxer()
	: fmt_ctx_(0),
	  frame_(av_frame_alloc())
{
}

demuxer::~demuxer()
{
	close();
	av_frame_free(&frame_);
}

int64_t demuxer::normalize_timestamp(AVStream *st, int64_t ts)
{
	if (ts == AV_NOPTS_VALUE)
		return ts;

	if (ts < 0)
		av_log(st->codec, AV_LOG_WARNING, "normalize_timestamp_timestamp: timestamp=%ld < 0\n", ts);

	ts -= start_timestamp(st);
	return av_rescale_q(ts, st->time_base, AV_TIME_BASE_Q);
}

int64_t demuxer::start_timestamp(AVStream *st)
{
	return st->start_time == AV_NOPTS_VALUE ? 0 : st->start_time;
}

int64_t demuxer::rescale_timestamp(AVStream *st, int64_t ts)
{
	if (ts == AV_NOPTS_VALUE)
		return ts;

	if (ts < 0)
		av_log(st->codec, AV_LOG_WARNING, "rescale_timestamp: timestamp=%ld < 0\n", ts);

	return av_rescale_q(ts, AV_TIME_BASE_Q, st->time_base) + start_timestamp(st);
}

std::string demuxer::format_timestamp(int64_t ts)
{
	int seconds = ts / AV_TIME_BASE;
	int minutes = seconds / 60;
	seconds %= 60;
	int hours = minutes / 60;
	minutes %= 60;
	ts %= AV_TIME_BASE;

	std::ostringstream s;
	s << hours << ':' << std::setfill('0') << std::setw(2) << minutes
	  << ':' << std::setw(2) << seconds
	  << '.' << std::setw(3) << ts * 1000 / AV_TIME_BASE;

	return s.str();
}

int64_t demuxer::ticks_per_frame(AVStream *st)
{
	return (int64_t) st->time_base.den * st->r_frame_rate.den / st->time_base.num / st->r_frame_rate.num;
}

int demuxer::open_input(const char *filename)
{
	int err;

	// open container
	err = avformat_open_input(&fmt_ctx_, filename, NULL, NULL);
	if (err < 0) {
		av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, av_err2str(err));
		return err;
	}

	// look for container and stream types
	err = avformat_find_stream_info(fmt_ctx_, NULL);
	if (err < 0) {
		av_log(fmt_ctx_, AV_LOG_ERROR, "%s: %s\n", filename, av_err2str(err));
		return err;
	}

	// initialize needed codecs
	for (unsigned i = 0; i < fmt_ctx_->nb_streams; ++i) {
		AVStream *st = fmt_ctx_->streams[i];
		AVCodecContext *dec_ctx = st->codec;
		AVCodec *dec = avcodec_find_decoder(dec_ctx->codec_id);
		if (!dec) {
			av_log(dec_ctx, AV_LOG_ERROR, "%s: failed to find %s codec\n",
			       filename, av_get_media_type_string(dec_ctx->codec_type));
			return AVERROR_DECODER_NOT_FOUND;
		}

		err = avcodec_open2(dec_ctx, dec, NULL);
		if (err < 0) {
			av_log(dec_ctx, AV_LOG_ERROR, "%s: failed to open %s codec; %s\n",
			       filename, av_get_media_type_string(dec_ctx->codec_type),
			       av_err2str(err));
			return err;
		}
	}

	return 0;
}

void demuxer::close()
{
	if (fmt_ctx_) {
		// close container
		avformat_close_input(&fmt_ctx_);
	}
}

int demuxer::seek(int stream_index, int64_t pts)
{
	AVStream *st = get_stream(stream_index);
	AVCodecContext *codec = st->codec;
	av_log(codec, AV_LOG_DEBUG, "demuxer::seek(%d, %ld)\n", stream_index, pts);
	avcodec_flush_buffers(codec);
	pts -= codec->gop_size * demuxer::ticks_per_frame(st);
	int err = avformat_seek_file(fmt_ctx_, stream_index, INT64_MIN, pts, pts, 0);
	if (err < 0)
		av_log(fmt_ctx_, AV_LOG_ERROR, "avformat_seek_file: err=%s\n", av_err2str(err));

	return err;
}

int demuxer::seek(AVMediaType codec_type, int64_t pts)
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
		av_log(dec_ctx, AV_LOG_ERROR, "%s\n", av_err2str(size));
		return false;
	}

	// advance pointer by consumed size
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
