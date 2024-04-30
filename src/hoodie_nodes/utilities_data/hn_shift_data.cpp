#include "hn_shift_data.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNShiftData::_process(const Array &p_inputs) {
    Array in;
    int offset = 0;

    out.clear();

    if (p_inputs.size() > 0) {
        if (p_inputs[0].get_type() == Variant::ARRAY) {
            Array a = p_inputs[0];
            if (a.size() > 0) {
                in = a;
            }
        }
        if (p_inputs[1].get_type() == Variant::ARRAY) {
            Array a = p_inputs[1];
            if (a.size() > 0) {
                offset = a[0];
            }
        }
    } else {
        return;
    }

    Array shifted;
    shifted.resize(in.size());

    for (int i = 0; i < in.size(); i++) {
        int n = in.size();
        int a = i + offset;

        a = ((a % n) + n) % n;

        shifted[i] = in[a];
    }

    out.append_array(shifted);
}

String HNShiftData::get_caption() const {
    return "Shift Data";
}

int HNShiftData::get_input_port_count() const {
    return 2;
}

HoodieNode::PortType HNShiftData::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_DATA;
        case 1:
            return PortType::PORT_TYPE_SCALAR_INT;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNShiftData::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Data";
        case 1:
            return "Offset";
        default:
            return "";
    }
}

int HNShiftData::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNShiftData::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_DATA;
}

String HNShiftData::get_output_port_name(int p_port) const {
    return "Data";
}

const Variant HNShiftData::get_output(int p_port) const {
    return Variant(out);
}