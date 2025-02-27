#include "hoodie_geo.h"

using namespace godot;
using namespace greencrow::hoodie;

void HoodieGeo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &HoodieGeo::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &HoodieGeo::set_points);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "set_points", "get_points");
}

PackedVector3Array HoodieGeo::get_points() const {
	return points;
}

void HoodieGeo::set_points(const PackedVector3Array &p_points) {
	points = p_points;
	// TODO: Update attributes depending on new points size.
}

HoodieGeo::HoodieGeo() {
}