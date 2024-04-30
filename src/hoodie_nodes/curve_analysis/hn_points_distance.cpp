#include "hn_points_distance.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNPointsDistance::_process(const Array &p_inputs) {
    PackedVector3Array as;
    PackedVector3Array bs;

    out.clear();

    if (p_inputs[0].get_type() == Variant::ARRAY) {
        Array a = p_inputs[0];
        if (a.size() > 0) {
            as = a;
        }
    }
    if (p_inputs[1].get_type() == Variant::ARRAY) {
        Array a = p_inputs[1];
        if (a.size() > 0) {
            bs = a;
        }
    }

    bs.resize(as.size());

    PackedFloat32Array distances;
    distances.resize(as.size());

    for (int i = 0; i < as.size(); i++) {
        Vector3 a = as[i];
        Vector3 b = bs[i];

        distances[i] = (b - a).length();
    }

    out.append_array(distances);
}

String HNPointsDistance::get_caption() const {
    return "Points Distance";
}

int HNPointsDistance::get_input_port_count() const {
    return 2;
}

HoodieNode::PortType HNPointsDistance::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 1:
            return PortType::PORT_TYPE_VECTOR_3D;
        default:
            return PortType::PORT_TYPE_VECTOR_3D;
    }
}

String HNPointsDistance::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Points";
        case 1:
            return "Points";
        default:
            return "";
    }
}

int HNPointsDistance::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNPointsDistance::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_SCALAR;
}

String HNPointsDistance::get_output_port_name(int p_port) const {
    return "Distance";
}

const Variant HNPointsDistance::get_output(int p_port) const {
    return Variant(out);
}