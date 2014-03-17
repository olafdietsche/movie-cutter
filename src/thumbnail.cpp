#include "thumbnail.h"

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

thumbnail::thumbnail(GtkWidget *container, int row, int column)
	: btn_(gtk_button_new()),
	  img_(gtk_image_new()),
	  handler_id_(0)
{
        gtk_button_set_relief(GTK_BUTTON(btn_), GTK_RELIEF_NONE);
        gtk_container_set_border_width(GTK_CONTAINER(btn_), 0);
	gtk_button_set_image(GTK_BUTTON(btn_), img_);
	gtk_widget_show(img_);

	gtk_table_attach_defaults(GTK_TABLE(container), btn_, column, column + 1, row, row + 1);
	gtk_widget_show(btn_);
}

void thumbnail::clear()
{
	gtk_widget_set_sensitive(btn_, false);
	GdkPixbuf *buf = gtk_image_get_pixbuf(GTK_IMAGE(img_));
	gtk_image_clear(GTK_IMAGE(img_));
	if (buf)
		g_object_unref(buf);

	pts_ = AV_NOPTS_VALUE;
	display_picture_number_ = coded_picture_number_ = -1;
}

void thumbnail::set_from_avframe(AVFrame *frame)
{
	pts_ = frame->pts;
	if (pts_ == AV_NOPTS_VALUE)
		pts_ = frame->pkt_pts;

	display_picture_number_ = frame->display_picture_number;
	coded_picture_number_ = frame->coded_picture_number;
	GdkPixbuf *buf = gdk_pixbuf_new_from_avframe(frame);
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_), buf);
	gtk_widget_set_sensitive(btn_, true);
}

void thumbnail::connect_clicked(GCallback callback, gpointer user_data)
{
	if (handler_id_ != 0)
		g_signal_handler_disconnect(btn_, handler_id_);

	handler_id_ = g_signal_connect(btn_, "clicked", callback, user_data);
}
