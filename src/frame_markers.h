// -*- mode: c++ -*-
#ifndef __frame_markers_h_included__
#define __frame_markers_h_included__

#include "thumbnail.h"
#include <gtk/gtk.h>
#include <vector>

class frame_markers {
public:
	frame_markers();

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), container_, false, false, 0);
	}

	void show() { gtk_widget_show(container_); }
	void add_start_marker(const thumbnail *marker);
	void add_stop_marker(const thumbnail *marker);
	void remove_current_marker();
private:
	struct marker : public thumbnail {
		enum marker_type { start, stop };

		marker(GtkWidget *container, marker_type type)
			: thumbnail(),
			  type_(type) {
			pack(container);
			connect_clicked(G_CALLBACK(marker_select_current), this);
		}
		marker(const marker &x) 
			: thumbnail(x),
			  type_(x.type_) {
			connect_clicked(G_CALLBACK(marker_select_current), this);
		}
		marker &operator=(const marker &x) {
			thumbnail::operator=(x);
			type_ = x.type_;
			connect_clicked(G_CALLBACK(marker_select_current), this);
			return *this;
		}

		void prefix_label();

		marker_type type_;
	};

	void insert_marker(const marker &m);
	void remove_marker(const marker &m);
	void select_current(marker *frame);

	static void marker_select_current(GtkWidget*, marker *frame);

	GtkWidget *container_;
	std::vector<marker> markers_;
	marker *current_marker_;

	frame_markers(const frame_markers &);
	frame_markers &operator=(const frame_markers &);
};

#endif
