#ifndef HOODIE_HOODIE_MESH_H
#define HOODIE_HOODIE_MESH_H

#include "hoodie_geo.h"
#include <pmp/surface_mesh.h>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <memory>

namespace godot::greencrow::hoodie {

class HoodieMesh : public HoodieGeo {
	GDCLASS(HoodieMesh, HoodieGeo)

private:
	std::unique_ptr<pmp::SurfaceMesh> mesh;

protected:
	static void _bind_methods();

public:
	PackedVector3Array get_points() const override;
	void set_points(const PackedVector3Array &p_points) override;

	PackedInt32Array get_vertices_id() const;

	Array get_vertex_property(const String &p_name) const override;
	void add_vertex_property(const String &p_name, const Array &p_property) override;
	bool has_vertex_property(const String &p_name) const override;

	int get_faces_size() const;
	TypedArray<PackedInt32Array> get_faces() const;
	void set_faces(const TypedArray<PackedInt32Array> &p_faces);

	void init_plane(int p_resolution = 4);

	Array to_array_mesh() const;

	HoodieMesh();
};

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_MESH_H