#include "toolbar.h"
#include "frame_sequence.h"
#include "main_screen.h"

namespace {
void sequence_backward(GtkWidget*, frame_sequence *sequence)
{
	sequence->page_backward();
}

void sequence_forward(GtkWidget*, frame_sequence *sequence)
{
	sequence->page_forward();
}

void sequence_zoomin(GtkWidget*, frame_sequence *sequence)
{
	sequence->zoom_in();
}

void sequence_zoomout(GtkWidget*, frame_sequence *sequence)
{
	sequence->zoom_out();
}

void main_fullscreen(GtkWidget*, main_screen *main)
{
	main->fullscreen();
}

void main_leave_fullscreen(GtkWidget*, main_screen *main)
{
	main->leave_fullscreen();
}
}

toolbar::toolbar(main_screen *main, frame_sequence *sequence)
	: main_(main),
	  sequence_(sequence)
{
	create_toolbar(main, sequence);
}

void toolbar::create_toolbar(main_screen *main, frame_sequence *sequence)
{
	toolbar_ = gtk_toolbar_new();
	GtkToolItem *toolitem, *separator;

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_REWIND);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_FORWARD);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_backward), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_forward), sequence);

	separator = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), separator, -1);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_IN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_zoomin), sequence);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_OUT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(sequence_zoomout), sequence);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(main_fullscreen), main);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar_), toolitem, -1);
	g_signal_connect(toolitem, "clicked", G_CALLBACK(main_leave_fullscreen), main);
}
