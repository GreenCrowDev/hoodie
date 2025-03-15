#include "pmp_to_godot_converter.h"

using namespace godot;
using namespace greencrow::hoodie;

Vector2 PMPToGodotConverter::tex_coord_to_v2(const pmp::TexCoord &p_point) {
	return Vector2(p_point[0], p_point[1]);
}

pmp::TexCoord PMPToGodotConverter::v2_to_tex_coord(const Vector2 &p_vector) {
	return pmp::TexCoord(p_vector.x, p_vector.y);
}

Vector3 PMPToGodotConverter::point_to_v3(const pmp::Point &p_point) {
	return Vector3(p_point[0], p_point[1], p_point[2]);
}

pmp::Point PMPToGodotConverter::v3_to_point(const Vector3 &p_vector) {
	return pmp::Point(p_vector.x, p_vector.y, p_vector.z);
}

Color PMPToGodotConverter::col_to_pmp_col(const pmp::Color &p_color) {
	return Color(p_color[0], p_color[1], p_color[2]);
}

pmp::Color PMPToGodotConverter::pmp_col_to_col(const Color &p_color) {
	return pmp::Color(p_color.r, p_color.g, p_color.b);
}

Color PMPToGodotConverter::point_to_col(const pmp::Point &p_point) {
	return Color(p_point[0], p_point[1], p_point[2]);
}

pmp::Point PMPToGodotConverter::col_to_point(const Color &p_color) {
	return pmp::Point(p_color.r, p_color.g, p_color.b);
}

Array PMPToGodotConverter::surface_to_array_mesh(const pmp::SurfaceMesh &p_surface_mesh) {
	Array mesh;
	mesh.resize(ArrayMesh::ARRAY_MAX);

	const int v_size = p_surface_mesh.vertices_size();

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

	// Normals.
	normals.resize(v_size);

	// Tangents.
	tangents.resize(v_size * 4);

	// Colors.
	pmp::VertexProperty<pmp::Color> surface_colors = p_surface_mesh.get_vertex_property<pmp::Color>("v:C");

	// UV.
	pmp::VertexProperty<pmp::TexCoord> surface_uvs = p_surface_mesh.get_vertex_property<pmp::TexCoord>("v:UV");

	// UV2.
	uv2.resize(v_size);

	custom0.resize(v_size);
	custom1.resize(v_size);
	custom2.resize(v_size);
	custom3.resize(v_size);

	// Bones.
	// TODO: Implement ARRAY_FLAG_USE_8_BONE_WEIGHTS.
	bones.resize(v_size * 4);

	// Weights.
	// TODO: Implement ARRAY_FLAG_USE_8_BONE_WEIGHTS.
	weights.resize(v_size * 4);

	for (pmp::Vertex v : p_surface_mesh.vertices()) {
		vertices.append(point_to_v3(points[v]));

		if (surface_colors) {
			colors.append(point_to_col(surface_colors[v]));
		}

		if (surface_uvs) {
			uv.append(tex_coord_to_v2(surface_uvs[v]));
		}
	}

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
				break;
			default:
				indices.append_array(indices);
				break;
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

Array PMPToGodotConverter::surfaces_to_array_mesh(const std::vector<pmp::SurfaceMesh> &p_surface_meshes) {
	Array ret;
	ret.resize(ArrayMesh::ARRAY_MAX);

	for (pmp::SurfaceMesh m : p_surface_meshes) {
		Array am = surface_to_array_mesh(m);

		PackedVector3Array vertices = ret[ArrayMesh::ARRAY_VERTEX];
		PackedVector3Array normals = ret[ArrayMesh::ARRAY_NORMAL];
		PackedFloat32Array tangents = ret[ArrayMesh::ARRAY_TANGENT];
		PackedColorArray colors = ret[ArrayMesh::ARRAY_COLOR];
		PackedVector2Array uv = ret[ArrayMesh::ARRAY_TEX_UV];
		PackedVector2Array uv2 = ret[ArrayMesh::ARRAY_TEX_UV2];
		PackedByteArray custom0 = ret[ArrayMesh::ARRAY_CUSTOM0];
		PackedByteArray custom1 = ret[ArrayMesh::ARRAY_CUSTOM1];
		PackedByteArray custom2 = ret[ArrayMesh::ARRAY_CUSTOM2];
		PackedByteArray custom3 = ret[ArrayMesh::ARRAY_CUSTOM3];
		PackedInt32Array bones = ret[ArrayMesh::ARRAY_BONES];
		PackedFloat64Array weights = ret[ArrayMesh::ARRAY_WEIGHTS];
		PackedInt32Array indices = ret[ArrayMesh::ARRAY_INDEX];

		const int v_size = vertices.size();

		vertices.append_array(am[ArrayMesh::ARRAY_VERTEX]);
		normals.append_array(am[ArrayMesh::ARRAY_NORMAL]);
		tangents.append_array(am[ArrayMesh::ARRAY_TANGENT]);
		colors.append_array(am[ArrayMesh::ARRAY_COLOR]);
		uv.append_array(am[ArrayMesh::ARRAY_TEX_UV]);
		uv2.append_array(am[ArrayMesh::ARRAY_TEX_UV2]);
		custom0.append_array(am[ArrayMesh::ARRAY_VERTEX]);
		custom1.append_array(am[ArrayMesh::ARRAY_VERTEX]);
		custom2.append_array(am[ArrayMesh::ARRAY_VERTEX]);
		custom3.append_array(am[ArrayMesh::ARRAY_VERTEX]);
		bones.append_array(am[ArrayMesh::ARRAY_BONES]);
		weights.append_array(am[ArrayMesh::ARRAY_WEIGHTS]);

		PackedInt32Array old_ids = am[ArrayMesh::ARRAY_INDEX];
		PackedInt32Array new_ids;
		new_ids.resize(old_ids.size());
		for (int i = 0; i < old_ids.size(); i++) {
			new_ids[i] = old_ids[i] + v_size;
		}
		indices.append_array(new_ids);

		ret[ArrayMesh::ARRAY_VERTEX] = vertices;
		ret[ArrayMesh::ARRAY_NORMAL] = normals;
		ret[ArrayMesh::ARRAY_TANGENT] = tangents;
		ret[ArrayMesh::ARRAY_COLOR] = colors;
		ret[ArrayMesh::ARRAY_TEX_UV] = uv;
		ret[ArrayMesh::ARRAY_TEX_UV2] = uv2;
		// ret[ArrayMesh::ARRAY_CUSTOM0] = custom0;
		// ret[ArrayMesh::ARRAY_CUSTOM1] = custom1;
		// ret[ArrayMesh::ARRAY_CUSTOM2] = custom2;
		// ret[ArrayMesh::ARRAY_CUSTOM3] = custom3;
		ret[ArrayMesh::ARRAY_BONES] = bones;
		ret[ArrayMesh::ARRAY_WEIGHTS] = weights;
		ret[ArrayMesh::ARRAY_INDEX] = indices;
	}

	return ret;
}