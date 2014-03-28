#include "main_screen.h"
#include "demuxer.h"
#include "muxer.h"
#include "keyboard_shortcuts.h"

namespace {
int ROWS = 5, COLUMNS = 5;
}

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

void main_screen::add_bookmark()
{
	frame_sequence::video_frame *frame = sequence_.get_current_video_frame();
	markers_.add_bookmark(frame);
}

void main_screen::create_keyboard_shortcuts(GtkAccelGroup *accel_group)
{
	sequence_.create_keyboard_shortcuts(accel_group);
	create_keyboard_shortcut(accel_group, 'o', GDK_CONTROL_MASK, accel_method_cb<main_screen, &main_screen::open_movie>, this);
	create_keyboard_shortcut(accel_group, 's', GDK_CONTROL_MASK, accel_method_cb<main_screen, &main_screen::save_movie>, this);
	create_keyboard_shortcut(accel_group, ',', accel_method_cb<main_screen, &main_screen::add_start_marker>, this);
	create_keyboard_shortcut(accel_group, '.', accel_method_cb<main_screen, &main_screen::add_stop_marker>, this);
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
	if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
		char *input_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		open_movie(input_file);
		g_free(input_file);
	}

	gtk_widget_destroy(dlg);
}

void main_screen::open_movie(const char *filename)
{
	input_file_ = filename;
	int err = sequence_.update_sequence(input_file_.c_str());
	if (err < 0) {
		GtkWidget *toplevel = gtk_widget_get_toplevel(vbox_);
		GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(toplevel),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_CLOSE,
							"Error loading %s: %s",
							input_file_.c_str(),
							av_err2str(err));
		gtk_dialog_run(GTK_DIALOG(dlg));
	 	gtk_widget_destroy(dlg);
	}
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
	if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
		char *output_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		save_movie(output_file);
		g_free(output_file);
	}

	gtk_widget_destroy(dlg);
}

void main_screen::save_movie(const char *output_file)
{
	demuxer dmux;
	dmux.open_input(input_file_.c_str());
	muxer mux;
	mux.open_output(output_file, dmux.get_format_context());

	frame_markers::marker_sequence m = markers_.get_markers();
	auto i = m.cbegin();

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	int64_t offset = i->start_;
	if (offset == INT64_MIN)
		offset = 0;

	while (i != m.cend() && dmux.read_next_packet(&pkt) >= 0) {
		while (pkt.pts >= i->start_) {
			if (pkt.pts < i->stop_) {
				pkt.pts -= offset;
				pkt.dts -= offset;
				mux.write_packet(&pkt);
				break;
			} else {
				int64_t stop = i->stop_;
				++i;
				if (i == m.cend())
					break;

				offset += i->start_ - stop;
			}
		}

		av_free_packet(&pkt);
	}

	// flush cached frames
	dmux.flush(&pkt);
}
