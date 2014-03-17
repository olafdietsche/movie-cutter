extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>
#include "demuxer.h"
#include "main_screen.h"

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	main_screen main;
	main.pack(window);
	main.show();
	gtk_widget_show(window);

	g_signal_connect (window, "destroy",
			  G_CALLBACK(gtk_main_quit), NULL);

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
