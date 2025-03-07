@tool
extends MeshInstance3D

@onready var path_3d = $"../Path3D"

const vertex_color_mat = preload("res://materials/vertex_color.tres")

func _ready() -> void:
	var hg_path := HoodieGeo.new()
	var hg_profile := HoodieGeo.new()
	
	hg_path.points = path_3d.curve.get_baked_points()
	var curvatures = HoodieOps.points_curvature(path_3d.curve.get_baked_points(), path_3d.curve.get_baked_up_vectors())
	var colors := PackedColorArray()
	colors.resize(hg_path.points.size())
	for i in colors.size():
		var r : float = curvatures[i] if curvatures[i] > 0 else 0.0
		var b : float = abs(curvatures[i]) if curvatures[i] < 0 else 0.0
		colors[i] = Color(r, 0, b)
	hg_path.add_vertex_property("C", colors)
	hg_profile.points = [Vector3(0, 0, 0), Vector3(1, 0, 0)]
	var hg_sweep := HoodieOps.curve_sweep(hg_path, hg_profile)
	var arr := hg_sweep.to_array_mesh()
	var array_mesh := ArrayMesh.new()
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arr)
	array_mesh.surface_set_material(0, vertex_color_mat)
	mesh = array_mesh

func print_hoodie_geo_info(hoodie_geo : HoodieGeo):
	print("HoodieGeo: points[%s]" % [hoodie_geo.points.size()])
