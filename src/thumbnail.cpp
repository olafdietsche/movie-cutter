#include "thumbnail.h"
#include "demuxer.h"
#include <sstream>

GdkPixbuf *thumbnail::gdk_pixbuf_new_from_avframe(AVFrame *frame)
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

thumbnail::thumbnail()
	: box_(gtk_vbox_new(false, 0)),
	  btn_(gtk_button_new()),
	  img_(gtk_image_new()),
	  lbl_(gtk_label_new("")),
	  handler_id_(0)
{
	gtk_button_set_relief(GTK_BUTTON(btn_), GTK_RELIEF_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(btn_), 0);
	gtk_button_set_image(GTK_BUTTON(btn_), img_);
	gtk_widget_show(img_);
	gtk_box_pack_start(GTK_BOX(box_), btn_, false, false, 0);
	gtk_widget_show(btn_);
	gtk_box_pack_start(GTK_BOX(box_), lbl_, false, false, 0);
	gtk_widget_show(lbl_);
	gtk_widget_show(box_);
}

void thumbnail::clear()
{
	gtk_widget_set_sensitive(btn_, false);
	GdkPixbuf *buf = gtk_image_get_pixbuf(GTK_IMAGE(img_));
	gtk_image_clear(GTK_IMAGE(img_));
	if (buf)
		g_object_unref(buf);

	set_label("");

	pts_ = AV_NOPTS_VALUE;
	pict_type_ = AV_PICTURE_TYPE_NONE;
	display_picture_number_ = coded_picture_number_ = -1;
}

void thumbnail::set_from_avframe(AVFrame *frame)
{
	pts_ = frame->pts;
	if (pts_ == AV_NOPTS_VALUE)
		pts_ = frame->pkt_pts;

	pict_type_ = frame->pict_type;
	display_picture_number_ = frame->display_picture_number;
	coded_picture_number_ = frame->coded_picture_number;
	GdkPixbuf *buf = gdk_pixbuf_new_from_avframe(frame);
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_), buf);
	gtk_widget_set_sensitive(btn_, true);
}

void thumbnail::set_from_thumbnail(const thumbnail &frame)
{
	pts_ = frame.pts_;
	display_picture_number_ = frame.display_picture_number_;
	coded_picture_number_ = frame.coded_picture_number_;
	GdkPixbuf *buf = gtk_image_get_pixbuf(GTK_IMAGE(frame.img_));
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_), buf);
	set_label(frame.label_);
}

void thumbnail::connect_clicked(GCallback callback, gpointer user_data)
{
	if (handler_id_ != 0)
		g_signal_handler_disconnect(btn_, handler_id_);

	handler_id_ = g_signal_connect(btn_, "clicked", callback, user_data);
}

void thumbnail::set_label(const std::string &s)
{
	label_ = s;
	gtk_label_set_text(GTK_LABEL(lbl_), label_.c_str());
}

void thumbnail::set_label(AVStream *st)
{
	int64_t ts = demuxer::normalize_timestamp(st, pts_);
	std::ostringstream s;
	s << '(' << av_get_picture_type_char(pict_type_) << ") " << demuxer::format_timestamp(ts);
	set_label(s.str());
}
