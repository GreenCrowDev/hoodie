#include "hoodie_geo.h"

using namespace godot;
using namespace greencrow::hoodie;

void HoodieGeo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &HoodieGeo::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &HoodieGeo::set_points);
	ClassDB::bind_method(D_METHOD("get_elements"), &HoodieGeo::get_elements);
	ClassDB::bind_method(D_METHOD("set_elements", "elements"), &HoodieGeo::set_elements);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "set_points", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "elements"), "set_elements", "get_elements");
}

PackedVector3Array HoodieGeo::get_points() const {
	return points;
}

void HoodieGeo::set_points(const PackedVector3Array &p_points) {
	points = p_points;
	// TODO: Update attributes depending on new points size.
}

TypedArray<PackedInt32Array> HoodieGeo::get_elements() const {
	return elements;
}

void HoodieGeo::set_elements(const TypedArray<PackedInt32Array> &p_elements) {
	elements = p_elements;
}

HoodieGeo::HoodieGeo() {
}
