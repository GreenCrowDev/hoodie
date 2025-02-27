#ifndef HOODIE_HOODIE_GEO_H
#define HOODIE_HOODIE_GEO_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot::greencrow::hoodie {

class HoodieGeo : public RefCounted {
	GDCLASS(HoodieGeo, RefCounted)

private:
	PackedVector3Array points;
	HashMap<String, Array> points_attributes;

	TypedArray<PackedInt32Array> elements;

protected:
	static void _bind_methods();

public:
	PackedVector3Array get_points() const;
	void set_points(const PackedVector3Array &p_points);

	TypedArray<PackedInt32Array> get_elements() const;
	void set_elements(const TypedArray<PackedInt32Array> &p_elements);

	HoodieGeo();
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_GEO_H