#include "hn_mesh_circle.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNMeshCircle::_process(const Array &p_inputs) {
    verts_count = 3;
    radius = 1.0;

    if (p_inputs.size() > 0) {
        if (p_inputs[0].get_type() == Variant::ARRAY) {
            Array a = p_inputs[0];
            verts_count = MAX((int)a[0], 3);
        }
        if (p_inputs[1].get_type() == Variant::ARRAY) {
            Array a = p_inputs[1];
            radius = (float)a[0];
        }
    } else {
        return;
    }

    PackedVector3Array vertices;
    PackedInt32Array indices;

    vertices.resize(verts_count + 1);

    Vector3 *v_write = vertices.ptrw();

    float two_pi = 2.0f * 3.14159265359f;
    float step = two_pi / verts_count;
    float angle = 0.0;

    for (int i = 0; i < verts_count + 1; i++) {
        v_write[i].x = radius * sin(angle);
        v_write[i].y = -radius * cos(angle);
        v_write[i].z = 0.0f;

        angle += step;
    }

    indices.resize((verts_count - 2) * 3);

    out.resize(ArrayMesh::ARRAY_MAX);
    out[ArrayMesh::ARRAY_VERTEX] = vertices;
    out[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNMeshCircle::get_caption() const {
    return "Mesh Circle";
}

int HNMeshCircle::get_input_port_count() const {
    return 2;
}

HNMeshCircle::PortType HNMeshCircle::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_SCALAR_UINT;
        case 1:
            return PortType::PORT_TYPE_SCALAR;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNMeshCircle::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Vertices";
        case 1:
            return "Radius";
        default:
            return "";
    }
}

int HNMeshCircle::get_output_port_count() const {
    return 1;
}

HNMeshCircle::PortType HNMeshCircle::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNMeshCircle::get_output_port_name(int p_port) const {
    return "Mesh";
}

const Variant HNMeshCircle::get_output(int p_port) const {
    return Variant(out);
}