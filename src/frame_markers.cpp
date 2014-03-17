#include "frame_markers.h"
#include <iostream>

void frame_markers::marker::prefix_label()
{
	const char *prefix = "";
	switch (type_) {
	case marker_start:
		prefix = "Start - ";
		break;
	case marker_stop:
		prefix = "Stop - ";
		break;
	}

	thumbnail::set_label(prefix + get_label());
}

frame_markers::frame_markers()
	: container_(gtk_vbox_new(false, 0)),
	  current_marker_(0)
{
	g_object_set_data(G_OBJECT(container_), "x-app-object", this);
}

void frame_markers::add_start_marker(const thumbnail *t)
{
	marker m(container_, marker_start);
	m.set_from_thumbnail(*t);
	m.prefix_label();
	insert_marker(m);
}

void frame_markers::add_stop_marker(const thumbnail *t)
{
	marker m(container_, marker_stop);
	m.set_from_thumbnail(*t);
	m.prefix_label();
	insert_marker(m);
}

void frame_markers::insert_marker(const marker &m)
{
	for (auto i = markers_.begin(); i != markers_.end(); ++i) {
		if (m.get_pts() < i->get_pts()) {
			int pos = std::distance(markers_.begin(), i);
			markers_.insert(i, m);
			markers_[pos].pack(container_, pos);
			return;
		}
	}

	markers_.push_back(m);
}

void frame_markers::remove_marker(const marker &m)
{
	for (auto i = markers_.begin(); i != markers_.end(); ++i) {
		if (m.get_pts() == i->get_pts()) {
			i->destroy();
			markers_.erase(i);
			return;
		}
	}
}

void frame_markers::remove_current_marker()
{
	if (current_marker_) {
		remove_marker(*current_marker_);
		current_marker_ = 0;
	}
}

frame_markers::marker_sequence frame_markers::get_markers()
{
	marker_sequence res;
	marker_segment segment = { INT64_MIN, INT64_MAX };
	bool segment_done = true;
	for (auto i = markers_.begin(); i != markers_.end(); ++i) {
		switch (i->type_) {
		case marker_start:
			segment.start_ = i->get_pts();
			segment_done = false;
			break;
		case marker_stop:
			segment.stop_ = i->get_pts();
			res.push_back(segment);
			segment_done = true;
			break;
		}
	}

	if (!segment_done) {
		segment.stop_ = INT64_MAX;
		res.push_back(segment);
	}

	return res;
}

void frame_markers::select_current(marker *frame)
{
	current_marker_ = frame;
}

void frame_markers::marker_select_current(GtkWidget *btn, marker *frame)
{
	GtkWidget *box = gtk_widget_get_parent(btn);
	GtkWidget *parent = gtk_widget_get_parent(box);
	gpointer data = g_object_get_data(G_OBJECT(parent), "x-app-object");
	frame_markers *markers = reinterpret_cast<frame_markers*>(data);
	markers->select_current(frame);
}
