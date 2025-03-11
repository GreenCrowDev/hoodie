#include "hoodie_mesh.h"
#include <core/enums/hoodie_enums.h>
#include <core/operations/hoodie_ops.h>
#include <core/utils/pmp_to_godot_converter.h>
#include <pmp/algorithms/shapes.h>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &HoodieMesh::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &HoodieMesh::set_points);

	ClassDB::bind_method(D_METHOD("get_vertices_id"), &HoodieMesh::get_vertices_id);

	ClassDB::bind_method(D_METHOD("get_vertex_property", "name"), &HoodieMesh::get_vertex_property);
	ClassDB::bind_method(D_METHOD("add_vertex_property", "name", "property"), &HoodieMesh::add_vertex_property);
	ClassDB::bind_method(D_METHOD("has_vertex_property", "name"), &HoodieMesh::has_vertex_property);

	ClassDB::bind_method(D_METHOD("get_faces_size"), &HoodieMesh::get_faces_size);
	ClassDB::bind_method(D_METHOD("get_faces"), &HoodieMesh::get_faces);
	ClassDB::bind_method(D_METHOD("set_faces", "faces"), &HoodieMesh::set_faces);

	ClassDB::bind_method(D_METHOD("init_plane", "resolution"), &HoodieMesh::init_plane, DEFVAL(4));

	ClassDB::bind_method(D_METHOD("to_array_mesh"), &HoodieMesh::to_array_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "set_points", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "faces"), "set_faces", "get_faces");
}

PackedVector3Array HoodieMesh::get_points() const {
	PackedVector3Array ret;

	pmp::VertexProperty<pmp::Point> points = mesh->get_vertex_property<pmp::Point>("v:point");
	for (pmp::Vertex v : mesh->vertices()) {
		ret.append(PMPToGodotConverter::point_to_v3(points[v]));
	}

	return ret;
}

void HoodieMesh::set_points(const PackedVector3Array &p_points) {
	mesh->clear();

	for (Vector3 p : p_points) {
		mesh->add_vertex(PMPToGodotConverter::v3_to_point(p));
	}
}

PackedInt32Array HoodieMesh::get_vertices_id() const {
	PackedInt32Array ret;
	for (pmp::Vertex v : mesh->vertices()) {
		ret.append(v.idx());
	}
	return ret;
}

void HoodieMesh::transform_mesh(const Vector3 &p_displacement, const Vector3 &p_rotation, const Vector3 &p_scale) {
	Quaternion rot_quaternion = HoodieOps::from_euler(p_rotation);
	Basis basis = Basis(rot_quaternion, p_scale);
	Transform3D transform = Transform3D(basis, p_displacement);

	pmp::VertexProperty<pmp::Point> points = mesh->get_vertex_property<pmp::Point>("v:point");
	for (pmp::Vertex v : mesh->vertices()) {
		Vector3 trans_pos = transform.xform(PMPToGodotConverter::point_to_v3(points[v]));
		points[v] = PMPToGodotConverter::v3_to_point(trans_pos);
	}
}

Array HoodieMesh::get_vertex_property(const String &p_name) const {
	PropertyKeyword pk = name_to_property_keyword(p_name);

	if (pk == PropertyKeyword::POINT) {
		return Variant();
	} else if (pk == PropertyKeyword::NORMAL) {
		pmp::VertexProperty<pmp::Normal> normal_vp = mesh->get_vertex_property<pmp::Normal>("v:" + std::string(p_name.ascii().get_data()));
		if (normal_vp) {
			PackedVector3Array ret;
			ret.resize(mesh->vertices_size());
			for (pmp::Vertex v : mesh->vertices()) {
				ret[v.idx()] = PMPToGodotConverter::point_to_v3(normal_vp[v]);
			}
			return ret;
		}
	} else if (pk == PropertyKeyword::TANGENT) {
		return Variant();
	} else if (pk == PropertyKeyword::COLOR) {
		pmp::VertexProperty<pmp::Color> color_vp = mesh->get_vertex_property<pmp::Color>("v:" + std::string(p_name.ascii().get_data()));
		if (color_vp) {
			PackedColorArray ret;
			ret.resize(mesh->vertices_size());
			for (pmp::Vertex v : mesh->vertices()) {
				ret[v.idx()] = PMPToGodotConverter::point_to_col(color_vp[v]);
			}
			return ret;
		}
		return Variant();
	} else if (pk == PropertyKeyword::UV) {
		pmp::VertexProperty<pmp::TexCoord> tex_coord_vp = mesh->get_vertex_property<pmp::TexCoord>("v:" + std::string(p_name.ascii().get_data()));
		if (tex_coord_vp) {
			PackedVector2Array ret;
			ret.resize(mesh->vertices_size());
			for (pmp::Vertex v : mesh->vertices()) {
				ret[v.idx()] = PMPToGodotConverter::tex_coord_to_v2(tex_coord_vp[v]);
			}
			return ret;
		}
		return Variant();
	} else if (pk == PropertyKeyword::UV2) {
		return Variant();
	} else {
		UtilityFunctions::push_warning("Vertex property type not supported.");
		return Variant();
	}
}

