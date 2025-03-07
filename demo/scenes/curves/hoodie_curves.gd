@tool
extends MeshInstance3D

@onready var path_3d = $"../Path3D"

func _ready() -> void:
	var hg_path := HoodieGeo.new()
	var hg_profile := HoodieGeo.new()
	
	hg_path.points = path_3d.curve.get_baked_points()
	# print(hg_path.points)
	hg_profile.points = [Vector3(0, 0, 0), Vector3(1, 0, 0)]
	
	var sweep := HoodieOps.curve_sweep(hg_path, hg_profile)
	print_hoodie_geo_info(sweep)
	
	var arr := sweep.to_array_mesh()
	# print(arr)
	var array_mesh := ArrayMesh.new()
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arr)
	
	mesh = array_mesh

func print_hoodie_geo_info(hoodie_geo : HoodieGeo):
	print("HoodieGeo: points[%s]" % [hoodie_geo.points.size()])
