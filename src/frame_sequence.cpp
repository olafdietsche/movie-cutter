#include "frame_sequence.h"
#include "demuxer.h"
#include "converter.h"

extern "C" {
#include "libavformat/avformat.h"
}

#include <iostream>

namespace {
int64_t START_PTS = 0 * AV_TIME_BASE, STEP_PTS = 60 * AV_TIME_BASE;

GdkPixbuf *gdk_pixbuf_new_from_avframe(AVFrame *frame)
{
	int size = avpicture_get_size(AV_PIX_FMT_RGB24, frame->width, frame->height);
	gpointer data = g_malloc(size);
	memcpy(data, frame->data[0], size);
	GdkPixbufDestroyNotify destroy_fn = reinterpret_cast<GdkPixbufDestroyNotify>(g_free);
	GdkPixbuf *buf = gdk_pixbuf_new_from_data(reinterpret_cast<const guchar*>(data),
						  GDK_COLORSPACE_RGB,
						  false, 8, frame->width, frame->height,
						  frame->linesize[0],
						  destroy_fn, NULL);
	return buf;
}

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

frame_sequence::thumbnail::thumbnail(GtkWidget *container, int row, int column)
	: btn_(gtk_button_new()),
	  img_(gtk_image_new()),
	  handler_id_(-1)
{
        gtk_button_set_relief(GTK_BUTTON(btn_), GTK_RELIEF_NONE);
        gtk_container_set_border_width(GTK_CONTAINER(btn_), 0);
	gtk_button_set_image(GTK_BUTTON(btn_), img_);
	gtk_widget_show(img_);

	gtk_table_attach_defaults(GTK_TABLE(container), btn_, column, column + 1, row, row + 1);
	gtk_widget_show(btn_);
	handler_id_ = g_signal_connect(btn_, "clicked", G_CALLBACK(sequence_goto_frame), this);
}

frame_sequence::thumbnail::thumbnail(const thumbnail &x)
	: btn_(x.btn_),
	  img_(x.img_),
	  handler_id_(-1),
	  pts_(x.pts_),
	  display_picture_number_(x.display_picture_number_),
	  coded_picture_number_(x.coded_picture_number_)
{
	g_signal_handler_disconnect(btn_, x.handler_id_);
	handler_id_ = g_signal_connect(btn_, "clicked", G_CALLBACK(sequence_goto_frame), this);
}

void frame_sequence::thumbnail::clear()
{
	gtk_widget_set_sensitive(btn_, false);
	GdkPixbuf *buf = gtk_image_get_pixbuf(GTK_IMAGE(img_));
	gtk_image_clear(GTK_IMAGE(img_));
	if (buf)
		g_object_unref(buf);

	pts_ = AV_NOPTS_VALUE;
	display_picture_number_ = coded_picture_number_ = -1;
}

void frame_sequence::thumbnail::set_from_avframe(AVFrame *frame)
{
	pts_ = frame->pts;
	if (pts_ == AV_NOPTS_VALUE)
		pts_ = frame->pkt_pts;

	display_picture_number_ = frame->display_picture_number;

	GdkPixbuf *buf = gdk_pixbuf_new_from_avframe(frame);
	coded_picture_number_ = frame->coded_picture_number;
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_), buf);
	gtk_widget_set_sensitive(btn_, true);
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
	for (auto i = imgs_.begin(); i != imgs_.end(); ++i) {
		i->clear();
	}
}

void frame_sequence::create_sequence(int rows, int columns)
{
	container_ = gtk_table_new(rows, columns, 1);
	g_object_set_data(G_OBJECT(container_), "x-app-object", this);
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			thumbnail t(container_, i, j);
			imgs_.push_back(t);
		}
	}
}

void frame_sequence::update_sequence(demuxer &dmux)
{
	dmux_ = &dmux;
	video_stream_index_ = dmux_->get_stream_index(AVMEDIA_TYPE_VIDEO);
	AVStream *video_stream = dmux_->get_stream(video_stream_index_);
	current_0_ = demuxer::rescale_timestamp(video_stream, START_PTS);
	int64_t stream_step = demuxer::rescale_timestamp(video_stream, START_PTS + STEP_PTS);
	stream_step -= current_0_;
	minimum_step_ = demuxer::ticks_per_frame(video_stream);

	update_sequence(current_0_, stream_step);
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
	AVStream *video_stream = dmux_->get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	dmux_->seek(video_stream_index_, start);
	converter conv(video_stream, AV_PIX_FMT_RGB24, THUMBNAIL_WIDTH_);
	n_thumbnails_ = 0;
	for (auto i = imgs_.begin(); i != imgs_.end()
		     && dmux_->read_next_packet(&pkt, video_stream_index_) >= 0; ) {
		AVPacket tmp = pkt;
		do {
			if (dmux_->decode_packet(dec_ctx, &tmp)) {
				if (step <= minimum_step_ || pkt.pts >= start) {
					AVFrame *frame = dmux_->get_current_frame();
					AVFrame *dest = conv.convert_frame(frame);
					i->set_from_avframe(dest);
					++i;
					n_thumbnails_ = std::distance(imgs_.begin(), i);
					start += step;
					if (step > minimum_step_)
						dmux_->seek(video_stream_index_, start);

					//break;
				}
			}
		} while (tmp.size > 0);

		av_free_packet(&pkt);
	}

	// flush cached frames
	dmux_->flush(&pkt);
	goto_frame(&imgs_[0]);
}

GdkPixbuf *frame_sequence::get_current_frame()
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	AVStream *video_stream = dmux_->get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	int width = container_->allocation.width;
	converter conv(video_stream, AV_PIX_FMT_RGB24, width);
	dmux_->seek(video_stream_index_, current_frame_->pts_);
	while (dmux_->read_next_packet(&pkt, video_stream_index_) >= 0) {
		AVPacket tmp = pkt;
		do {
			if (dmux_->decode_packet(dec_ctx, &tmp)) {
				if (pkt.pts >= current_frame_->pts_) {
					AVFrame *frame = dmux_->get_current_frame();
					AVFrame *dest = conv.convert_frame(frame);
					return gdk_pixbuf_new_from_avframe(dest);
				}
			}
		} while (tmp.size > 0);

		av_free_packet(&pkt);
	}

	return NULL;
}

void frame_sequence::page_backward()
{
	if (n_thumbnails_) {
		int64_t start = imgs_[0].pts_ - current_step_ * (imgs_.size() - 1);
		clear_sequence();
		update_sequence(start, current_step_);
	}
}

void frame_sequence::page_forward()
{
	if (n_thumbnails_) {
		int64_t start = imgs_[n_thumbnails_ - 1].pts_;
		clear_sequence();
		update_sequence(start, current_step_);
	}
}

void frame_sequence::zoom_in()
{
	if (current_frame_ && current_step_ > minimum_step_) {
		int64_t first = imgs_[0].pts_;
		zoom_ins_.push(std::make_tuple(first, current_step_));

		int64_t step = current_step_ / (imgs_.size() - 1);
		if (step <= 0)
			step = 1;

		update_sequence(current_frame_->pts_, step);
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

void frame_sequence::goto_frame(thumbnail *frame)
{
	current_frame_ = frame;
}

void frame_sequence::sequence_goto_frame(GtkWidget *btn, frame_sequence::thumbnail *frame)
{
	GtkWidget *parent = gtk_widget_get_parent(btn);
	gpointer data = g_object_get_data(G_OBJECT(parent), "x-app-object");
	frame_sequence *sequence = reinterpret_cast<frame_sequence*>(data);
	sequence->goto_frame(frame);
}
