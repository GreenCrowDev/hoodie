#include "pmp_to_godot_converter.h"

using namespace godot;
using namespace greencrow::hoodie;

Vector3 PMPToGodotConverter::point_to_v3(const pmp::Point &p_point) {
	return Vector3(p_point[0], p_point[1], p_point[2]);
}

pmp::Point PMPToGodotConverter::v3_to_point(const Vector3 &p_vector) {
	return pmp::Point(p_vector.x, p_vector.y, p_vector.z);
}

Array PMPToGodotConverter::surface_to_array_mesh(const pmp::SurfaceMesh &p_surface_mesh) {
	Array mesh;
	mesh.resize(ArrayMesh::ARRAY_MAX);

	PackedVector3Array vertices;
	PackedVector3Array normals;
	PackedFloat32Array tangents;
	PackedColorArray colors;
	PackedVector2Array uv;
	PackedVector2Array uv2;
	PackedByteArray custom0;
	PackedByteArray custom1;
	PackedByteArray custom2;
	PackedByteArray custom3;
	PackedInt32Array bones;
	PackedFloat64Array weights;
	PackedInt32Array indices;

	// Vertices.
	pmp::VertexProperty<pmp::Point> points = p_surface_mesh.get_vertex_property<pmp::Point>("v:point");
	for (auto v : p_surface_mesh.vertices()) {
		vertices.append(point_to_v3(points[v]));
	}

	// Normals.
	normals.resize(vertices.size());

	// Tangents.
	tangents.resize(vertices.size() * 4);

	// Colors.
	colors.resize(vertices.size());

	// UV.
	uv.resize(vertices.size());

	// UV2.
	uv2.resize(vertices.size());

	custom0.resize(vertices.size());
	custom1.resize(vertices.size());
	custom2.resize(vertices.size());
	custom3.resize(vertices.size());

	// Bones.
	// TODO: Implement ARRAY_FLAG_USE_8_BONE_WEIGHTS.
	bones.resize(vertices.size() * 4);

	// Weights.
	// TODO: Implement ARRAY_FLAG_USE_8_BONE_WEIGHTS.
	weights.resize(vertices.size() * 4);

	// Indices.
	for (pmp::Face f : p_surface_mesh.faces()) {
		PackedInt32Array ids;

		for (pmp::Vertex v : p_surface_mesh.vertices(f)) {
			ids.append(v.idx());
		}

		switch (ids.size()) {
			case 4:
				indices.append(ids[0]);
				indices.append(ids[3]);
				indices.append(ids[1]);
				indices.append(ids[3]);
				indices.append(ids[2]);
				indices.append(ids[1]);
			default:
				indices.append_array(indices);
		}
	}

	mesh[ArrayMesh::ARRAY_VERTEX] = vertices;
	mesh[ArrayMesh::ARRAY_NORMAL] = normals;
	mesh[ArrayMesh::ARRAY_TANGENT] = tangents;
	mesh[ArrayMesh::ARRAY_COLOR] = colors;
	mesh[ArrayMesh::ARRAY_TEX_UV] = uv;
	mesh[ArrayMesh::ARRAY_TEX_UV2] = uv2;
	// TODO: Implement customs.
	// mesh[ArrayMesh::ARRAY_CUSTOM0] = custom0;
	// mesh[ArrayMesh::ARRAY_CUSTOM1] = custom1;
	// mesh[ArrayMesh::ARRAY_CUSTOM2] = custom2;
	// mesh[ArrayMesh::ARRAY_CUSTOM3] = custom3;
	mesh[ArrayMesh::ARRAY_BONES] = bones;
	mesh[ArrayMesh::ARRAY_WEIGHTS] = weights;
	mesh[ArrayMesh::ARRAY_INDEX] = indices;

	return mesh;
}