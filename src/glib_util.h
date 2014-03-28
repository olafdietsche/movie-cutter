// -*- mode: c++ -*-
#ifndef __glib_util_h_included__
#define __glib_util_h_included__

template<typename T, void (T::*method)()> void signal_method_cb(gpointer, T *obj)
{
	(obj->*method)();
}

template<typename F, typename T> static inline gulong signal_connect(gpointer instance, const char *signal_name, F cb, T *user_data)
{
	return g_signal_connect(instance, signal_name, G_CALLBACK(cb), user_data);
}

#endif
