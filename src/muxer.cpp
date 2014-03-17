#include "muxer.h"
#include <cstring>
#include <memory>
#include <vector>

namespace {
AVOutputFormat *find_output_format(const AVInputFormat *iformat)
{
	std::unique_ptr<char, void(*)(void*)> names(strdup(iformat->name), &free);
	std::vector<AVOutputFormat*> output_formats;
	char *name = strtok(names.get(), ",");
	while (name) {
		if (AVOutputFormat *fmt = av_guess_format(name, NULL, NULL))
			output_formats.push_back(fmt);

		name = strtok(NULL, ",");
	}

	if (output_formats.empty()) {
		av_log(NULL, AV_LOG_ERROR, "No output format found.\n");
		return NULL;
	}

	if (output_formats.size() > 1) {
		av_log(NULL, AV_LOG_ERROR, "More than one output format found.\n");
		return NULL;
	}

	return output_formats[0];
}
}

int muxer::open_output(const char *filename, const AVFormatContext *fmt_ctx_in)
{
	fmt_ctx_ = avformat_alloc_context();
	if (!fmt_ctx_) {
		av_log(NULL, AV_LOG_ERROR, "Failed to allocate format context\n");
		return -1;
	}

	fmt_ctx_->oformat = find_output_format(fmt_ctx_in->iformat);
	if (!fmt_ctx_->oformat) {
		avformat_free_context(fmt_ctx_);
		return -1;
	}

	if (!(fmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
		int ret = avio_open(&fmt_ctx_->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			av_log(fmt_ctx_, AV_LOG_ERROR, "Cannot open %s (%s)\n", filename, av_err2str(ret));
			return ret;
		}
	}

	for (unsigned i = 0; i < fmt_ctx_in->nb_streams; ++i) {
		AVStream *st = avformat_new_stream(fmt_ctx_, fmt_ctx_in->streams[i]->codec->codec);
		avcodec_copy_context(st->codec, fmt_ctx_in->streams[i]->codec);
	}

	int ret = avformat_write_header(fmt_ctx_, NULL);
	if (ret < 0) {
		av_log(fmt_ctx_, AV_LOG_ERROR, "Error writing header %s (%s)\n", filename, av_err2str(ret));
		return ret;
	}

	return 0;
}

void muxer::close()
{
	if (fmt_ctx_) {
		av_write_trailer(fmt_ctx_);
		if (!(fmt_ctx_->oformat->flags & AVFMT_NOFILE))
			avio_close(fmt_ctx_->pb);

		for (unsigned i = 0; i < fmt_ctx_->nb_streams; ++i) {
			avcodec_close(fmt_ctx_->streams[i]->codec);
		}

		avformat_free_context(fmt_ctx_);
		fmt_ctx_ = 0;
	}
}

int muxer::write_packet(AVPacket *pkt)
{
	int ret = av_interleaved_write_frame(fmt_ctx_, pkt);
	if (ret < 0)
		av_log(fmt_ctx_, AV_LOG_ERROR, "Error writing packet (%s)\n", av_err2str(ret));

	return ret;
}
