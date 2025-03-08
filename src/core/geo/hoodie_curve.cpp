#include "hoodie_curve.h"
#include <core/enums/hoodie_enums.h>
#include <core/operations/hoodie_ops.h>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieCurve::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &HoodieCurve::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &HoodieCurve::set_points);

	ClassDB::bind_method(D_METHOD("get_vertex_property", "name"), &HoodieCurve::get_vertex_property);
	ClassDB::bind_method(D_METHOD("add_vertex_property", "name", "property"), &HoodieCurve::add_vertex_property);
	ClassDB::bind_method(D_METHOD("has_vertex_property", "name"), &HoodieCurve::has_vertex_property);

	ClassDB::bind_method(D_METHOD("init_from_curve_3d", "curve", "loop"), &HoodieCurve::init_from_curve_3d, DEFVAL(false));

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "set_points", "get_points");
}

PackedVector3Array HoodieCurve::get_points() const {
	return points;
}

void HoodieCurve::set_points(const PackedVector3Array &p_points) {
	points = p_points;
}

Array HoodieCurve::get_vertex_property(const String &p_name) const {
	if (properties.has(p_name)) {
		return properties[p_name];
	} else {
		return Array();
	}
}

void HoodieCurve::add_vertex_property(const String &p_name, const Array &p_property) {
	properties[p_name] = p_property;
}

bool HoodieCurve::has_vertex_property(const String &p_name) const {
	return properties.has(p_name);
}

void HoodieCurve::init_from_curve_3d(const Ref<Curve3D> &p_curve, const bool p_loop) {
	if (p_curve.is_null()) {
		return;
	}

	points = p_curve->get_baked_points();

	properties["N"] = p_curve->get_baked_up_vectors();
	properties["T"] = HoodieOps::calc_path_tangents(points, p_loop);
	properties["Tilt"] = p_curve->get_baked_tilts();

	baked_length = p_curve->get_baked_length();
}

HoodieCurve::HoodieCurve() {
}