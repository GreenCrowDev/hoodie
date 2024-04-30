#include "hn_math_less_than.h"

using namespace godot;

void HNMathLessThan::_process(const Array &p_inputs) {
    out.clear();

    if (p_inputs.size() == 0) {
        return;
    }

    Array fac_0 = p_inputs[0].duplicate();
    Array fac_1 = p_inputs[1].duplicate();

    while (fac_0.size() < fac_1.size()) {
        fac_0.push_back(fac_0[fac_0.size() - 1]);
    }

    while (fac_1.size() < fac_0.size()) {
        fac_1.push_back(fac_1[fac_1.size() - 1]);
    }

    TypedArray<bool> results;
    results.resize(fac_0.size());

    for (int i = 0; i < fac_0.size(); i++) {
        const float a = (float)fac_0[i];
        const float b = (float)fac_1[i];
        results[i] = a < b;
    }

    out.append_array(results);
}

String HNMathLessThan::get_caption() const {
    return "Math Less Than";
}

int HNMathLessThan::get_input_port_count() const {
    return 2;
}

HoodieNode::PortType HNMathLessThan::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_SCALAR;
        case 1:
            return PortType::PORT_TYPE_SCALAR;
    }

    return PortType::PORT_TYPE_SCALAR;
}

String HNMathLessThan::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Value";
        case 1:
            return "Value";
    }

    return "Value";
}

int HNMathLessThan::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNMathLessThan::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_BOOLEAN;
}

String HNMathLessThan::get_output_port_name(int p_port) const {
    return "Bool";
}

const Variant HNMathLessThan::get_output(int p_port) const {
    return Variant(out);
}