#include "hoodie_geo.h"
#include <core/utils/pmp_to_godot_converter.h>
#include <pmp/algorithms/shapes.h>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieGeo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &HoodieGeo::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &HoodieGeo::set_points);
	ClassDB::bind_method(D_METHOD("init_plane", "resolution"), &HoodieGeo::init_plane, DEFVAL(4));
	ClassDB::bind_method(D_METHOD("to_array_mesh"), &HoodieGeo::to_array_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "set_points", "get_points");
}

PackedVector3Array HoodieGeo::get_points() const {
	PackedVector3Array ret;

	pmp::VertexProperty<pmp::Point> points = mesh->get_vertex_property<pmp::Point>("v:point");
	for (pmp::Vertex v : mesh->vertices()) {
		ret.append(PMPToGodotConverter::point_to_v3(points[v]));
	}

	return ret;
}

void HoodieGeo::set_points(const PackedVector3Array &p_points) {
	mesh->clear();

	for (Vector3 p : p_points) {
		mesh->add_vertex(PMPToGodotConverter::v3_to_point(p));
	}
}

void HoodieGeo::init_plane(int p_resolution) {
	mesh = std::make_unique<pmp::SurfaceMesh>(pmp::plane(p_resolution));
}

Array HoodieGeo::to_array_mesh() const {
	return PMPToGodotConverter::surface_to_array_mesh(*mesh);
}

HoodieGeo::HoodieGeo() : mesh(std::make_unique<pmp::SurfaceMesh>()) {
}
