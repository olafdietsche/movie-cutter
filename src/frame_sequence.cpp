#include "frame_sequence.h"
#include "demuxer.h"
#include "converter.h"

extern "C" {
#include "libavformat/avformat.h"
}

#include <iostream>

namespace {
int64_t START_PTS = 0 * AV_TIME_BASE, STEP_PTS = 60 * AV_TIME_BASE;

std::string normalize_timestamp(AVStream *st, int64_t ts)
{
	ts = demuxer::normalize_timestamp(st, ts);
	return demuxer::format_timestamp(ts);
}

#define a(n) ", " #n "=" << frame->n
#define t(n) std::cerr << ", " #n "=" << normalize_timestamp(st, frame->n)

void dump_avframe(AVStream *st, AVFrame *frame)
{
	std::cerr << "dump_avframe: key_frame=" << frame->key_frame;
	t(pts); t(pkt_pts); t(pkt_dts);
	std::cerr << a(coded_picture_number) << a(display_picture_number)
		  << '\n';
}

#undef t
#undef a

}

frame_sequence::frame_sequence(int rows, int columns)
	: current_frame_(0),
	  current_step_(0)
{
	create_sequence(rows, columns);
}

void frame_sequence::clear_sequence()
{
	current_frame_ = 0;
	for (auto i = frames_.begin(); i != frames_.end(); ++i) {
		i->clear();
	}
}

void frame_sequence::create_sequence(int rows, int columns)
{
	container_ = gtk_table_new(rows, columns, 1);
	g_object_set_data(G_OBJECT(container_), "x-app-object", this);
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			video_frame f(container_, i, j);
			frames_.push_back(f);
		}
	}
}

int frame_sequence::update_sequence(const char *filename)
{
	dmux_.close();
	int err = dmux_.open_input(filename);
	if (err >= 0) {
		video_stream_index_ = dmux_.get_stream_index(AVMEDIA_TYPE_VIDEO);
		AVStream *video_stream = dmux_.get_stream(video_stream_index_);
		current_0_ = demuxer::rescale_timestamp(video_stream, START_PTS);
		int64_t stream_step = demuxer::rescale_timestamp(video_stream, START_PTS + STEP_PTS);
		stream_step -= current_0_;
		minimum_step_ = demuxer::ticks_per_frame(video_stream);
		update_sequence(current_0_, stream_step);
	}

	return err;
}

void frame_sequence::update_sequence(int64_t start, int64_t step)
{
	if (start < current_0_)
		start = current_0_;

	if (step < minimum_step_)
		step = minimum_step_;

	current_step_ = step;

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	AVStream *video_stream = dmux_.get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	dmux_.seek(video_stream_index_, start);
	converter conv(video_stream, AV_PIX_FMT_RGB24, thumbnail::DEFAULT_WIDTH);
	n_frames_ = 0;
	auto i = frames_.begin();
	while (i != frames_.end()
	       && dmux_.read_next_packet(&pkt, video_stream_index_) >= 0) {
		AVPacket tmp = pkt;
		do {
			if (dmux_.decode_packet(dec_ctx, &tmp)) {
				if (step <= minimum_step_ || pkt.pts >= start) {
					AVFrame *frame = dmux_.get_current_frame();
					AVFrame *dest = conv.convert_frame(frame);
					i->set_from_avframe(dest);
					i->set_label(video_stream);
					++i;
					n_frames_ = std::distance(frames_.begin(), i);
					start += step;
					if (step > minimum_step_)
						dmux_.seek(video_stream_index_, start);

					//break;
				}
			}
		} while (tmp.size > 0);

		av_free_packet(&pkt);
	}

	for (; i != frames_.end(); ++i)
		i->clear();

	// flush cached frames
	dmux_.flush(&pkt);
	goto_frame(&frames_[0]);
}

GdkPixbuf *frame_sequence::get_current_pixbuf()
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	AVStream *video_stream = dmux_.get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	int width = container_->allocation.width;
	converter conv(video_stream, AV_PIX_FMT_RGB24, width);
	dmux_.seek(video_stream_index_, current_frame_->get_pts());
	while (dmux_.read_next_packet(&pkt, video_stream_index_) >= 0) {
		AVPacket tmp = pkt;
		do {
			if (dmux_.decode_packet(dec_ctx, &tmp)) {
				if (pkt.pts >= current_frame_->get_pts()) {
					AVFrame *frame = dmux_.get_current_frame();
					AVFrame *dest = conv.convert_frame(frame);
					return thumbnail::gdk_pixbuf_new_from_avframe(dest);
				}
			}
		} while (tmp.size > 0);

		av_free_packet(&pkt);
	}

	return NULL;
}

void frame_sequence::goto_first_frame()
{
	if (n_frames_) {
		clear_sequence();
		update_sequence(INT64_MIN, current_step_);
	}
}

void frame_sequence::page_backward()
{
	if (n_frames_) {
		int64_t start = frames_[0].get_pts() - current_step_ * (frames_.size() - 1);
		clear_sequence();
		update_sequence(start, current_step_);
	}
}

void frame_sequence::page_forward()
{
	if (n_frames_) {
		int64_t start = frames_[n_frames_ - 1].get_pts();
		clear_sequence();
		update_sequence(start, current_step_);
	}
}

void frame_sequence::goto_last_frame()
{
	if (n_frames_) {
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		dmux_.seek(video_stream_index_, INT64_MAX);
		dmux_.read_next_packet(&pkt, video_stream_index_);
		int64_t last_pts = pkt.pts;

		clear_sequence();
		update_sequence(last_pts - current_step_ * frames_.size(), current_step_);
	}
}

void frame_sequence::zoom_in()
{
	if (current_frame_ && current_step_ > minimum_step_) {
		int64_t first = frames_[0].get_pts();
		zoom_ins_.push(std::make_tuple(first, current_step_));

		int64_t step = current_step_ / (frames_.size() - 1);
		if (step <= 0)
			step = 1;

		update_sequence(current_frame_->get_pts(), step);
	}
}

void frame_sequence::zoom_out()
{
	if (!zoom_ins_.empty()) {
		int64_t first, step;
		std::tie(first, step) = zoom_ins_.top();
		zoom_ins_.pop();
		update_sequence(first, step);
	}
}

void frame_sequence::zoom_home()
{
	if (!zoom_ins_.empty()) {
		int64_t first, step;

		do {
			std::tie(first, step) = zoom_ins_.top();
			zoom_ins_.pop();
		} while (!zoom_ins_.empty());

		update_sequence(first, step);
	}
}

void frame_sequence::goto_frame(video_frame *frame)
{
	current_frame_ = frame;
}

void frame_sequence::sequence_goto_frame(GtkWidget *btn, video_frame *frame)
{
	GtkWidget *box = gtk_widget_get_parent(btn);
	GtkWidget *parent = gtk_widget_get_parent(box);
	gpointer data = g_object_get_data(G_OBJECT(parent), "x-app-object");
	frame_sequence *sequence = reinterpret_cast<frame_sequence*>(data);
	sequence->goto_frame(frame);
}
