#ifndef PMP_TO_GODOT_CONVERTER_H
#define PMP_TO_GODOT_CONVERTER_H

#include <pmp/surface_mesh.h>
#include <pmp/types.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot::greencrow::hoodie {

class PMPToGodotConverter {
public:
	static Vector2 tex_coord_to_v2(const pmp::TexCoord &p_tex_coord);
	static pmp::TexCoord v2_to_tex_coord(const Vector2 &p_vector);

	static Vector3 point_to_v3(const pmp::Point &p_point);
	static pmp::Point v3_to_point(const Vector3 &p_vector);

	static Array surface_to_array_mesh(const pmp::SurfaceMesh &p_surface_mesh);
};

} // namespace godot::greencrow::hoodie

#endif // PMP_TO_GODOT_CONVERTER_H