void HoodieMesh::add_vertex_property(const String &p_name, const Array &p_property) {
	PropertyKeyword pk = name_to_property_keyword(p_name);

	if (pk == PropertyKeyword::POINT) {
		return;
	} else if (pk == PropertyKeyword::NORMAL) {
		PackedVector3Array vec_prop = p_property;
		if (!vec_prop) {
			return;
		}
		pmp::VertexProperty<pmp::Point> vp = mesh->add_vertex_property<pmp::Point>("v:" + std::string(p_name.ascii().get_data()));
		for (pmp::Vertex v : mesh->vertices()) {
			vp[v] = PMPToGodotConverter::v3_to_point(vec_prop[v.idx()]);
		}
	} else if (pk == PropertyKeyword::TANGENT) {
		return;
	} else if (pk == PropertyKeyword::COLOR) {
		PackedColorArray col_prop = p_property;
		if (!col_prop) {
			return;
		}
		pmp::VertexProperty<pmp::Color> vp = mesh->add_vertex_property<pmp::Color>("v:" + std::string(p_name.ascii().get_data()));
		for (pmp::Vertex v : mesh->vertices()) {
			vp[v] = PMPToGodotConverter::col_to_point(col_prop[v.idx()]);
		}
	} else if (pk == PropertyKeyword::UV) {
		PackedVector2Array vec_prop = p_property;
		if (!vec_prop) {
			return;
		}
		pmp::VertexProperty<pmp::TexCoord> vp = mesh->add_vertex_property<pmp::TexCoord>("v:" + std::string(p_name.ascii().get_data()));
		for (pmp::Vertex v : mesh->vertices()) {
			vp[v] = PMPToGodotConverter::v2_to_tex_coord(vec_prop[v.idx()]);
		}
	} else if (pk == PropertyKeyword::UV2) {
		return;
	} else {
		UtilityFunctions::push_warning("Vertex property type not supported.");
		return;
	}
}

bool HoodieMesh::has_vertex_property(const String &p_name) const {
	return mesh->has_vertex_property("v:" + std::string(p_name.ascii().get_data()));
}

int HoodieMesh::get_faces_size() const {
	return mesh->faces_size();
}

TypedArray<PackedInt32Array> HoodieMesh::get_faces() const {
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

void HoodieMesh::set_faces(const TypedArray<PackedInt32Array> &p_faces) {
	for (int i = 0; i < p_faces.size(); i++) {
		PackedInt32Array face = p_faces[i];
		switch (face.size()) {
			case 4: {
				for (int i = 0; i < 4; i++) {
					if (face[i] > mesh->vertices_size()) {
						UtilityFunctions::push_warning("Face id out of bounds.");
						break;
					}
				}
				mesh->add_quad(pmp::Vertex(face[0]), pmp::Vertex(face[1]), pmp::Vertex(face[2]), pmp::Vertex(face[3]));
				break;
			}
			default: {
				std::vector<pmp::Vertex> face_ids;
				for (int j = 0; j < face.size(); j++) {
					face_ids.push_back(pmp::Vertex(face[j]));
				}
				if (face_ids.size() > 0) {
					// FIXME: You can't add faces with less than 3 vertices.
					mesh->add_face(face_ids);
				}
				break;
			}
		}
	}
}

void HoodieMesh::init_plane(int p_resolution) {
	mesh = std::make_unique<pmp::SurfaceMesh>(pmp::plane(p_resolution));
}

Array HoodieMesh::to_array_mesh() const {
	return PMPToGodotConverter::surface_to_array_mesh(*mesh);
}

void HoodieMesh::_assign_mesh(const pmp::SurfaceMesh &p_mesh) {
	mesh = std::make_unique<pmp::SurfaceMesh>(p_mesh);
}

Ref<HoodieGeo> HoodieMesh::duplicate() const {
	Ref<HoodieMesh> ret;
	ret.instantiate();

	ret->_assign_mesh(*mesh);

	return ret;
}

HoodieMesh::HoodieMesh() : mesh(std::make_unique<pmp::SurfaceMesh>()) {
}
