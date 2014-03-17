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

	thumbnail();
	/* FIXME: copy and assignment must look for and adjust connected signals
	 * thumbnail(const thumbnail&);
	 * thumbnail &operator=(const thumbnail&); */

	void attach(GtkWidget *table, int row, int column) {
		gtk_table_attach_defaults(GTK_TABLE(table), btn_, column, column + 1, row, row + 1);
	}

	void pack(GtkWidget *box) {
		gtk_box_pack_start(GTK_BOX(box), btn_, false, false, 0);
	}

	void pack(GtkWidget *box, int pos) {
		gtk_box_reorder_child(GTK_BOX(box), btn_, pos);
	}

	int64_t get_pts() const { return pts_; }

	void clear();
	void set_from_avframe(AVFrame *frame);
	void set_from_thumbnail(const thumbnail &frame);
	void connect_clicked(GCallback callback, gpointer user_data);
private:
	GtkWidget *btn_, *img_;
	gulong handler_id_;

	int64_t pts_;
	int display_picture_number_, coded_picture_number_;
};

#endif
