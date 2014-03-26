#include "toolbar.h"
#include "frame_markers.h"
#include "frame_sequence.h"
#include "main_screen.h"

namespace {
void sequence_goto_first(GtkWidget*, frame_sequence *sequence)
{
	sequence->goto_first_frame();
}

void sequence_backward(GtkWidget*, frame_sequence *sequence)
{
	sequence->page_backward();
}

void sequence_forward(GtkWidget*, frame_sequence *sequence)
{
	sequence->page_forward();
}

void sequence_goto_last(GtkWidget*, frame_sequence *sequence)
{
	sequence->goto_last_frame();
}

void sequence_zoomin(GtkWidget*, frame_sequence *sequence)
{
	sequence->zoom_in();
}

void sequence_zoomout(GtkWidget*, frame_sequence *sequence)
{
	sequence->zoom_out();
}

void sequence_zoom_home(GtkWidget*, frame_sequence *sequence)
{
	sequence->zoom_home();
}

void main_fullscreen(GtkWidget*, main_screen *main)
{
	main->fullscreen();
}

void main_leave_fullscreen(GtkWidget*, main_screen *main)
{
	main->leave_fullscreen();
}

void marker_add_start(GtkWidget*, main_screen *main)
{
	main->add_start_marker();
}

void marker_add_stop(GtkWidget*, main_screen *main)
{
	main->add_stop_marker();
}

void marker_delete(GtkWidget*, frame_markers *markers)
{
	markers->remove_current_marker();
}

void open_movie(GtkWidget*, main_screen *main)
{
	main->open_movie();
}

void save_movie(GtkWidget*, main_screen *main)
{
	main->save_movie();
}
}

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
	g_signal_connect(toolitem, "clicked", G_CALLBACK(open_movie), main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(save_movie), main);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(marker_add_start), main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(marker_add_stop), main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(marker_delete), markers);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_goto_first), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_REWIND);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_backward), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_FORWARD);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_forward), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_goto_last), sequence);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_IN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_zoomin), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_OUT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_zoomout), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_zoom_home), sequence);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(main_fullscreen), main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(main_leave_fullscreen), main);
}
