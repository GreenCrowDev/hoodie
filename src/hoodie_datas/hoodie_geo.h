#ifndef HOODIE_HOODIEGEO_H
#define HOODIE_HOODIEGEO_H

#include "hoodie_data.h"

#include <godot_cpp/templates/vector.hpp>

namespace godot
{

class HoodieGeo : public HoodieData {
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
    // Point attributes.
    HashMap<String, Array> attributes;
    // Primitive data.
    Vector<Primitive> primitives;

protected:
    static void _bind_methods();

public:
    void clear() override;

    HashMap<String, String> populate_tab_inspector() const override;

    static Ref<HoodieData> create_reference(const PackedVector3Array &p_points);
    static Ref<HoodieData> create_reference(const PackedVector3Array &p_points, const Vector<Primitive> &p_primitives);
    static Ref<HoodieData> create_reference(const PackedVector3Array &p_points, const PackedInt32Array &p_vertices);

    Vector<PackedVector3Array> pack_primitive_points() const;
    void unpack_primitive_points(const TypedArray<PackedVector3Array> &p_array);

    Vector<HashMap<String, Array>> pack_primitive_attributes() const;

    Array to_mesh() const;

    HoodieGeo();
    HoodieGeo(PackedVector3Array p_points, Vector<Primitive> p_primitives);
    HoodieGeo(PackedVector3Array p_points, PackedInt32Array p_vertices);
};

} // namespace godot

#endif // HOODIE_HOODIEGEO_H