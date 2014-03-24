// -*- mode: c++ -*-
#ifndef __thumbnail_h_included__
#define __thumbnail_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>
#include <string>

class thumbnail {
public:
	static const int DEFAULT_WIDTH = 200;

	static GdkPixbuf *gdk_pixbuf_new_from_avframe(AVFrame *frame);

	thumbnail();
	/* FIXME: copy and assignment must look for and adjust connected signals
	 * thumbnail(const thumbnail&);
	 * thumbnail &operator=(const thumbnail&); */

	void destroy() { gtk_widget_destroy(box_); box_ = NULL; }

	void attach(GtkWidget *table, int row, int column) {
		gtk_table_attach_defaults(GTK_TABLE(table), box_, column, column + 1, row, row + 1);
	}

	void pack(GtkWidget *box) {
		gtk_box_pack_start(GTK_BOX(box), box_, false, false, 0);
	}

	void pack(GtkWidget *box, int pos) {
		gtk_box_reorder_child(GTK_BOX(box), box_, pos);
	}

	int64_t get_pts() const { return pts_; }

	void clear();
	const std::string &get_label() const { return label_; }
	void set_from_avframe(AVFrame *frame);
	void set_label(const std::string &s);
	void set_label(AVStream *st);
	void set_from_thumbnail(const thumbnail &frame);
	void connect_clicked(GCallback callback, gpointer user_data);
private:
	GtkWidget *box_;
	GtkWidget *btn_, *img_, *lbl_;
	std::string label_;
	gulong handler_id_;

	int64_t pts_;
	AVPictureType pict_type_;
	int display_picture_number_, coded_picture_number_;
};

#endif
