#include "hn_output.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNOutput::_process(const Array &p_inputs) {
    // TODO: optimize output _process function

    output_arr.clear();
    output_arr.resize(ArrayMesh::ARRAY_MAX);

    if (p_inputs.size() > 0) {
        if (p_inputs[0].get_type() == Variant::ARRAY) {
            Array surface = p_inputs[0];

            if (surface.size() != ArrayMesh::ARRAY_MAX) {
                return;
            }
            
            output_arr[ArrayMesh::ARRAY_VERTEX] = surface[ArrayMesh::ARRAY_VERTEX];
            output_arr[ArrayMesh::ARRAY_NORMAL] = surface[ArrayMesh::ARRAY_NORMAL];
            output_arr[ArrayMesh::ARRAY_TANGENT] = surface[ArrayMesh::ARRAY_TANGENT];
            output_arr[ArrayMesh::ARRAY_COLOR] = surface[ArrayMesh::ARRAY_COLOR];
            output_arr[ArrayMesh::ARRAY_TEX_UV] = surface[ArrayMesh::ARRAY_TEX_UV];
            output_arr[ArrayMesh::ARRAY_TEX_UV2] = surface[ArrayMesh::ARRAY_TEX_UV2];
            output_arr[ArrayMesh::ARRAY_CUSTOM0] = surface[ArrayMesh::ARRAY_CUSTOM0];
            output_arr[ArrayMesh::ARRAY_CUSTOM1] = surface[ArrayMesh::ARRAY_CUSTOM1];
            output_arr[ArrayMesh::ARRAY_CUSTOM2] = surface[ArrayMesh::ARRAY_CUSTOM2];
            output_arr[ArrayMesh::ARRAY_CUSTOM3] = surface[ArrayMesh::ARRAY_CUSTOM3];
            output_arr[ArrayMesh::ARRAY_BONES] = surface[ArrayMesh::ARRAY_BONES];
            output_arr[ArrayMesh::ARRAY_WEIGHTS] = surface[ArrayMesh::ARRAY_WEIGHTS];
            output_arr[ArrayMesh::ARRAY_INDEX] = surface[ArrayMesh::ARRAY_INDEX];
        }
    }
}

String HNOutput::get_caption() const {
    return "Output";
}

int HNOutput::get_input_port_count() const {
    return 1;
}

HNOutput::PortType HNOutput::get_input_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNOutput::get_input_port_name(int p_port) const {
    return "Geometry";
}

int HNOutput::get_output_port_count() const {
    return 0;
}

HNOutput::PortType HNOutput::get_output_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 1:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 2:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 3:
            return PortType::PORT_TYPE_VECTOR_4D;
        case 4:
            return PortType::PORT_TYPE_VECTOR_2D;
        case 5:
            return PortType::PORT_TYPE_VECTOR_2D;
        case 6:
            return PortType::PORT_TYPE_SCALAR_INT;
        case 7:
            return PortType::PORT_TYPE_SCALAR_INT;
        case 8:
            return PortType::PORT_TYPE_SCALAR_INT;
        case 9:
            return PortType::PORT_TYPE_SCALAR_INT;
        case 10:
            return PortType::PORT_TYPE_SCALAR;
        case 11:
            return PortType::PORT_TYPE_SCALAR;
        case 12:
            return PortType::PORT_TYPE_SCALAR_INT;
    }

    return PortType::PORT_TYPE_SCALAR;
}

String HNOutput::get_output_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Vertex";
        case 1:
            return "Normal";
        case 2:
            return "Tangent";
        case 3:
            return "Color";
        case 4:
            return "Tex UV";
        case 5:
            return "Tex UV 2";
        case 6:
            return "Custom 0";
        case 7:
            return "Custom 1";
        case 8:
            return "Custom 2";
        case 9:
            return "Custom 3";
        case 10:
            return "Bones";
        case 11:
            return "Weights";
        case 12:
            return "Index";
    }

    return "";
}

const Variant HNOutput::get_output(int p_port) const {
    if (p_port == 0) {
        return Variant(output_arr);
    }

    return Variant();
}