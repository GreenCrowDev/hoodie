#ifndef HOODIE_HOODIE_OPS_H
#define HOODIE_HOODIE_OPS_H

#include <core/geo/hoodie_geo.h>

namespace godot::greencrow::hoodie {

class HoodieOps : public Object {
	GDCLASS(HoodieOps, Object)

protected:
	static void _bind_methods();

public:
	// Float.
	static PackedFloat32Array noise_reduction(const PackedFloat32Array &p_values, const int p_severity = 4, const bool p_loop = false);

	// Curve.
	static Ref<HoodieGeo> curve_sweep(Ref<HoodieGeo> p_curve, Ref<HoodieGeo> p_profile, const bool p_closed = false, const bool p_u_dist = false, const bool p_v_dist = false);
	static PackedFloat32Array progressive_path_distances(const PackedVector3Array &p_points);
	static PackedVector3Array calc_path_tangents(const PackedVector3Array &p_points, const bool p_loop = false);
	static PackedFloat32Array points_curvature(const PackedVector3Array &p_points, const PackedVector3Array &p_up_vectors, const bool p_loop = false);
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_OPS_H