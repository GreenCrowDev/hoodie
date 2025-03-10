#ifndef HOODIE_HOODIE_GEO_H
#define HOODIE_HOODIE_GEO_H

#include <pmp/surface_mesh.h>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <memory>

namespace godot::greencrow::hoodie {

class HoodieGeo : public RefCounted {
	GDCLASS(HoodieGeo, RefCounted)

protected:
	static void _bind_methods();

public:
	virtual PackedVector3Array get_points() const;
	virtual void set_points(const PackedVector3Array &p_points);

	virtual Array get_vertex_property(const String &p_name) const;
	virtual void add_vertex_property(const String &p_name, const Array &p_property);
	virtual bool has_vertex_property(const String &p_name) const;

	virtual Ref<HoodieGeo> duplicate() const;

	HoodieGeo();
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_GEO_H