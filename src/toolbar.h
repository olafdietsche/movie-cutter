#ifndef __toolbar_h_included__
#define __toolbar_h_included__

#include <gtk/gtk.h>

class main_screen;
class frame_sequence;

class toolbar {
public:
	toolbar(main_screen *main, frame_sequence *sequence);

	void pack(GtkWidget *container) {
		gtk_box_pack_start(GTK_BOX(container), toolbar_, false, false, 0);
	}

	void show() { gtk_widget_show_all(toolbar_); }
private:
	void create_toolbar(main_screen *main, frame_sequence *sequence);

	main_screen *main_;
	frame_sequence *sequence_;
	GtkWidget *toolbar_;

	toolbar(const toolbar &);
	toolbar &operator=(const toolbar &);
};

#endif
