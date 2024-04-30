#include "hn_connect_points.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNConnectPoints::_process(const Array &p_inputs) {
    Ref<HoodieGeo> in;
    TypedArray<bool> filter;

    out.clear();

    if (p_inputs.size() > 0) {
        if (p_inputs[0].get_type() == Variant::ARRAY) {
            Array a = p_inputs[0];
            if (a.size() > 0) {
                in = a[0];
            }
        }
        if (p_inputs[1].get_type() == Variant::ARRAY) {
            Array a = p_inputs[1];
            if (a.size() > 0) {
                filter = a;
            }
        }
    } else {
        return;
    }

    const int pts_size = in.ptr()->points.size();

    filter.resize(pts_size);

    Array hgeo_arr;
    hgeo_arr.resize(1);

    PackedInt32Array connections;

    for (int i = 0; i < pts_size; i++) {
        if (filter[i]) {
            if (i == 0) {
                connections.append(0);
                connections.append(1);
            } else {
                if (connections.size() != 0 && connections[connections.size() - 1] != i) {
                    connections.append(i);
                }

                if (i < pts_size - 1) {
                    connections.append(i + 1);
                } else {
                    connections.append(0);
                }
            }
        }
    }

    in.ptr()->primitives.append(HoodieGeo::Primitive(connections));

    out.append_array(hgeo_arr);
}

String HNConnectPoints::get_caption() const {
    return "Connect Points";
}

int HNConnectPoints::get_input_port_count() const {
    return 2;
}

HoodieNode::PortType HNConnectPoints::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_HGEO;
        case 1:
            return PortType::PORT_TYPE_BOOLEAN;
        default:
            return PortType::PORT_TYPE_BOOLEAN;
    }
}

String HNConnectPoints::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Geo";
        case 1:
            return "Filter";
        default:
            return "";
    }
}

int HNConnectPoints::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNConnectPoints::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_HGEO;
}

String HNConnectPoints::get_output_port_name(int p_port) const {
    return "Geo";
}

const Variant HNConnectPoints::get_output(int p_port) const {
    return Variant(out);
}