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
		gtk_box_pack_start(GTK_BOX(container), scrolled_, false, false, 0);
	}

	void show() { gtk_widget_show(scrolled_); }
	void add_start_marker(const thumbnail *marker);
	void add_stop_marker(const thumbnail *marker);
	void remove_current_marker();
	struct marker_segment {
		int64_t start_, stop_;
	};

	typedef std::vector<marker_segment> marker_sequence;
	marker_sequence get_markers();
private:
	enum marker_type { marker_start, marker_stop };

	struct marker : public thumbnail {
		marker(GtkWidget *container, marker_type type)
			: thumbnail(),
			  type_(type) {
			pack(container);
			connect_clicked(G_CALLBACK(marker_select_marker), this);
		}

		marker(const marker &x) 
			: thumbnail(x),
			  type_(x.type_) {
			connect_clicked(G_CALLBACK(marker_select_marker), this);
		}

		marker &operator=(const marker &x) {
			thumbnail::operator=(x);
			type_ = x.type_;
			connect_clicked(G_CALLBACK(marker_select_marker), this);
			return *this;
		}

		void prefix_label();

		marker_type type_;
	};

	void adjust_scrolled_width();
	void insert_marker(const marker &m);
	void remove_marker(const marker &m);
	void select_marker(marker *frame);

	static void marker_select_marker(GtkWidget *btn, marker *frame);

	GtkWidget *scrolled_, *container_;
	std::vector<marker> markers_;
	marker *current_marker_;

	frame_markers(const frame_markers &);
	frame_markers &operator=(const frame_markers &);
};

#endif
