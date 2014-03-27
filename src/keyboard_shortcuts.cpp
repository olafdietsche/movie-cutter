#include "keyboard_shortcuts.h"

void create_keyboard_shortcut(GtkAccelGroup *accel_group, guint key, GdkModifierType mods, GCallback cb, gpointer user_data)
{
	GClosure *closure = g_cclosure_new(G_CALLBACK(cb), user_data, NULL);
	gtk_accel_group_connect(accel_group, key, mods, (GtkAccelFlags) 0, closure);
}
