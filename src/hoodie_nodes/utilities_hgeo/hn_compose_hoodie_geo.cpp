#include "hn_compose_hoodie_geo.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNComposeHoodieGeo::_process(const Array &p_inputs) {
    out.clear();
    out.resize(2);

    if (p_inputs.size() == 0) {
        return;
    }

    Array in_arr = p_inputs;

    PackedVector3Array in_points = in_arr[0];

    out[0] = in_points;
}

String HNComposeHoodieGeo::get_caption() const {
    return "Compose Hoodie Geo";
}

int HNComposeHoodieGeo::get_input_port_count() const {
    return 2;
}

HoodieNode::PortType HNComposeHoodieGeo::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 1:
            return PortType::PORT_TYPE_DATA;
        default:
            return PortType::PORT_TYPE_DATA;
    }
}

String HNComposeHoodieGeo::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Points";
        case 1:
            return "Primitives";
        default:
            return "";
    }
}

int HNComposeHoodieGeo::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNComposeHoodieGeo::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_HGEO;
}

String HNComposeHoodieGeo::get_output_port_name(int p_port) const {
    return "Geo";
}

const Variant HNComposeHoodieGeo::get_output(int p_port) const {
    return Variant(out);
}