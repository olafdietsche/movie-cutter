#include "main_screen.h"

static int ROWS = 5, COLUMNS = 5;

main_screen::main_screen()
	: vbox_(gtk_vbox_new(false, 0)),
	  hbox_(gtk_hbox_new(false, 0)),
	  sequence_(ROWS, COLUMNS),
	  bar_(&sequence_)
{
	markers_.pack(hbox_);
	markers_.show();

	sequence_.pack(hbox_);
	sequence_.show();

	gtk_box_pack_start(GTK_BOX(vbox_), hbox_, true, true, 0);
	gtk_widget_show(hbox_);

	bar_.pack(vbox_);
	bar_.show();
}

void main_screen::update(demuxer &dmux)
{
	sequence_.update_sequence(dmux);
}
