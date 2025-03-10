#ifndef HOODIE_HOODIE_CURVE_H
#define HOODIE_HOODIE_CURVE_H

#include "hoodie_geo.h"
#include <godot_cpp/classes/curve3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot::greencrow::hoodie {

class HoodieCurve : public HoodieGeo {
	GDCLASS(HoodieCurve, HoodieGeo)

private:
	PackedVector3Array points;
	HashMap<String, Array> properties;

	float baked_length;

protected:
	static void _bind_methods();

public:
	PackedVector3Array get_points() const override;
	void set_points(const PackedVector3Array &p_points) override;

	Array get_vertex_property(const String &p_name) const override;
	void add_vertex_property(const String &p_name, const Array &p_property) override;
	bool has_vertex_property(const String &p_name) const override;

	void init_from_curve_3d(const Ref<Curve3D> &p_curve, const bool p_loop = false);

	HoodieCurve();
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_CURVE_H