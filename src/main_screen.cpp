#include "main_screen.h"
#include "demuxer.h"
#include "muxer.h"

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

void main_screen::open_movie()
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(vbox_);
	GtkWidget *dlg = gtk_file_chooser_dialog_new("Load File",
						     GTK_WINDOW(toplevel),
						     GTK_FILE_CHOOSER_ACTION_OPEN,
						     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						     NULL);
	if (gtk_dialog_run(GTK_DIALOG(dlg)) != GTK_RESPONSE_ACCEPT)
		return;

	char *input_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
	open_movie(input_file);
	g_free(input_file);
	gtk_widget_destroy(dlg);
}

void main_screen::open_movie(const char *filename)
{
	input_file_ = filename;
	sequence_.update_sequence(input_file_.c_str());
}

void main_screen::save_movie()
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(vbox_);
	GtkWidget *dlg = gtk_file_chooser_dialog_new("Save File",
						     GTK_WINDOW(toplevel),
						     GTK_FILE_CHOOSER_ACTION_SAVE,
						     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						     NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg), TRUE);
	if (gtk_dialog_run(GTK_DIALOG(dlg)) != GTK_RESPONSE_ACCEPT)
		return;

	char *output_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
	demuxer dmux;
	dmux.open_input(input_file_.c_str());
	muxer mux;
	mux.open_output(output_file, dmux.get_format_context());
	g_free(output_file);
	gtk_widget_destroy(dlg);

	frame_markers::marker_sequence m = markers_.get_markers();
	auto i = m.cbegin();

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	while (i != m.cend() && dmux.read_next_packet(&pkt) >= 0) {
		while (pkt.pts >= i->start_) {
			if (pkt.pts < i->stop_) {
				mux.write_packet(&pkt);
				break;
			} else {
				++i;
				if (i == m.cend())
					break;
			}
		}

		av_free_packet(&pkt);
	}

	// flush cached frames
	dmux.flush(&pkt);
}
