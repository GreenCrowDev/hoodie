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

int HoodieGeo::get_points_size() const {
	return mesh->vertices_size();
}

PackedInt32Array HoodieGeo::get_vertices_id() const {
	PackedInt32Array ret;
	for (pmp::Vertex v : mesh->vertices()) {
		ret.append(v.idx());
	}
	return ret;
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

Variant HoodieGeo::get_vertex_property(const String &p_name) const {
	// TexCoord type.
	pmp::VertexProperty<pmp::TexCoord> tex_coord_vp = mesh->get_vertex_property<pmp::TexCoord>("v:" + std::string(p_name.ascii().get_data()));
	if (tex_coord_vp) {
		PackedVector2Array ret;
		for (pmp::Vertex v : mesh->vertices()) {
			ret[v.idx()] = PMPToGodotConverter::tex_coord_to_v2(tex_coord_vp[v]);
		}
		return ret;
	}

	// Normal type.
	pmp::VertexProperty<pmp::Point> normal_vp = mesh->get_vertex_property<pmp::Normal>("v:" + std::string(p_name.ascii().get_data()));
	if (normal_vp) {
		PackedVector3Array ret;
		for (pmp::Vertex v : mesh->vertices()) {
			ret[v.idx()] = PMPToGodotConverter::point_to_v3(normal_vp[v]);
		}
		return ret;
	}

	UtilityFunctions::push_warning("Vertex property type not supported.");
	return Variant();
}

void HoodieGeo::add_vertex_property(const String &p_name, const Variant p_property) {
	// Convert types.
	switch (p_property.get_type()) {
		case Variant::PACKED_VECTOR2_ARRAY: {
			PackedVector2Array vec_prop = p_property;
			pmp::VertexProperty<pmp::TexCoord> vp = mesh->add_vertex_property<pmp::TexCoord>("v:" + std::string(p_name.ascii().get_data()));
			for (pmp::Vertex v : mesh->vertices()) {
				vp[v] = PMPToGodotConverter::v2_to_tex_coord(vec_prop[v.idx()]);
			}
			break;
		}
		case Variant::PACKED_VECTOR3_ARRAY: {
			PackedVector3Array vec_prop = p_property;
			pmp::VertexProperty<pmp::Point> vp = mesh->add_vertex_property<pmp::Normal>("v:" + std::string(p_name.ascii().get_data()));
			for (pmp::Vertex v : mesh->vertices()) {
				vp[v] = PMPToGodotConverter::v3_to_point(vec_prop[v.idx()]);
			}
			break;
		}
		default: {
			UtilityFunctions::push_warning("Vertex property type not supported.");
			return;
		}
	}
}

bool HoodieGeo::has_vertex_property(const String &p_name) const {
	return mesh->has_vertex_property("v:" + std::string(p_name.ascii().get_data()));
}

int HoodieGeo::get_faces_size() const {
	return mesh->faces_size();
}

TypedArray<PackedInt32Array> HoodieGeo::get_faces() const {
	TypedArray<PackedInt32Array> ret;

	for (pmp::Face f : mesh->faces()) {
		PackedInt32Array face_ids;
		for (pmp::Vertex v : mesh->vertices(f)) {
			face_ids.append(v.idx());
		}
		ret.append(face_ids);
	}

	return ret;
}

void HoodieGeo::set_faces(TypedArray<PackedInt32Array> &p_faces) {
	for (int i = 0; i < p_faces.size(); i++) {
		PackedInt32Array face = p_faces[i];
		switch (face.size()) {
			case 4: {
				mesh->add_quad(pmp::Vertex(face[0]), pmp::Vertex(face[1]), pmp::Vertex(face[2]), pmp::Vertex(face[3]));
				break;
			}
			default: {
				std::vector<pmp::Vertex> face_ids;
				for (int j = 0; j < face.size(); j++) {
					face_ids.push_back(pmp::Vertex(face[j]));
				}
				mesh->add_face(face_ids);
				break;
			}
		}
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
