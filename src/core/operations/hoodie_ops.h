#ifndef HOODIE_HOODIE_OPS_H
#define HOODIE_HOODIE_OPS_H

#include <core/geo/hoodie_geo.h>

namespace godot::greencrow::hoodie {

class HoodieOps : public Object {
	GDCLASS(HoodieOps, Object)

protected:
	static void _bind_methods();

public:
	static Ref<HoodieGeo> curve_sweep(Ref<HoodieGeo> p_curve, Ref<HoodieGeo> p_profile, const bool p_closed = false, const bool p_u_dist = false, const bool p_v_dist = false);
	static PackedFloat32Array progressive_path_distances(const PackedVector3Array &p_points);
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_OPS_H