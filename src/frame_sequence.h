// -*- mode: c++ -*-
#ifndef __frame_sequence_h_included__
#define __frame_sequence_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include "demuxer.h"
#include "thumbnail.h"
#include <gtk/gtk.h>
#include <deque>
#include <stack>
#include <tuple>
#include <vector>

class frame_sequence {
public:
	frame_sequence(int rows, int columns);

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), container_, true, true, 0);
	}

	void show() { gtk_widget_show(container_); }
	void hide() { gtk_widget_hide(container_); }

	int update_sequence(const char *filename);

	void page_backward();
	void page_forward();
	void zoom_in();
	void zoom_out();
	void zoom_home();

	struct video_frame : public thumbnail {
		video_frame(GtkWidget *container, int row, int column)
			: thumbnail() {
			attach(container, row, column);
			connect_clicked(G_CALLBACK(sequence_goto_frame), this);
		}
		video_frame(const video_frame &x) 
			: thumbnail(x) {
			connect_clicked(G_CALLBACK(sequence_goto_frame), this);
		}
		video_frame &operator=(const video_frame &x) {
			thumbnail::operator=(x);
			connect_clicked(G_CALLBACK(sequence_goto_frame), this);
			return *this;
		}
	};

	GdkPixbuf *get_current_pixbuf();
	video_frame *get_current_video_frame() { return current_frame_; }
private:
	void create_sequence(int rows, int columns);
	void clear_sequence();
	void update_sequence(int64_t start, int64_t step);

	void goto_frame(video_frame *frame);
	static void sequence_goto_frame(GtkWidget*, video_frame *frame);

	GtkWidget *container_;
	int n_frames_;
	std::vector<video_frame> frames_;
	video_frame *current_frame_;
	std::stack<std::tuple<int64_t, int64_t> > zoom_ins_;

	demuxer dmux_;
	int video_stream_index_;
	int64_t current_0_, current_step_;
	int64_t minimum_step_;

	frame_sequence();
	frame_sequence(const frame_sequence &);
	frame_sequence &operator=(const frame_sequence &);
};

#endif
