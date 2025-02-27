extends MeshInstance3D

func _ready() -> void:
	var hoodie_geo := HoodieGeo.new()
	
	print(hoodie_geo.points.size())
	
	var pts : Array[Vector3]
	pts.push_back(Vector3(0, 0, 0))
	pts.push_back(Vector3(1, 0, 0))
	pts.push_back(Vector3(2, 0, 0))
	pts.push_back(Vector3(3, 0, 0))
	
	hoodie_geo.points = pts;
	
	print(hoodie_geo.points.size())
