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
	
	var el0 : PackedInt32Array
	el0.append_array([0, 1, 2, 3])
	var els : Array[PackedInt32Array]
	els.push_back(el0)
	hoodie_geo.elements = els
	
	print_hoodie_geo_info(hoodie_geo)

func print_hoodie_geo_info(hoodie_geo : HoodieGeo):
	print("HoodieGeo: points[%s], elements[%s]" % [hoodie_geo.points.size(), hoodie_geo.elements.size()])
