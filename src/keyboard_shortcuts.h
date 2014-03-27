// -*- mode: c++ -*-
#ifndef __keyboard_shortcuts_h_included__
#define __keyboard_shortcuts_h_included__

#include <gtk/gtk.h>

void create_keyboard_shortcut(GtkAccelGroup *accel_group, guint key, GdkModifierType mods, GCallback cb, gpointer user_data);

template<typename F, typename T> static inline void create_keyboard_shortcut(GtkAccelGroup *accel_group, guint key, GdkModifierType mods, F cb, T user_data)
{
	create_keyboard_shortcut(accel_group, key, mods, G_CALLBACK(cb), static_cast<gpointer>(user_data));
}

template<typename F, typename T> static inline void create_keyboard_shortcut(GtkAccelGroup *accel_group, guint key, F cb, T user_data)
{
	create_keyboard_shortcut(accel_group, key, static_cast<GdkModifierType>(0), cb, user_data);
}

#endif
