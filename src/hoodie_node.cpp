#include "hoodie_node.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void HoodieNode::mark_dirty() {
    dirty = true;
    // TODO: emit_changed?
}

void HoodieNode::set_status(const ProcessStatus &p_status) {
    status = p_status;
}

HoodieNode::ProcessStatus HoodieNode::get_status() const {
    return status;
}

void HoodieNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("mark_dirty"), &HoodieNode::mark_dirty);
    ClassDB::bind_method(D_METHOD("set_property_input", "id", "value"), &HoodieNode::set_property_input);
}

bool HoodieNode::update(bool p_inputs_updated, const Array &p_inputs) {
    bool updated = dirty || p_inputs_updated;
    dirty = false;

    if (updated) {
        outputs.clear();
        outputs.resize(get_output_port_count());

        Array packaged_inputs = _package_inputs(p_inputs);
        
        for (int i = 0; i < packaged_inputs.size(); i++) {
            _process(packaged_inputs[i]);
        }
    }

    return updated;
}

void HoodieNode::_process(const Array &p_inputs) {
}

String HoodieNode::get_caption() const {
    return "Hoodie Node";
}

int HoodieNode::get_input_port_count() const {
    return 0;
}

HoodieNode::PortType HoodieNode::get_input_port_type(int p_port) const {
    return PortType::PORT_TYPE_SCALAR;
}

HoodieNode::PortHint HoodieNode::get_input_port_hint(int p_port) const {
    return PortHint::PORT_HINT_OBJECT;
}

String HoodieNode::get_input_port_name(int p_port) const {
    return "";
}

int HoodieNode::get_output_port_count() const {
    return 0;
}

HoodieNode::PortType HoodieNode::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_SCALAR;
}

HoodieNode::PortHint HoodieNode::get_output_port_hint(int p_port) const {
    return PortHint::PORT_HINT_OBJECT;
}

String HoodieNode::get_output_port_name(int p_port) const {
    return "";
}

int HoodieNode::get_property_input_count() const {
    return 0;
}

Variant::Type HoodieNode::get_property_input_type(vec_size_t p_prop) const {
    return Variant::NIL;
}

String HoodieNode::get_property_input_hint(vec_size_t p_prop) const {
    return "";
}

Variant HoodieNode::get_property_input(vec_size_t p_port) const {
    return Variant();
}

void HoodieNode::set_property_input(vec_size_t p_prop, Variant p_input) {
}

Vector<StringName> HoodieNode::get_editable_properties() const {
    return Vector<StringName>();
}

HashMap<StringName, String> HoodieNode::get_editable_properties_names() const {
    return HashMap<StringName, String>();
}

