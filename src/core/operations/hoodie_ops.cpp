#include "hoodie_ops.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieOps::_bind_methods() {
	ClassDB::bind_static_method("HoodieOps", D_METHOD("noise_reduction", "values", "severity", "loop"), &HoodieOps::noise_reduction, DEFVAL(4), DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("curve_sweep", "curve", "profile", "loop", "u_dist", "v_dist"), &HoodieOps::curve_sweep, DEFVAL(false), DEFVAL(false), DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("points_curvature", "points", "up_vectors", "loop"), &HoodieOps::points_curvature, DEFVAL(false));
}

PackedFloat32Array HoodieOps::noise_reduction(const PackedFloat32Array &p_values, const int p_severity, const bool p_loop) {
	// https://stackoverflow.com/questions/7811761/smoothing-a-2d-line-from-an-array-of-points
	PackedFloat32Array ret;

	for (int i = 0; i < p_values.size(); i++) {
		int start;
		int end;

		if (p_loop) {
			start = i - p_severity;
			end = i + p_severity;
		} else {
			start = Math::max(0, i - p_severity);
			end = Math::min(p_values.size() - 1, (int64_t)(i + p_severity));
		}

		float sum = 0;
		for (int j = start; j < end; j++) {
			int id = (((j) % p_values.size()) + p_values.size()) % p_values.size();
			sum += p_values[id];
		}

		float avg = sum / p_severity * 2;
		ret.push_back(avg);
	}

	return ret;
}

Ref<HoodieGeo> HoodieOps::curve_sweep(const Ref<HoodieGeo> p_curve, const Ref<HoodieGeo> p_profile, const bool p_loop, const bool p_u_dist, const bool p_v_dist) {
	if (p_curve.is_null() || p_profile.is_null()) {
		return Ref<HoodieGeo>();
	}

	const int shape_size = p_profile->get_points().size();
	const int shape_verts_size = shape_size;
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

	PackedVector3Array vertices;
	PackedVector3Array normals;
	PackedFloat32Array tangents;
	PackedVector2Array uvs;
	PackedColorArray colors;
	PackedInt32Array indices;

	vertices.resize(total_verts);
	normals.resize(total_verts);
	tangents.resize(total_verts * 4);
	uvs.resize(total_verts);
	colors.resize(total_verts);
	indices.resize(total_indices);

	PackedVector3Array curve_pos = p_curve->get_points();
	PackedVector3Array profile_pos = p_profile->get_points();

	PackedVector3Array curve_tan;
	PackedVector3Array curve_nor;
	PackedFloat32Array curve_tilt;
	PackedFloat32Array curve_weight;
	PackedColorArray curve_color;

	if (p_curve->has_vertex_property("T")) {
		curve_tan = p_curve->get_vertex_property("T");
	} else {
		curve_tan.resize(path_size);

		for (int i = 0; i < path_size - 1; i++) {
			curve_tan[i] = (curve_pos[i + 1] - curve_pos[i]).normalized();
		}

		if (p_loop) {
			curve_tan[path_size - 1] = curve_tan[0];
		} else {
			curve_tan[path_size - 1] = curve_tan[path_size - 2];
		}
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

	if (p_curve.ptr()->has_vertex_property("C")) {
		curve_color = p_curve->get_vertex_property("C");
	} else {
		curve_color.resize(path_size);
		curve_color.fill(Color(0, 0, 0));
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

			frame = Transform3D(-curve_tan[p].cross(curve_nor[p]).normalized(), curve_nor[p], curve_tan[p], curve_pos[p]);
			frame.rotate_basis(frame.basis.get_column(2), curve_tilt[p]);

			// Populate arrays.
			for (int s = 0; s < shape_verts_size; s++) {
				PackedInt32Array face_ids;
				face_ids.resize(4);

				int index = p * shape_verts_size + s;
				vertices[index] = frame.xform(profile_pos[s] * curve_weight[p]);
				normals[index] = curve_nor[p];
				colors[index] = curve_color[p];

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
	r_hgeo->add_vertex_property("C", colors);
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

PackedFloat32Array HoodieOps::points_curvature(const PackedVector3Array &p_points, const PackedVector3Array &p_up_vectors, const bool p_loop) {
	PackedVector3Array up_vecs = p_up_vectors;
	up_vecs.resize(p_points.size());

	PackedFloat32Array curvature_values;
	curvature_values.resize(p_points.size());

	if (p_points.size() < 3) {
		UtilityFunctions::push_warning("List has not enough points (needs at least 3).");
		return PackedFloat32Array();
	}

	// https://hratliff.com/posts/2019/02/curvature-of-three-points
	// https://math.stackexchange.com/questions/128991/how-to-calculate-the-area-of-a-3d-triangle
	for (int i = 0; i < p_points.size(); i++) {
		Vector3 a;
		Vector3 b = p_points[i];
		Vector3 c;

		if (i == 0) {
			if (p_loop) {
				a = p_points[p_points.size() - 2];
			} else {
				curvature_values[i] = 0;
				continue;
			}
		} else {
			a = p_points[i - 1];
		}

		if (i == p_points.size() - 1) {
			if (p_loop) {
				curvature_values[i] = curvature_values[0];
			} else {
				curvature_values[i] = 0;
				continue;
			}
		} else {
			c = p_points[i + 1];
		}

		Vector3 bc = c - b;
		Vector3 ba = a - b;
		Vector3 ca = a - c;

		Vector3 cross = ba.cross(bc);

		float area = cross.length() * 0.5;

		float signed_angle = ba.signed_angle_to(bc, up_vecs[i]);
		float sign = Math::sign(signed_angle);

		float curvature = sign * (4 * area) / (bc.length() * ba.length() * ca.length());

		curvature_values[i] = curvature;
	}

	return curvature_values;
}