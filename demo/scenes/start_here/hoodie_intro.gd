@tool
extends MeshInstance3D

func _ready() -> void:
	var hoodie_geo := HoodieGeo.new()
	
	print_hoodie_geo_info(hoodie_geo)
	
	var pts : Array[Vector3]
	pts.push_back(Vector3(0, 0, 0))
	pts.push_back(Vector3(1, 0, 0))
	pts.push_back(Vector3(2, 0, 0))
	pts.push_back(Vector3(3, 0, 0))
	hoodie_geo.points = pts;
	
	print_hoodie_geo_info(hoodie_geo)
	
	hoodie_geo.init_plane(1)
	
	print_hoodie_geo_info(hoodie_geo)
	
	var array_mesh := ArrayMesh.new()
	var arr := hoodie_geo.to_array_mesh()
	print(arr)
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arr)
	
	mesh = array_mesh

func print_hoodie_geo_info(hoodie_geo : HoodieGeo):
	print("HoodieGeo: points[%s]" % [hoodie_geo.points.size()])