Array HoodieNode::_package_inputs(const Array &p_inputs) const {
    Vector<Array> first_pass;

    // 1. Put each input atom in the respective input column.

    // For each input port package the data according to the hint.
    for (int i = 0; i < p_inputs.size(); i++) {
        Array fp_i;

        Variant v = p_inputs[i];

        switch (get_input_port_hint(i)) {
            case PORT_HINT_ITEM:
                {
                    if (v.get_type() == Variant::Type::ARRAY) {
                        fp_i.append_array(p_inputs[i].duplicate());
                    }
                }
                break;
            case PORT_HINT_LIST:
                {
                    if (v.get_type() == Variant::Type::ARRAY) {
                        fp_i.append(p_inputs[i].duplicate());
                    }
                }
                break;
            case PORT_HINT_HGEO_POINT:
                {
                    Ref<HoodieGeo> hgeo = v;

                    if(!hgeo.is_null()) {
                        auto points_packs = hgeo->pack_primitive_points();
                        auto attributes_packs = hgeo->pack_primitive_attributes();

                        TypedArray<Ref<HoodieGeo>> new_hgeos;

                        // Pack in singular HoodieGeos.
                        for (int p = 0; p < points_packs.size(); p++) {
                            PackedVector3Array prim_pts = points_packs[p];
                            auto prim_attr = attributes_packs[p];

                            for (int v = 0; v < prim_pts.size(); v++) {
                                Ref<HoodieGeo> new_hgeo;
                                new_hgeo.instantiate();
                                new_hgeo->points.push_back(prim_pts[v]);

                                for (auto a : prim_attr) {
                                    Variant val = a.value[v];
                                    new_hgeo->attributes[a.key].push_back(val);
                                }

                                new_hgeos.push_back(new_hgeo);
                            }
                        }

                        fp_i.append_array(new_hgeos);
                    } else {
                        hgeo.instantiate();
                        fp_i.append(hgeo);
                    }
                }
                break;
            case PORT_HINT_HGEO_PRIMITIVE:
                {
                    Ref<HoodieGeo> hgeo = v;

                    if (!hgeo.is_null()) {
                        auto points_packs = hgeo->pack_primitive_points();
                        auto attributes_packs = hgeo->pack_primitive_attributes();

                        TypedArray<Ref<HoodieGeo>> new_hgeos;

                        // Pack in singular HoodieGeos.
                        for (int p = 0; p < points_packs.size(); p++) {
                            Ref<HoodieGeo> new_hgeo;
                            new_hgeo.instantiate();
                            new_hgeo->points = points_packs[p];
                            new_hgeo->attributes = attributes_packs[p];

                            new_hgeos.push_back(new_hgeo);
                        }

                        fp_i.append_array(new_hgeos);
                    } else {
                        hgeo.instantiate();
                        fp_i.append(hgeo);
                    }
                }
                break;
            case PORT_HINT_HGEO_ALL:
                {
                    Ref<HoodieGeo> hgeo = v;

                    if (hgeo.is_null()) {
                        hgeo.instantiate();
                    }

                    fp_i.append(hgeo);
                }
                break;
            case PORT_HINT_OBJECT:
                {
                    if (v.get_type() == Variant::Type::OBJECT) {
                        fp_i.append(v);
                    }
                }
                break;
        }

        first_pass.push_back(fp_i);
    }

    // 2. Fill the arrays where needed, in order to always provide minimal data to process.

    int counter = first_pass[0].size();
    for (int i = 1; i < first_pass.size(); i++) {
        if (first_pass[i].size() > counter) {
            counter = first_pass[i].size();
        }
    }
    const int count = counter;

    for (int i = 0; i < first_pass.size(); i++) {
        Array arr = first_pass[i];
        Variant last_item = arr[arr.size() - 1];

        while (arr.size() < count) {
            arr.append(last_item);
        }
    }

    // 3. Pack the columns into rows to feed into the process.

    Array ret;

    for (int i = 0; i < count; i++) {
        Array package;
        package.resize(get_input_port_count());

        for (int j = 0; j < package.size(); j++) {
            package[j] = first_pass[j][i];
        }

        ret.push_back(package);
    }

    return ret;
}


const Variant HoodieNode::get_output(int p_port) const {
    if (outputs.size() == 0 || outputs.size() < p_port) {
        return Variant();
    } else {
        return outputs[p_port];
    }
}

void HoodieNode::set_output(int p_port, const Variant &p_data) {
    /*
    if (p_data.get_type() == Variant::OBJECT || Variant::can_convert(p_data.get_type(), Variant::ARRAY)) {
        outputs[p_port] = p_data;
    } else {
        Array out_arr;
        out_arr.push_back(p_data);
        outputs[p_port] = out_arr;
    }
    */

    if (p_data.get_type() == Variant::OBJECT || Variant::can_convert(p_data.get_type(), Variant::ARRAY)) {
        outputs[p_port].append(p_data);
    } else {
        outputs[p_port].append_array(out_arr);
    }

    /*
    switch (get_output_port_hint(p_port)) {
        case PORT_HINT_ITEM:
            {

            }
            break;
        case PORT_HINT_LIST:
            {

            }
            break;
        case PORT_HINT_HGEO_POINT:
            {

            }
            break;
        case PORT_HINT_HGEO_PRIMITIVE:
            {

            }
            break;
        case PORT_HINT_HGEO_ALL:
            {

            }
            break;
        case PORT_HINT_OBJECT:
            {

            }
            break;
    }
    */
}

bool HoodieNode::is_output_port_connected(vec_size_t p_port) const {
    if (connected_output_ports.has(p_port)) {
        return connected_output_ports[p_port];
    }
    return false;
}

void HoodieNode::set_output_port_connected(vec_size_t p_port, bool p_connected) {
    connected_output_ports[p_port] = p_connected;
}

bool HoodieNode::is_input_port_connected(vec_size_t p_port) const {
    if (connected_input_ports.has(p_port)) {
        return connected_input_ports[p_port];
    }
    return false;
}

void HoodieNode::set_input_port_connected(vec_size_t p_port, bool p_connected) {
    connected_input_ports[p_port] = p_connected;
}

HoodieNode::HoodieNode() {
}
