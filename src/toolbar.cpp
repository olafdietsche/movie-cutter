#include "toolbar.h"
#include "frame_markers.h"
#include "frame_sequence.h"
#include "main_screen.h"
#include "glib_util.h"

toolbar::toolbar(main_screen *main, frame_markers *markers, frame_sequence *sequence)
{
	create_toolbar(main, markers, sequence);
}

void toolbar::create_toolbar(main_screen *main, frame_markers *markers, frame_sequence *sequence)
{
	toolbar_ = gtk_toolbar_new();
	GtkToolItem *toolitem, *separator;

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::open_movie>, main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::save_movie>, main);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::add_start_marker>, main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::add_stop_marker>, main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::add_bookmark>, main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_markers, &frame_markers::remove_current_marker>, markers);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::goto_first_frame>, sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_REWIND);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::page_backward>, sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_FORWARD);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::page_forward>, sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::goto_last_frame>, sequence);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_IN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::zoom_in>, sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_OUT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::zoom_out>, sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<frame_sequence, &frame_sequence::zoom_home>, sequence);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::fullscreen>, main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	signal_connect(toolitem, "clicked", signal_method_cb<main_screen, &main_screen::leave_fullscreen>, main);
}
