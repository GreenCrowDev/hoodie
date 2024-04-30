#include "hn_decompose_mesh.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNDecomposeMesh::_process(const Array &p_inputs) {
    arr_vertex.clear();
    arr_normal.clear();
    arr_tangent.clear();
    arr_color.clear();
    arr_tex_uv.clear();
    arr_tex_uv2.clear();
    arr_custom0.clear();
    arr_custom1.clear();
    arr_custom2.clear();
    arr_custom3.clear();
    arr_bones.clear();
    arr_weights.clear();
    arr_index.clear();

    if (p_inputs.size() > 0) {
        Array surface = p_inputs[0];

        if (surface.size() != ArrayMesh::ARRAY_MAX) {
            return;
        }

        arr_vertex = surface[ArrayMesh::ARRAY_VERTEX];
        arr_normal = surface[ArrayMesh::ARRAY_NORMAL];
        arr_tangent = surface[ArrayMesh::ARRAY_TANGENT];
        arr_color = surface[ArrayMesh::ARRAY_COLOR];
        arr_tex_uv = surface[ArrayMesh::ARRAY_TEX_UV];
        arr_tex_uv2 = surface[ArrayMesh::ARRAY_TEX_UV2];
        arr_custom0 = surface[ArrayMesh::ARRAY_CUSTOM0];
        arr_custom1 = surface[ArrayMesh::ARRAY_CUSTOM1];
        arr_custom2 = surface[ArrayMesh::ARRAY_CUSTOM2];
        arr_custom3 = surface[ArrayMesh::ARRAY_CUSTOM3];
        arr_bones = surface[ArrayMesh::ARRAY_BONES];
        arr_weights = surface[ArrayMesh::ARRAY_WEIGHTS];
        arr_index = surface[ArrayMesh::ARRAY_INDEX];
    }
}

String HNDecomposeMesh::get_caption() const {
    return "Decompose Mesh";
}

int HNDecomposeMesh::get_input_port_count() const {
    return 1;
}

HNDecomposeMesh::PortType HNDecomposeMesh::get_input_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNDecomposeMesh::get_input_port_name(int p_port) const {
    return "Mesh";
}

int HNDecomposeMesh::get_output_port_count() const {
    return 13;
}

HNDecomposeMesh::PortType HNDecomposeMesh::get_output_port_type(int p_port) const {
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

String HNDecomposeMesh::get_output_port_name(int p_port) const {
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

const Variant HNDecomposeMesh::get_output(int p_port) const {
    switch (p_port) {
        case 0:
            return Variant(arr_vertex);
        case 1:
            return Variant(arr_normal);
        case 2:
            return Variant(arr_tangent);
        case 3:
            return Variant(arr_color);
        case 4:
            return Variant(arr_tex_uv);
        case 5:
            return Variant(arr_tex_uv2);
        case 6:
            return Variant(arr_custom0);
        case 7:
            return Variant(arr_custom1);
        case 8:
            return Variant(arr_custom2);
        case 9:
            return Variant(arr_custom3);
        case 10:
            return Variant(arr_bones);
        case 11:
            return Variant(arr_weights);
        case 12:
            return Variant(arr_index);
    }

    return Variant();
}