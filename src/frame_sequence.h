// -*- mode: c++ -*-
#ifndef __frame_sequence_h_included__
#define __frame_sequence_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>
#include <deque>
#include <stack>
#include <tuple>
#include <vector>

class demuxer;

class frame_sequence {
public:
	frame_sequence(int rows, int columns);

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), container_, true, true, 0);
	}

	void show() { gtk_widget_show(container_); }
	void hide() { gtk_widget_hide(container_); }

	void update_sequence(demuxer &dmux);

	void page_backward();
	void page_forward();
	void zoom_in();
	void zoom_out();

	GdkPixbuf *get_current_frame();

private:
	void create_sequence(int rows, int columns);
	void clear_sequence();
	void update_sequence(int64_t start, int64_t step);

	struct thumbnail {
		GtkWidget *btn_, *img_;
		gulong handler_id_;

		int64_t pts_;
		int display_picture_number_, coded_picture_number_;

		thumbnail(GtkWidget *container, int row, int column);
		thumbnail(const thumbnail&);
		void clear();
		void set_from_avframe(AVFrame *frame);
	};

	void goto_frame(thumbnail *img);
	static void sequence_goto_frame(GtkWidget*, thumbnail *img);

	GtkWidget *container_;
	int n_thumbnails_;
	std::vector<thumbnail> imgs_;
	thumbnail *current_frame_;
	std::stack<std::tuple<int64_t, int64_t> > zoom_ins_;

	demuxer *dmux_;
	int video_stream_index_;
	int64_t current_0_, current_step_;
	int64_t minimum_step_;

	static const int THUMBNAIL_WIDTH_ = 200;

	frame_sequence();
	frame_sequence(const frame_sequence &);
	frame_sequence &operator=(const frame_sequence &);
};

#endif
