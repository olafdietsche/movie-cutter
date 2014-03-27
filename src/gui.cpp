extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>
#include "demuxer.h"
#include "main_screen.h"

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	GtkWidget *toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(toplevel), accel_group);

	main_screen main;
	main.pack(toplevel);
	main.create_keyboard_shortcuts(accel_group);
	main.show();
	gtk_widget_show(toplevel);

	g_signal_connect(toplevel, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	av_log_set_level(AV_LOG_INFO);
	av_register_all();
	avformat_network_init();

	if (argc > 1) {
		const char *filename = argv[1];
		main.open_movie(filename);
	}

	gtk_main();
	return 0;
}
