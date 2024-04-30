#ifndef HOODIE_HOODIEGEO_H
#define HOODIE_HOODIEGEO_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot
{

class HoodieGeo : public RefCounted {
    GDCLASS(HoodieGeo, RefCounted)

public:
    struct Primitive {
        PackedInt32Array vertices;

        Primitive() : vertices() {
        }

        Primitive(PackedInt32Array p_vertices) : vertices(p_vertices) {
        }
    };

public:
    // Point data.
    PackedVector3Array points;
    // Primitive data.
    Vector<Primitive> primitives;

protected:
    static void _bind_methods();
};

} // namespace godot

#endif // HOODIE_HOODIEGEO_H