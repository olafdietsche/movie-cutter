#ifndef __toolbar_h_included__
#define __toolbar_h_included__

#include "frame_sequence.h"
#include <gtk/gtk.h>

class toolbar {
public:
	toolbar(frame_sequence *sequence);

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), toolbar_, false, false, 0);
	}

	void show() { gtk_widget_show_all(toolbar_); }
private:
	void create_toolbar(frame_sequence *sequence);

	frame_sequence *sequence_;
	GtkWidget *toolbar_;

	toolbar(const toolbar &);
	toolbar &operator=(const toolbar &);
};

#endif
