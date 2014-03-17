#include "frame_markers.h"
#include <iostream>

frame_markers::frame_markers()
	: container_(gtk_vbox_new(false, 0))
{
}

void frame_markers::add_start_marker(const thumbnail *t)
{
	marker m(container_);
	m.set_from_thumbnail(*t);
	insert_marker(m);
}

void frame_markers::add_stop_marker(const thumbnail *t)
{
	marker m(container_);
	m.set_from_thumbnail(*t);
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
