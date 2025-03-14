#include "hoodie_ops.h"
#include <core/utils/pmp_to_godot_converter.h>
#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/classes/geometry3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace greencrow::hoodie;

void HoodieOps::_bind_methods() {
	ClassDB::bind_static_method("HoodieOps", D_METHOD("noise_reduction", "values", "severity", "loop"), &HoodieOps::noise_reduction, DEFVAL(4), DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("curve_sweep", "curve", "profile", "loop", "u_dist", "v_dist"), &HoodieOps::curve_sweep, DEFVAL(false), DEFVAL(false), DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("curve_sweep_convex_collision_points", "curve", "profile", "loop"), &HoodieOps::curve_sweep_convex_collision_points, DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("progressive_path_distances", "points"), &HoodieOps::progressive_path_distances);
	ClassDB::bind_static_method("HoodieOps", D_METHOD("calc_path_tangents", "points", "loop"), &HoodieOps::calc_path_tangents, DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("points_curvature", "points", "up_vectors", "loop"), &HoodieOps::points_curvature, DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("break_path", "ids", "filter"), &HoodieOps::break_path);
	ClassDB::bind_static_method("HoodieOps", D_METHOD("curve_offset", "curve", "delta", "normal", "polygon"), &HoodieOps::curve_offset, DEFVAL(Vector3(0, 1, 0)), DEFVAL(false));
	ClassDB::bind_static_method("HoodieOps", D_METHOD("equidistant_curve_sampling", "curve", "distance"), &HoodieOps::equidistant_curve_sampling);
	ClassDB::bind_static_method("HoodieOps", D_METHOD("surfaces_to_mesh", "meshes"), &HoodieOps::surfaces_to_mesh);
	ClassDB::bind_static_method("HoodieOps", D_METHOD("copy_mesh", "mesh", "displacement", "rotation", "scale"), &HoodieOps::copy_mesh);
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

Quaternion HoodieOps::from_euler(const Vector3 &p_euler) {
	real_t half_a1 = p_euler.y * 0.5f;
	real_t half_a2 = p_euler.x * 0.5f;
	real_t half_a3 = p_euler.z * 0.5f;

	real_t cos_a1 = Math::cos(half_a1);
	real_t sin_a1 = Math::sin(half_a1);
	real_t cos_a2 = Math::cos(half_a2);
	real_t sin_a2 = Math::sin(half_a2);
	real_t cos_a3 = Math::cos(half_a3);
	real_t sin_a3 = Math::sin(half_a3);

	return Quaternion(
			sin_a1 * cos_a2 * sin_a3 + cos_a1 * sin_a2 * cos_a3,
			sin_a1 * cos_a2 * cos_a3 - cos_a1 * sin_a2 * sin_a3,
			-sin_a1 * sin_a2 * cos_a3 + cos_a1 * cos_a2 * sin_a3,
			sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
}

Ref<HoodieMesh> HoodieOps::curve_sweep(const Ref<HoodieCurve> p_curve, const Ref<HoodieCurve> p_profile, const bool p_loop, const bool p_u_dist, const bool p_v_dist) {
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
		curve_tan = calc_path_tangents(curve_pos, p_loop);
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

	const PackedVector3Array in_points = p_curve->get_points();

	// Calculate path progressive distance for UV U purpose.
	const PackedFloat32Array u_distances = progressive_path_distances(in_points);
	const PackedFloat32Array v_distances = progressive_path_distances(profile_pos);

	// Extrusion: each quad is a primitive.
	for (int j = 0; j < in_points.size(); j++) {
		// Construct frame with vectors taken from the Curve3D and tilt.
		Transform3D frame;

		frame = Transform3D(-curve_tan[j].cross(curve_nor[j]).normalized(), curve_nor[j], curve_tan[j], curve_pos[j]);
		frame.rotate_basis(frame.basis.get_column(2), curve_tilt[j]);

		// Populate arrays.
		for (int s = 0; s < shape_verts_size; s++) {
			PackedInt32Array face_ids;
			face_ids.resize(4);

			int index = j * shape_verts_size + s;
			vertices[index] = frame.xform(profile_pos[s] * curve_weight[j]);
			normals[index] = curve_nor[j];
			colors[index] = curve_color[j];

			float uv_u = p_u_dist ? u_distances[j] : j;
			float uv_v = p_v_dist ? v_distances[s] : s;
			uvs[index] = Vector2(uv_u, uv_v);

			if (j > in_points.size() - 2)
				continue;
			if (s > shape_verts_size - 2)
				continue;

			int id = j * shape_verts_size + s;

			face_ids[0] = id + shape_verts_size + 1;
			face_ids[1] = id + 1;
			face_ids[2] = id;
			face_ids[3] = id + shape_verts_size;

			out_faces.push_back(face_ids);
		}
	}

	Ref<HoodieMesh> r_hm;
	r_hm.instantiate();
	r_hm->set_points(vertices);
	r_hm->add_vertex_property("N", normals);
	r_hm->add_vertex_property("UV", uvs);
	r_hm->add_vertex_property("C", colors);
	r_hm->set_faces(out_faces);

	return r_hm;
}

TypedArray<PackedVector3Array> HoodieOps::curve_sweep_convex_collision_points(Ref<HoodieCurve> p_curve, Ref<HoodieCurve> p_profile, const bool p_loop) {
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
		curve_tan = calc_path_tangents(curve_pos, p_loop);
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

	TypedArray<PackedVector3Array> ret;

	const PackedVector3Array in_points = p_curve->get_points();

	// Calculate path progressive distance for UV U purpose.
	const PackedFloat32Array u_distances = progressive_path_distances(in_points);
	const PackedFloat32Array v_distances = progressive_path_distances(profile_pos);

	// Extrusion: each quad is a primitive.
	for (int j = 0; j < in_points.size(); j++) {
		// Construct frame with vectors taken from the Curve3D and tilt.
		Transform3D frame;

		frame = Transform3D(-curve_tan[j].cross(curve_nor[j]).normalized(), curve_nor[j], curve_tan[j], curve_pos[j]);
		frame.rotate_basis(frame.basis.get_column(2), curve_tilt[j]);

		// Populate arrays.
		for (int s = 0; s < shape_verts_size; s++) {
			int index = j * shape_verts_size + s;
			vertices[index] = frame.xform(profile_pos[s] * curve_weight[j]);
		}
	}

	for (int j = 0; j < in_points.size() - 1; j++) {
		PackedVector3Array pts;
		for (int s = 0; s < shape_verts_size; s++) {
			int index = j * shape_verts_size + s;
			int index_2 = (j + 1) * shape_verts_size + s;
			pts.append(vertices[index]);
			pts.append(vertices[index_2]);
		}
		ret.append(pts);
	}

	return ret;
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

PackedVector3Array HoodieOps::calc_path_tangents(const PackedVector3Array &p_points, const bool p_loop) {
	PackedVector3Array ret;
	const int path_size = p_points.size();

	ret.resize(path_size);

	for (int i = 0; i < path_size - 1; i++) {
		ret[i] = (p_points[i + 1] - p_points[i]).normalized();
	}

	if (p_loop) {
		ret[path_size - 1] = ret[0];
	} else {
		ret[path_size - 1] = ret[path_size - 2];
	}

	return ret;
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

TypedArray<PackedInt32Array> HoodieOps::break_path(const PackedInt32Array &p_ids, const TypedArray<bool> &p_break) {
	if (p_ids.size() != p_break.size()) {
		UtilityFunctions::push_warning("Lists are not of the same size.");
		return TypedArray<PackedInt32Array>();
	}

	const int pts_size = p_ids.size();

	TypedArray<PackedInt32Array> ret;
	PackedInt32Array vertices;

	int prim_counter = 0;
	for (int i = 0; i < pts_size; i++) {
		if (!p_break[i]) {
			if (i == 0) {
				vertices.append(0);
				vertices.append(1);
			} else {
				if (vertices.size() != 0 && vertices[vertices.size() - 1] != i) {
					vertices.append(i);
				}

				if (i < pts_size - 1) {
					vertices.append(i + 1);
				} else {
					vertices.append(0);
				}
			}
		} else {
			if (i == 0) {
				continue;
			}

			ret.push_back(vertices);
			vertices.clear();

			vertices.push_back((i + 1) % pts_size);

			prim_counter++;
		}
	}

	ret.push_back(vertices);

	return ret;
}

TypedArray<HoodieCurve> HoodieOps::curve_offset(Ref<HoodieCurve> p_curve, const float p_delta, const Vector3 &p_normal, const bool p_polygon) {
	Geometry2D *g2d = Geometry2D::get_singleton();
	TypedArray<PackedVector2Array> offset;

	// Create plane and uv vectors.
	const Plane plane = Plane(p_normal, Vector3(0, 0, 0));
	Vector3 u = p_normal.cross(Vector3(1, 0, 0));
	if (u.length() < 1e-6) {
		u = p_normal.cross(Vector3(0, 1, 0));
	}
	u.normalize();
	Vector3 v = p_normal.cross(u);
	v.normalize();

	// Calculate points relative to the plane frame.
	PackedVector2Array in;
	for (int i = 0; i < p_curve->get_points().size(); i++) {
		Vector3 proj_pt = plane.project(p_curve->get_points()[i]);
		float x = proj_pt.dot(u);
		float y = proj_pt.dot(v);
		in.append(Vector2(x, y));
	}

	if (p_polygon) {
		offset = g2d->offset_polygon(in, p_delta);
	} else {
		offset = g2d->offset_polyline(in, p_delta, Geometry2D::JOIN_SQUARE, Geometry2D::END_ROUND);
	}

	TypedArray<HoodieCurve> ret;

	for (int i = 0; i < offset.size(); i++) {
		PackedVector2Array o = offset[i];
		PackedVector3Array v3;
		for (int j = 0; j < o.size(); j++) {
			// Transform back to global ref.
			v3.append(Vector3(0, 0, 0) + o[j].x * u + o[j].y * v);
		}
		Ref<HoodieCurve> hc;
		hc.instantiate();
		hc->set_points(v3);
		ret.append(hc);
	}

	return ret;
}

Ref<HoodieCurve> HoodieOps::equidistant_curve_sampling(Ref<HoodieCurve> p_curve, const float p_distance) {
	const float distance = Math::max(p_distance, 0.01f);

	const PackedVector3Array in_pts = p_curve->get_points();

	if (in_pts.size() < 2) {
		return Ref<HoodieCurve>();
	}

	PackedVector3Array sampled_pts;
	Geometry3D *geo = Geometry3D::get_singleton();

	// Add the starting point.
	sampled_pts.push_back(in_pts[0]);

	Vector3 prev_found_pt = in_pts[0];
	PackedVector3Array intersections;

	int i = 0;
	do {
		if ((in_pts[i] - prev_found_pt).length() > distance) {
			i--;
		}

		const int blocked_i = i;

		do {
			intersections.clear();

			if (i + 1 > in_pts.size() - 1) {
				break;
			}

			Vector3 a = (in_pts[i] - prev_found_pt).length() > distance ? prev_found_pt : in_pts[i];
			Vector3 b = in_pts[i + 1];

			float dist = (b - a).length();

			// The order of the points of the segment is important.
			intersections = geo->segment_intersects_sphere(b, a, prev_found_pt, distance);

			i++;
		} while (intersections.size() == 0);

		if (intersections.size() == 0) {
			break;
		}

		Vector3 found_point = intersections[0];
		sampled_pts.push_back(found_point);

		prev_found_pt = found_point;

	} while (i < in_pts.size() - 1 || (in_pts[i] - prev_found_pt).length() > distance);

	Ref<HoodieCurve> ret;
	ret.instantiate();
	ret->set_points(sampled_pts);
	return ret;
}

Array HoodieOps::surfaces_to_mesh(const TypedArray<HoodieMesh> &p_meshes) {
	std::vector<pmp::SurfaceMesh> surfaces;
	for (int i = 0; i < p_meshes.size(); i++) {
		Ref<HoodieMesh> hm = p_meshes[i];
		surfaces.push_back(*(hm->_get_mesh()));
	}
	return PMPToGodotConverter::surfaces_to_array_mesh(surfaces);
}

Ref<HoodieMesh> HoodieOps::copy_mesh(Ref<HoodieMesh> p_mesh, const Vector3 &p_position, const Vector3 &p_rotation, const Vector3 &p_scale) {
	Ref<HoodieMesh> ret;
	ret = p_mesh->duplicate();

	ret->transform_mesh(p_position, p_rotation, p_scale);

	return ret;
}