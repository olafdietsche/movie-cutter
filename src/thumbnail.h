// -*- mode: c++ -*-
#ifndef __thumbnail_h_included__
#define __thumbnail_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>

class thumbnail {
public:
	static GdkPixbuf *gdk_pixbuf_new_from_avframe(AVFrame *frame);

	thumbnail(GtkWidget *container, int row, int column);
	/* FIXME: copy and assignment must look for and adjust connected signals
	 * thumbnail(const thumbnail&);
	 * thumbnail &operator=(const thumbnail&); */

	int64_t get_pts() const { return pts_; }

	void clear();
	void set_from_avframe(AVFrame *frame);
	void connect_clicked(GCallback callback, gpointer user_data);
private:
	GtkWidget *btn_, *img_;
	gulong handler_id_;

	int64_t pts_;
	int display_picture_number_, coded_picture_number_;

	thumbnail();
};

#endif
