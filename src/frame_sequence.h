#ifndef __frame_sequence_h_included__
#define __frame_sequence_h_included__

extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>
#include <vector>

class demuxer;

class frame_sequence {
public:
	frame_sequence(int rows, int columns);

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), container_, true, true, 0);
	}

	void show() { gtk_widget_show(container_); }

	void update_sequence(demuxer &dmux);

	void page_backward();
	void page_forward();
	void zoom_in();
	void zoom_out();

private:
	void create_sequence(int rows, int columns);
	void clear_sequence();
	void update_sequence(int64_t start, int64_t step);

	struct thumbnail {
		GtkWidget *img_;
		int64_t pts_;
		int display_picture_number_;

		thumbnail(GtkWidget *container, int row, int column);
		void clear();
		void set_from_avframe(AVFrame *frame);
	};

	GtkWidget *container_;
	std::vector<thumbnail> imgs_;
	demuxer *dmux_;
	int video_stream_index_;
	int64_t current_0_, current_step_;

	static const int THUMBNAIL_WIDTH_ = 200;

	frame_sequence();
	frame_sequence(const frame_sequence &);
	frame_sequence &operator=(const frame_sequence &);
};

#endif
