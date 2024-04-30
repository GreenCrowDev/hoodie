#include "hn_compose_mesh.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNComposeMesh::_process(const Array &p_inputs) {
    out_mesh.clear();
    out_mesh.resize(ArrayMesh::ARRAY_MAX);

    if (p_inputs.size() == 0) {
        return;
    }

    Array in_arr = p_inputs;

    Array in_arr_vert = in_arr[ArrayMesh::ARRAY_VERTEX];
    Array in_arr_norm = in_arr[ArrayMesh::ARRAY_NORMAL];
    Array in_arr_tang = in_arr[ArrayMesh::ARRAY_TANGENT];
    Array in_arr_col = in_arr[ArrayMesh::ARRAY_COLOR];
    Array in_arr_uv = in_arr[ArrayMesh::ARRAY_TEX_UV];
    Array in_arr_uv2 = in_arr[ArrayMesh::ARRAY_TEX_UV2];
    Array in_arr_custom0 = in_arr[ArrayMesh::ARRAY_CUSTOM0];
    Array in_arr_custom1 = in_arr[ArrayMesh::ARRAY_CUSTOM1];
    Array in_arr_custom2 = in_arr[ArrayMesh::ARRAY_CUSTOM2];
    Array in_arr_custom3 = in_arr[ArrayMesh::ARRAY_CUSTOM3];
    Array in_arr_bones = in_arr[ArrayMesh::ARRAY_BONES];
    Array in_arr_weights = in_arr[ArrayMesh::ARRAY_WEIGHTS];
    Array in_arr_idx = in_arr[ArrayMesh::ARRAY_INDEX];

    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedFloat32Array tangents;
    PackedColorArray colors;
    PackedVector2Array uv;
    PackedVector2Array uv2;
    PackedByteArray custom0;
    PackedByteArray custom1;
    PackedByteArray custom2;
    PackedByteArray custom3;
    PackedInt32Array bones;
    PackedFloat64Array weights;
    PackedInt32Array indices;

    const int v_size = in_arr_vert.size();

    // Vertices
    vertices = in_arr_vert;
    // Normals
    if (in_arr_norm.size() != v_size) {
        normals.resize(v_size);
    } else {
        normals = in_arr_norm;
    }
    // Tangents
    if (in_arr_tang.size() != (v_size * 4)) {
        tangents.resize(v_size * 4);
    } else {
        tangents = in_arr_tang;
    }
    // Color
    if (in_arr_col.size() != v_size) {
        colors.resize(v_size);
    } else {
        colors = in_arr_col;
    }
    // UV
    if (in_arr_uv.size() != v_size) {
        uv.resize(v_size);
    } else {
        uv = in_arr_uv;
    }
    // UV2
    if (in_arr_uv2.size() != v_size) {
        uv2.resize(v_size);
    } else {
        uv2 = in_arr_uv2;
    }
    // Custom0
    if (in_arr_custom0.size() != v_size * 4) {
        custom0.resize(v_size * 4);
    } else {
        custom0 = in_arr_custom0;
    }
    // Custom1
    if (in_arr_custom1.size() != v_size * 4) {
        custom1.resize(v_size * 4);
    } else {
        custom1 = in_arr_custom1;
    }
    // Custom2
    if (in_arr_custom2.size() != v_size * 4) {
        custom2.resize(v_size * 4);
    } else {
        custom2 = in_arr_custom2;
    }
    // Custom3
    if (in_arr_custom3.size() != v_size * 4) {
        custom3.resize(v_size * 4);
    } else {
        custom3 = in_arr_custom3;
    }
    // Bones
    if (in_arr_bones.size() != v_size * 4) {
        bones.resize(v_size * 4);
    } else {
        bones = in_arr_bones;
    }
    // Weights
    if (in_arr_weights.size() != v_size * 4) {
        weights.resize(v_size * 4);
    } else {
        weights = in_arr_weights;
    }
    // Indices
    indices = in_arr_idx;

    out_mesh[ArrayMesh::ARRAY_VERTEX] = vertices;
    out_mesh[ArrayMesh::ARRAY_NORMAL] = normals;
    out_mesh[ArrayMesh::ARRAY_TANGENT] = tangents;
    out_mesh[ArrayMesh::ARRAY_COLOR] = colors;
    out_mesh[ArrayMesh::ARRAY_TEX_UV] = uv;
    out_mesh[ArrayMesh::ARRAY_TEX_UV2] = uv2;
    out_mesh[ArrayMesh::ARRAY_CUSTOM0] = custom0;
    out_mesh[ArrayMesh::ARRAY_CUSTOM1] = custom1;
    out_mesh[ArrayMesh::ARRAY_CUSTOM2] = custom2;
    out_mesh[ArrayMesh::ARRAY_CUSTOM3] = custom3;
    out_mesh[ArrayMesh::ARRAY_BONES] = bones;
    out_mesh[ArrayMesh::ARRAY_WEIGHTS] = weights;
    out_mesh[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNComposeMesh::get_caption() const {
    return "Compose Mesh";
}

int HNComposeMesh::get_input_port_count() const {
    return 13;
}

HNComposeMesh::PortType HNComposeMesh::get_input_port_type(int p_port) const {
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

String HNComposeMesh::get_input_port_name(int p_port) const {
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

int HNComposeMesh::get_output_port_count() const {
    return 1;
}

HNComposeMesh::PortType HNComposeMesh::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNComposeMesh::get_output_port_name(int p_port) const {
    return "Mesh";
}

const Variant HNComposeMesh::get_output(int p_port) const {
    return Variant(out_mesh);
}