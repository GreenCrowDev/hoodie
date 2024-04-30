#include "hn_mesh_rect.h"
#include "hn_mesh_line.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNMeshRect::_process(const Array &p_inputs) {
    width = 1.0;
    height = 1.0;
    w_subs = 1;
    h_subs = 1;

    out.clear();
    out.resize(ArrayMesh::ARRAY_MAX);

    if (p_inputs.size() == 0) {
        return;
    }

    if (p_inputs[0].get_type() == Variant::ARRAY) {
        Array a = p_inputs[0].duplicate();
        if (a.size() > 0) {
            width = (float)a[0];
        }
    }
    if (p_inputs[1].get_type() == Variant::ARRAY) {
        Array a = p_inputs[1].duplicate();
        if (a.size() > 0) {
            height = (float)a[0];
        }
    }
    if (p_inputs[0].get_type() == Variant::ARRAY) {
        Array a = p_inputs[1].duplicate();
        if (a.size() > 0) {
            w_subs = MAX((int)a[0], 1);
        }
    }
    if (p_inputs[0].get_type() == Variant::ARRAY) {
        Array a = p_inputs[2].duplicate();
        if (a.size() > 0) {
            h_subs = MAX((int)a[0], 1);
        }
    }

    PackedVector3Array vertices;
    PackedInt32Array indices;

    PackedVector3Array top = HNMeshLine::generate_line(width, w_subs);
    PackedVector3Array bottom = HNMeshLine::generate_line(width, w_subs);
    for (int i = 0; i < top.size(); i++) {
        top[i] += Vector3(width * -0.5, height * 0.5, 0);
        bottom[i] += Vector3(width * -0.5, height * -0.5, 0);
    }
    top.reverse();

    PackedVector3Array right = HNMeshLine::generate_line(height, h_subs);
    PackedVector3Array left = HNMeshLine::generate_line(height, h_subs);
    for (int i = 0; i < right.size(); i++) {
        right[i] = Vector3(0, right[i].x, 0);
        left[i] = Vector3(0, left[i].x, 0);
        right[i] += Vector3(width * 0.5, height * -0.5, 0);
        left[i] += Vector3(width * -0.5, height * -0.5, 0);
    }
    left.reverse();

    vertices.append_array(top);
    vertices.append_array(left);
    vertices.append_array(bottom);
    vertices.append_array(right);

    indices.resize(0);

    out.resize(ArrayMesh::ARRAY_MAX);
    out[ArrayMesh::ARRAY_VERTEX] = vertices;
    out[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNMeshRect::get_caption() const {
    return "Mesh Rect";
}

int HNMeshRect::get_input_port_count() const {
    return 4;
}

HoodieNode::PortType HNMeshRect::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_SCALAR;
        case 1:
            return PortType::PORT_TYPE_SCALAR;
        case 2:
            return PortType::PORT_TYPE_SCALAR_UINT;
        case 3:
            return PortType::PORT_TYPE_SCALAR_UINT;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNMeshRect::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Width";
        case 1:
            return "Height";
        case 2:
            return "Width Subdivisions";
        case 3:
            return "Height Subdivisions";
        default:
            return "";
    }
}

int HNMeshRect::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNMeshRect::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNMeshRect::get_output_port_name(int p_port) const {
    return "Mesh";
}

const Variant HNMeshRect::get_output(int p_port) const {
    return Variant(out);
}