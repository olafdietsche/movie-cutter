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
}

frame_sequence::thumbnail::thumbnail(GtkWidget *container, int row, int column)
	: img_(gtk_image_new())
{
	gtk_table_attach_defaults(GTK_TABLE(container), img_, column, column + 1, row, row + 1);
	gtk_widget_show(img_);
}

void frame_sequence::thumbnail::clear()
{
	GdkPixbuf *buf = gtk_image_get_pixbuf(GTK_IMAGE(img_));
	gtk_image_clear(GTK_IMAGE(img_));
	if (buf)
		g_object_unref(buf);
}

void frame_sequence::thumbnail::set_from_avframe(AVFrame *frame)
{
	pts_ = frame->pts;
	if (pts_ == AV_NOPTS_VALUE)
		pts_ = frame->pkt_pts;

	display_picture_number_ = frame->display_picture_number;

	GdkPixbuf *buf = gdk_pixbuf_new_from_avframe(frame);
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_), buf);
}

frame_sequence::frame_sequence(int rows, int columns)
{
	create_sequence(rows, columns);
}

void frame_sequence::clear_sequence()
{
	for (auto i = imgs_.begin(); i != imgs_.end(); ++i) {
		i->clear();
	}
}

void frame_sequence::create_sequence(int rows, int columns)
{
	container_ = gtk_table_new(rows, columns, 0);
	gtk_table_set_row_spacings(GTK_TABLE(container_), 2);
	gtk_table_set_col_spacings(GTK_TABLE(container_), 2);
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
	update_sequence(current_0_, stream_step);
}

void frame_sequence::update_sequence(int64_t start, int64_t step)
{
	std::cerr << "update_sequence(" << start << ", " << step << ")\n";
	if (start < current_0_)
		start = current_0_;

	current_step_ = step;

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	AVStream *video_stream = dmux_->get_stream(video_stream_index_);
	AVCodecContext *dec_ctx = video_stream->codec;
	dmux_->seek(video_stream_index_, start);
	converter conv(video_stream, AV_PIX_FMT_RGB24, THUMBNAIL_WIDTH_);
	for (auto i = imgs_.begin(); i != imgs_.end()
		     && dmux_->read_next_packet(&pkt, video_stream_index_) >= 0; ) {
		AVPacket tmp = pkt;
		do {
			if (dmux_->decode_packet(dec_ctx, &tmp)) {
				if (pkt.pts >= start) {
					AVFrame *frame = dmux_->get_current_frame();
					AVFrame *dest = conv.convert_frame(frame);
					i->set_from_avframe(dest);
					++i;
					start += step;
					dmux_->seek(video_stream_index_, start);
					break;
				}
			}
		} while (tmp.size > 0);

		av_free_packet(&pkt);
	}

	// flush cached frames
	dmux_->flush(&pkt);
}

void frame_sequence::page_backward()
{
	thumbnail &first = imgs_.front();
	int64_t start = first.pts_ - current_step_ * imgs_.size();
	clear_sequence();
	update_sequence(start, current_step_);
}

void frame_sequence::page_forward()
{
	thumbnail &last = imgs_.back();
	clear_sequence();
	update_sequence(last.pts_, current_step_);
}

void frame_sequence::zoom_in()
{
	std::cerr << "zoom_in\n";
}

void frame_sequence::zoom_out()
{
	std::cerr << "zoom_out\n";
}
