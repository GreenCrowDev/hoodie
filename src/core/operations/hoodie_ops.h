#ifndef HOODIE_HOODIE_OPS_H
#define HOODIE_HOODIE_OPS_H

#include <core/geo/hoodie_curve.h>
#include <core/geo/hoodie_geo.h>
#include <core/geo/hoodie_mesh.h>
#include <godot_cpp/variant/plane.hpp>

namespace godot::greencrow::hoodie {

class HoodieOps : public Object {
	GDCLASS(HoodieOps, Object)

protected:
	static void _bind_methods();

public:
	// Float.
	static PackedFloat32Array noise_reduction(const PackedFloat32Array &p_values, const int p_severity = 4, const bool p_loop = false);

	// Vector.
	static Quaternion from_euler(const Vector3 &p_euler);

	// Curve.
	static Ref<HoodieMesh> curve_sweep(Ref<HoodieCurve> p_curve, Ref<HoodieCurve> p_profile, const bool p_loop = false, const bool p_u_dist = false, const bool p_v_dist = false);
	static TypedArray<PackedVector3Array> curve_sweep_convex_collision_points(Ref<HoodieCurve> p_curve, Ref<HoodieCurve> p_profile, const bool p_loop = false);
	static PackedFloat32Array progressive_path_distances(const PackedVector3Array &p_points);
	static PackedVector3Array calc_path_tangents(const PackedVector3Array &p_points, const bool p_loop = false);
	static PackedFloat32Array points_curvature(const PackedVector3Array &p_points, const PackedVector3Array &p_up_vectors, const bool p_loop = false);
	static TypedArray<PackedInt32Array> break_path(const PackedInt32Array &p_ids, const TypedArray<bool> &p_break);
	static TypedArray<HoodieCurve> curve_offset(Ref<HoodieCurve> p_curve, const float p_delta, const Vector3 &p_normal = Vector3(0, 1, 0), const bool p_polygon = false);
	static Ref<HoodieCurve> equidistant_curve_sampling(Ref<HoodieCurve> p_curve, const float p_distance);

	// Mesh.
	static Array surfaces_to_mesh(const TypedArray<HoodieMesh> &p_meshes);

	// HoodieMesh.
	static Ref<HoodieMesh> copy_mesh(Ref<HoodieMesh> p_mesh, const Vector3 &p_position, const Vector3 &p_rotation, const Vector3 &p_scale);
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_OPS_H