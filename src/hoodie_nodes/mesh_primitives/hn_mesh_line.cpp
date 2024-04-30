#include "hn_mesh_line.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

PackedVector3Array HNMeshLine::generate_line(const float p_length, const int p_subdivisions) {
    PackedVector3Array vertices;
    vertices.resize(p_subdivisions + 1);

    Vector3 start = Vector3(0,0,0);
    Vector3 offset = Vector3(p_length,0,0);

    for (int i = 0; i < p_subdivisions + 1; i++) {
        float t = (float)i / p_subdivisions;
        vertices[i] = start.lerp(offset, t);
    }

    return vertices;
}

void HNMeshLine::_process(const Array &p_inputs) {
    count = 1;
    start = Vector3(0,0,0);
    offset = Vector3(1,0,0);

    if (p_inputs.size() > 0) {
        if (p_inputs[0].get_type() == Variant::ARRAY) {
            Array a = p_inputs[0];
            count = MAX((int)a[0], 1);
        }
        if (p_inputs[1].get_type() == Variant::ARRAY) {
            Array a = p_inputs[1];
            start = (Vector3)a[0];
        }
        if (p_inputs[2].get_type() == Variant::ARRAY) {
            Array a = p_inputs[2];
            offset = (Vector3)a[0];
        }
    } else {
        return;
    }

    PackedVector3Array vertices;
    PackedInt32Array indices;

    vertices.resize(count + 1);
    for (int i = 0; i < count + 1; i++) {
        float t = (float)i / count;
        vertices[i] = start.lerp(offset, t);
    }

    indices.resize(0);

    mesh_line.resize(ArrayMesh::ARRAY_MAX);
    mesh_line[ArrayMesh::ARRAY_VERTEX] = vertices;
    mesh_line[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNMeshLine::get_caption() const {
    return "Mesh Line";
}

int HNMeshLine::get_input_port_count() const {
    return 3;
}

HNMeshLine::PortType HNMeshLine::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_SCALAR_UINT;
        case 1:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 2:
            return PortType::PORT_TYPE_VECTOR_3D;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNMeshLine::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Count";
        case 1:
            return "Start Location";
        case 2:
            return "Offset";
        default:
            return "";
    }
}

int HNMeshLine::get_output_port_count() const {
    return 1;
}

HNMeshLine::PortType HNMeshLine::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNMeshLine::get_output_port_name(int p_port) const {
    return "Mesh";
}

const Variant HNMeshLine::get_output(int p_port) const {
    return Variant(mesh_line);
}