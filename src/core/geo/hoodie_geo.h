#ifndef HOODIE_HOODIE_GEO_H
#define HOODIE_HOODIE_GEO_H

#include <godot_cpp/classes/resource.hpp>

namespace godot::greencrow::hoodie {

class HoodieGeo : public RefCounted {
	GDCLASS(HoodieGeo, RefCounted)

protected:
	static void _bind_methods();

public:
	HoodieGeo();
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_GEO_H