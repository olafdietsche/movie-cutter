// -*- mode: c++ -*-
#ifndef __main_screen_h_included__
#define __main_screen_h_included__

#include "frame_markers.h"
#include "frame_sequence.h"
#include "toolbar.h"
#include <string>

class main_screen {
public:
	main_screen();

	void pack(GtkWidget *container) {
		gtk_container_add(GTK_CONTAINER(container), vbox_);
	}

	void show() { gtk_widget_show(vbox_); }

	void fullscreen();
	void leave_fullscreen();

	void add_start_marker();
	void add_stop_marker();
	void add_bookmark();

	void open_movie();
	void open_movie(const char *filename);
	void save_movie();
	void save_movie(const char *output_file);
private:
	void create_widgets();

	GtkWidget *vbox_;
	GtkWidget *hbox_;
	GtkWidget *full_image_;

	std::string input_file_;

	frame_markers markers_;
	frame_sequence sequence_;
	toolbar bar_;

	main_screen(const main_screen &);
	main_screen &operator=(const main_screen &);
};

#endif
