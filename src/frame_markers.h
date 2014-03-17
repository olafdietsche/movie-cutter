#ifndef __frame_markers_h_included__
#define __frame_markers_h_included__

#include <gtk/gtk.h>

class frame_markers {
public:
	frame_markers() : markers_(gtk_vbox_new(true, 0)) {}

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), markers_, false, false, 0);
	}

	void show() { gtk_widget_show(markers_); }
private:
	GtkWidget *markers_;

	frame_markers(const frame_markers &);
	frame_markers &operator=(const frame_markers &);
};

#endif
