#include "hoodie_geo.h"
#include <core/enums/hoodie_enums.h>
#include <core/utils/pmp_to_godot_converter.h>
#include <pmp/algorithms/shapes.h>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieGeo::_bind_methods() {
}

PackedVector3Array HoodieGeo::get_points() const {
	return PackedVector3Array();
}

void HoodieGeo::set_points(const PackedVector3Array &p_points) {
}

Array HoodieGeo::get_vertex_property(const String &p_name) const {
	return Array();
}

void HoodieGeo::add_vertex_property(const String &p_name, const Array &p_property) {
}

bool HoodieGeo::has_vertex_property(const String &p_name) const {
	return false;
}

HoodieGeo::HoodieGeo() {
}
