extern "C" {
#include "libavformat/avformat.h"
}

#include <gtk/gtk.h>
#include "demuxer.h"
#include <fstream>
#include <sstream>
#include <vector>

std::vector<GtkWidget*> thumbnails;

static void setup_gui()
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *hbox = gtk_hbox_new(false, 0);
	GtkWidget *markers = gtk_vbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(hbox), markers, false, false, 0);
	gtk_widget_show(markers);
	guint rows = 4, columns = 5;
	GtkWidget *grid = gtk_table_new(rows, columns, 0);
	for (guint i = 0; i < rows; ++i) {
		for (guint j = 0; j < columns; ++j) {
			GtkWidget *tn = gtk_image_new();
			thumbnails.push_back(tn);
			gtk_table_attach_defaults(GTK_TABLE(grid), tn, j, j + 1, i, i + 1);
			gtk_widget_show(tn);
		}
	}

	gtk_box_pack_start(GTK_BOX(hbox), grid, true, true, 0);
	gtk_widget_show(grid);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);
	gtk_widget_show(window);

	g_signal_connect (window, "destroy",
			  G_CALLBACK(gtk_main_quit), NULL);
}

static void write_ppm_image(AVFrame *frame, int n)
{
	std::ostringstream name;
	name << "img" << n << ".ppm";
	std::ofstream f(name.str().c_str());
	// write header
	f << "P6\n" << frame->width << ' ' << frame->height << "\n255\n";
	f.write(reinterpret_cast<const char*>(frame->data[0]), frame->linesize[0]);
}

void setup_video(const char *filename)
{
	demuxer dmux(filename);
	int64_t pts = 0;
	for (auto i = thumbnails.begin(); i != thumbnails.end(); ++i) {
		AVFrame *f = dmux.seek(0, pts);
		pts += 30 * AV_TIME_BASE;
		if (!f)
			continue;

		AVFrame *dest = dmux.scale_frame(0, f, 200);
		//write_ppm_image(dest, std::distance(thumbnails.begin(), i));
		GdkPixbuf *buf = gdk_pixbuf_new_from_data(dest->data[0],
							  GDK_COLORSPACE_RGB,
							  false, 8, dest->width, dest->height,
							  dest->linesize[0], NULL, NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(*i), buf);
	}
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	setup_gui();

	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	av_log_set_level(AV_LOG_INFO);
	av_register_all();
	avformat_network_init();

	setup_video(argv[1]);

	gtk_main();
	return 0;
}
