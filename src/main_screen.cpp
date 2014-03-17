#include "main_screen.h"

static int ROWS = 5, COLUMNS = 5;

main_screen::main_screen()
	: vbox_(gtk_vbox_new(false, 0)),
	  hbox_(gtk_hbox_new(false, 0)),
	  full_image_(gtk_image_new()),
	  sequence_(ROWS, COLUMNS),
	  bar_(this, &markers_, &sequence_)
{
	markers_.pack(hbox_);
	markers_.show();

	gtk_box_pack_start(GTK_BOX(hbox_), full_image_, true, true, 0);

	sequence_.pack(hbox_);
	sequence_.show();

	gtk_box_pack_start(GTK_BOX(vbox_), hbox_, true, true, 0);
	gtk_widget_show(hbox_);

	bar_.pack(vbox_);
	bar_.show();
}

void main_screen::update(const char *filename)
{
	sequence_.update_sequence(filename);
}

void main_screen::fullscreen()
{
	GdkPixbuf *buf = sequence_.get_current_pixbuf();
	gtk_image_set_from_pixbuf(GTK_IMAGE(full_image_), buf);
	gtk_widget_show(full_image_);
	sequence_.hide();
}

void main_screen::leave_fullscreen()
{
	sequence_.show();
	gtk_widget_hide(full_image_);
}

void main_screen::add_start_marker()
{
	frame_sequence::video_frame *frame = sequence_.get_current_video_frame();
	markers_.add_start_marker(frame);
}

void main_screen::add_stop_marker()
{
	frame_sequence::video_frame *frame = sequence_.get_current_video_frame();
	markers_.add_stop_marker(frame);
}
