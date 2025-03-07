#include "hoodie_ops.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieOps::_bind_methods() {
	ClassDB::bind_static_method("HoodieOps", D_METHOD("curve_sweep", "curve", "profile", "closed", "u_dist", "v_dist"), &HoodieOps::curve_sweep, DEFVAL(false), DEFVAL(false), DEFVAL(false));
}

Ref<HoodieGeo> HoodieOps::curve_sweep(const Ref<HoodieGeo> p_curve, const Ref<HoodieGeo> p_profile, const bool p_closed, const bool p_u_dist, const bool p_v_dist) {
	if (p_curve.is_null() || p_profile.is_null()) {
		return Ref<HoodieGeo>();
	}

	const int shape_size = p_profile->get_points().size();
	const int shape_verts_size = shape_size + (p_closed ? 1 : 0);
	const int path_size = p_curve->get_points().size();
	const int total_verts = shape_verts_size * path_size;
	const int total_indices = 2 * 3 * (path_size - 1) * (shape_verts_size - 1);

	if (path_size <= 1) {
		UtilityFunctions::push_warning("Path has not enough points (needs at least 2).");
		return Ref<HoodieGeo>();
	}

	if (shape_size <= 1) {
		UtilityFunctions::push_warning("Shape has not enough points (needs at least 2).");
		return Ref<HoodieGeo>();
	}

	if (p_closed && (shape_size == 2)) {
		UtilityFunctions::push_warning("Shape has not enough points to be closed (needs at least 3).");
		return Ref<HoodieGeo>();
	}

	PackedVector3Array vertices;
	PackedVector3Array normals;
	PackedFloat32Array tangents;
	PackedVector2Array uvs;
	PackedInt32Array indices;

	vertices.resize(total_verts);
	normals.resize(total_verts);
	tangents.resize(total_verts * 4);
	uvs.resize(total_verts);
	indices.resize(total_indices);

	PackedVector3Array curve_pos = p_curve->get_points();
	PackedVector3Array profile_pos = p_profile->get_points();

	PackedVector3Array curve_tan;
	PackedVector3Array curve_nor;
	PackedFloat32Array curve_tilt;
	PackedFloat32Array curve_weight;

	if (p_curve->has_vertex_property("T")) {
		curve_tan = p_curve->get_vertex_property("T");
	} else {
		curve_tan.resize(path_size);
		for (int i = 0; i < path_size - 1; i++) {
			curve_tan[i] = curve_pos[i + 1] - curve_pos[i];
		}
		// TODO: if the path is closed, the tangent should point towards the first point.
		curve_tan[path_size - 1] = curve_tan[path_size - 2];
	}

	if (p_curve->has_vertex_property("N")) {
		curve_nor = p_curve->get_vertex_property("N");
	} else {
		curve_nor.resize(path_size);
		curve_nor.fill(Vector3(0.0, 1.0, 0.0));
	}

	if (p_curve->has_vertex_property("Tilt")) {
		curve_tilt = p_curve->get_vertex_property("Tilt");
	} else {
		curve_tilt.resize(path_size);
		curve_tilt.fill(0.0);
	}

	if (p_curve.ptr()->has_vertex_property("Weight")) {
		curve_weight = p_curve->get_vertex_property("Weight");
	} else {
		curve_weight.resize(path_size);
		curve_weight.fill(1.0);
	}

	float shape_length = 0;
	{
		Vector3 prev_pt = profile_pos[0];
		for (int j = 1; j < shape_verts_size; j++) {
			Vector3 pt = profile_pos[j % shape_size];
			shape_length += (pt - prev_pt).length();
			prev_pt = pt;
		}
	}

	TypedArray<PackedInt32Array> out_faces;

	TypedArray<PackedInt32Array> curve_ids;
	curve_ids = p_curve->get_faces();

	// If there are no faces just connect all the points in a chain.
	if (curve_ids.size() == 0) {
		PackedInt32Array vertices;
		for (int v : p_curve->get_vertices_id()) {
			vertices.push_back(v);
		}
		curve_ids.push_back(vertices);
	}

	for (int i = 0; i < curve_ids.size(); i++) {
		PackedInt32Array in_verts = curve_ids[i];

		// FIXME: Is this correct if the pmp::mesh has deleted points? Are the ids correct?
		PackedVector3Array in_points;
		for (int v : in_verts) {
			in_points.push_back(curve_pos[v]);
		}

		// Calculate path progressive distance for UV U purpose.
		PackedFloat32Array u_distances = progressive_path_distances(in_points);
		PackedFloat32Array v_distances = progressive_path_distances(profile_pos);

		// Extrusion: each quad is a primitive.
		int triangle_index = 0;
		for (int j = 0; j < in_verts.size(); j++) {
			int p = in_verts[j];
			// Construct frame with vectors taken from the Curve3D and tilt.
			Transform3D frame;

			if (j == in_verts.size() - 1 && in_verts.size() > 1) {
				curve_tan[p] = curve_tan[in_verts[j - 1]];
			}

			frame = Transform3D(-curve_tan[p].cross(curve_nor[p]).normalized(), curve_nor[p], curve_tan[p], curve_pos[p]);
			frame.rotate_basis(frame.basis.get_column(2), curve_tilt[p]);

			// Populate arrays.
			for (int s = 0; s < shape_verts_size; s++) {
				PackedInt32Array face_ids;
				face_ids.resize(4);

				int index = p * shape_verts_size + s;
				vertices[index] = frame.xform(profile_pos[s] * curve_weight[p]);
				normals[index] = curve_nor[p];

				float uv_u = p_u_dist ? u_distances[j] : p;
				float uv_v = p_v_dist ? v_distances[s] : s;
				uvs[index] = Vector2(uv_u, uv_v);

				if (j > in_verts.size() - 2)
					continue;
				if (s > shape_verts_size - 2)
					continue;

				int id = p * shape_verts_size + s;

				face_ids[0] = id + shape_verts_size + 1;
				face_ids[1] = id + 1;
				face_ids[2] = id;
				face_ids[3] = id + shape_verts_size;

				out_faces.push_back(face_ids);
			}
		}
	}

	Ref<HoodieGeo> r_hgeo;
	r_hgeo.instantiate();
	r_hgeo->set_points(vertices);
	r_hgeo->add_vertex_property("N", normals);
	r_hgeo->add_vertex_property("UV", uvs);
	r_hgeo->set_faces(out_faces);

	return r_hgeo;
}

PackedFloat32Array HoodieOps::progressive_path_distances(const PackedVector3Array &p_points) {
	PackedFloat32Array distances;
	distances.resize(p_points.size());
	distances.fill(0);

	if (p_points.size() < 1) {
		return distances;
	}

	float dist = 0;
	Vector3 prev_pt = p_points[0];

	for (int j = 1; j < distances.size(); j++) {
		dist += (p_points[j] - prev_pt).length();
		distances[j] = dist;
		prev_pt = p_points[j];
	}

	return distances;
}