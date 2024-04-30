#include "hn_mesh_grid.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

Array HNMeshGrid::generate_plane(const float x_length, const float y_length, const int x_subs, const int y_subs, const Transform3D frame, const int id_offset) {
    PackedVector3Array vertices;
    PackedInt32Array indices;
    PackedVector3Array normals;
    PackedVector2Array uvs;

    vertices.resize(x_subs * y_subs);
    normals.resize(x_subs * y_subs);
    uvs.resize(x_subs * y_subs);
    const int total_indices = 2 * 3 * (y_subs - 1) * (x_subs - 1);
    indices.resize(total_indices);

    float x_2 = x_length * 0.5;
    float y_2 = y_length * 0.5;

    int triangle_index = 0;
    for (int y = 0; y < y_subs; y++) {
        float y_time = (float)y / (y_subs - 1);
        float y_coord = Math::lerp(-y_2, y_2, y_time);

        for (int x = 0; x < x_subs; x++) {
            float x_time = (float)x / (x_subs - 1);
            float x_coord = Math::lerp(-x_2, x_2, x_time);

            int index = y * x_subs + x;

            vertices[index] = frame.xform(Vector3(x_coord, y_coord, 0.0));
            normals[index] = frame.xform(Vector3(0,0,-1));
            uvs[index] = Vector2(x_time, y_time);

            if (y > y_subs - 2) continue;
            if (x > x_subs - 2) continue;

            int i = y * x_subs + x;
            indices[triangle_index++] = id_offset + i;
            indices[triangle_index++] = id_offset + i + 1;
            indices[triangle_index++] = id_offset + i + x_subs;
            indices[triangle_index++] = id_offset + i + 1;
            indices[triangle_index++] = id_offset + i + x_subs + 1;
            indices[triangle_index++] = id_offset + i + x_subs;
        }
    }

    Array out;
    out.push_back(vertices);
    out.push_back(indices);
    out.push_back(normals);
    out.push_back(uvs);
    return out;
}

void HNMeshGrid::_process(const Array &p_inputs) {
    float x = 1;
    float z = 1;

    if (p_inputs.size() > 0) {
        if (p_inputs[0].get_type() == Variant::ARRAY) {
            Array a = p_inputs[0];
            x = (float)a[0];
        }
        if (p_inputs[1].get_type() == Variant::ARRAY) {
            Array a = p_inputs[1];
            z = (float)a[0];
        }
    }

    PackedVector3Array vertices;
    PackedInt32Array indices;

    vertices.resize(4);
    vertices[0] = Vector3(0,0,0);
    vertices[1] = Vector3(x,0,0);
    vertices[2] = Vector3(x,0,z);
    vertices[3] = Vector3(0,0,z);

    indices.resize(6);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0;

    surface_arr.resize(ArrayMesh::ARRAY_MAX);
    surface_arr[ArrayMesh::ARRAY_VERTEX] = vertices;
    surface_arr[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNMeshGrid::get_caption() const {
    return "Mesh Grid";
}

int HNMeshGrid::get_input_port_count() const {
    return 4;
}

HNMeshGrid::PortType HNMeshGrid::get_input_port_type(int p_port) const {
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

String HNMeshGrid::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Size X";
        case 1:
            return "Size Z";
        case 2:
            return "Vertices X";
        case 3:
            return "Vertices Z";
        default:
            return "";
    }
}

int HNMeshGrid::get_output_port_count() const {
    return 2;
}

HNMeshGrid::PortType HNMeshGrid::get_output_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_MESH;
        case 1:
            return PortType::PORT_TYPE_VECTOR_2D;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNMeshGrid::get_output_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Mesh";
        case 1:
            return "UV Map";
        default:
            return "";
    }
}

const Variant HNMeshGrid::get_output(int p_port) const {
    switch (p_port) {
        case 0:
            return Variant(surface_arr);
        case 1:
            return Variant();
        default:
            return Variant();
    }
}