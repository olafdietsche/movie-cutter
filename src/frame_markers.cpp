#include "frame_markers.h"
#include <iostream>

void frame_markers::marker::prefix_label()
{
	const char *prefix = "";
	switch (type_) {
	case start:
		prefix = "Start - ";
		break;
	case stop:
		prefix = "Stop - ";
		break;
	}

	thumbnail::set_label(prefix + get_label());
}

frame_markers::frame_markers()
	: container_(gtk_vbox_new(false, 0))
{
}

void frame_markers::add_start_marker(const thumbnail *t)
{
	marker m(container_, marker::start);
	m.set_from_thumbnail(*t);
	m.prefix_label();
	insert_marker(m);
}

void frame_markers::add_stop_marker(const thumbnail *t)
{
	marker m(container_, marker::stop);
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
