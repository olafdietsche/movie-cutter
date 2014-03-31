#include "frame_sequence.h"
#include "demuxer.h"
#include "converter.h"
#include "keyboard_shortcuts.h"

extern "C" {
#include "libavformat/avformat.h"
}

#include <gdk/gdkkeysyms.h>
#include <iostream>

namespace {
int64_t START_PTS = 0 * AV_TIME_BASE, STEP_PTS = 60 * AV_TIME_BASE;
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
		dmux_.index_stream(video_stream_index_);
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

	AVStream *video_stream = dmux_.get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	converter conv(video_stream, AV_PIX_FMT_RGB24, thumbnail::DEFAULT_WIDTH);
	auto i = frames_.begin();
	for (; i != frames_.end(); ++i) {
		if (dmux_.seek_to_keyframe(video_stream_index_, start) < 0)
			break;

		AVFrame *frame = dmux_.decode_packets_until(dec_ctx, video_stream_index_, start);
		if (!frame)
			break;

		AVFrame *dest = conv.convert_frame(frame);
		i->set_from_avframe(dest);
		i->set_label(video_stream);
		start += step;
	}

	n_frames_ = std::distance(frames_.begin(), i);

	for (; i != frames_.end(); ++i)
		i->clear();

	select_frame(&frames_[0]);
}

GdkPixbuf *frame_sequence::get_current_pixbuf()
{
	AVStream *video_stream = dmux_.get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	int width = container_->allocation.width;
	converter conv(video_stream, AV_PIX_FMT_RGB24, width);
	int64_t pts = current_frame_->get_pts();
	dmux_.seek_to_keyframe(video_stream_index_, pts);
	if (AVFrame *frame = dmux_.decode_packets_until(dec_ctx, video_stream_index_, pts)) {
		AVFrame *dest = conv.convert_frame(frame);
		return thumbnail::gdk_pixbuf_new_from_avframe(dest);
	}

	return 0;
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

void frame_sequence::create_keyboard_shortcuts(GtkAccelGroup *accel_group)
{
	create_keyboard_shortcut(accel_group, '+', accel_method_cb<frame_sequence, &frame_sequence::zoom_in>, this);
	create_keyboard_shortcut(accel_group, '-', accel_method_cb<frame_sequence, &frame_sequence::zoom_out>, this);
	create_keyboard_shortcut(accel_group, '0', accel_method_cb<frame_sequence, &frame_sequence::zoom_home>, this);
	create_keyboard_shortcut(accel_group, GDK_KEY_Page_Up, accel_method_cb<frame_sequence, &frame_sequence::page_backward>, this);
	create_keyboard_shortcut(accel_group, GDK_KEY_Page_Down, accel_method_cb<frame_sequence, &frame_sequence::page_forward>, this);
	create_keyboard_shortcut(accel_group, GDK_KEY_Home, GDK_CONTROL_MASK, accel_method_cb<frame_sequence, &frame_sequence::goto_first_frame>, this);
	create_keyboard_shortcut(accel_group, GDK_KEY_End, GDK_CONTROL_MASK, accel_method_cb<frame_sequence, &frame_sequence::goto_last_frame>, this);
}

void frame_sequence::select_frame(video_frame *frame)
{
	if (current_frame_)
		current_frame_->remove_mark();

	if (frame == current_frame_) {
		current_frame_ = 0;
	} else {
		current_frame_ = frame;
		if (frame)
			frame->mark_as_selected();
	}
}

void frame_sequence::sequence_select_frame(GtkWidget *btn, video_frame *frame)
{
	GtkWidget *box = gtk_widget_get_parent(btn);
	GtkWidget *parent = gtk_widget_get_parent(box);
	gpointer data = g_object_get_data(G_OBJECT(parent), "x-app-object");
	frame_sequence *sequence = reinterpret_cast<frame_sequence*>(data);
	sequence->select_frame(frame);
}
