#ifndef HOODIE_HOODIE_GEO_H
#define HOODIE_HOODIE_GEO_H

#include <pmp/surface_mesh.h>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <memory>

namespace godot::greencrow::hoodie {

class HoodieGeo : public RefCounted {
	GDCLASS(HoodieGeo, RefCounted)

private:
	std::unique_ptr<pmp::SurfaceMesh> mesh;

protected:
	static void _bind_methods();

public:
	int get_points_size() const;
	PackedInt32Array get_vertices_id() const;
	PackedVector3Array get_points() const;
	void set_points(const PackedVector3Array &p_points);

	Variant get_vertex_property(const String &p_name) const;
	void add_vertex_property(const String &p_name, const Variant p_property);
	bool has_vertex_property(const String &p_name) const;

	int get_faces_size() const;
	TypedArray<PackedInt32Array> get_faces() const;
	void set_faces(TypedArray<PackedInt32Array> &p_faces);

	void init_plane(int p_resolution = 4);

	Array to_array_mesh() const;

	HoodieGeo();
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_GEO_H