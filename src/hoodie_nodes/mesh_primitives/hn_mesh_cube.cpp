#include "hn_mesh_cube.h"

#include <godot_cpp/classes/array_mesh.hpp>
#include "hn_mesh_grid.h"

using namespace godot;

void HNMeshCube::_process(const Array &p_inputs) {
    float size_x = 1;
    float size_y = 1;
    float size_z = 1;

    int verts_x = 2;
    int verts_y = 2;
    int verts_z = 2;

    if (p_inputs.size() > 0) {
        Array inputs = p_inputs.duplicate();

        if (inputs[0].get_type() == Variant::ARRAY) {
            Array a = inputs[0];
            Vector3 v3;
            v3 = a[0];
            size_x = v3.x;
            size_y = v3.y;
            size_z = v3.z;
        }
        if (inputs[1].get_type() == Variant::ARRAY) {
            Array a = inputs[1];
            verts_x = MAX((int)a[0], verts_x);
        }
        if (inputs[2].get_type() == Variant::ARRAY) {
            Array a = inputs[2];
            verts_y = MAX((int)a[0], verts_y);
        }
        if (inputs[3].get_type() == Variant::ARRAY) {
            Array a = inputs[3];
            verts_z = MAX((int)a[0], verts_z);
        }
    }

    // size_x = size_x * 0.5;
    // size_y = size_y * 0.5;
    // size_z = size_z * 0.5;

    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    int index_offset = 0;
    Transform3D front_frame = Transform3D(Basis(Vector3(0,0,1), Math_PI), Vector3(0, 0, -size_z * 0.5));
    Array front_plane = HNMeshGrid::generate_plane(size_x, size_y, verts_x, verts_y, front_frame);
    PackedVector3Array fp_verts = front_plane[0];
    index_offset += fp_verts.size();

    Transform3D back_frame = Transform3D(Basis(Vector3(1,0,0), Vector3(0,-1,0), Vector3(0,0,-1)), Vector3(0, 0, size_z * 0.5));
    Array back_plane = HNMeshGrid::generate_plane(size_x, size_y, verts_x, verts_y, back_frame, index_offset);
    PackedVector3Array bf_verts = back_plane[0];
    index_offset += bf_verts.size();

    Transform3D top_frame = Transform3D(Basis(Vector3(1,0,0), Math_PI * 0.5), Vector3(0, size_y * 0.5, 0));
    Array top_plane = HNMeshGrid::generate_plane(size_x, size_z, verts_x, verts_z, top_frame, index_offset);
    PackedVector3Array tf_verts = top_plane[0];
    index_offset += tf_verts.size();

    Transform3D bottom_frame = Transform3D(Basis(Vector3(1,0,0), Vector3(0,0,-1), Vector3(0,1,0)), Vector3(0, -size_y * 0.5, 0));
    Array bottom_plane = HNMeshGrid::generate_plane(size_x, size_z, verts_x, verts_z, bottom_frame, index_offset);
    PackedVector3Array botf_verts = bottom_plane[0];
    index_offset += botf_verts.size();

    Transform3D right_frame = Transform3D(Basis(Vector3(0,0,-1), Vector3(0,-1,0), Vector3(-1,0,0)), Vector3(size_x * 0.5, 0, 0));
    Array right_plane = HNMeshGrid::generate_plane(size_z, size_y, verts_z, verts_y, right_frame, index_offset);
    PackedVector3Array rf_verts = right_plane[0];
    index_offset += rf_verts.size();

    Transform3D left_frame = Transform3D(Basis(Vector3(0,0,1), Vector3(0,-1,0), Vector3(1,0,0)), Vector3(-size_x * 0.5, 0, 0));
    Array left_plane = HNMeshGrid::generate_plane(size_z, size_y, verts_z, verts_y, left_frame, index_offset);
    PackedVector3Array lf_verts = left_plane[0];
    index_offset += lf_verts.size();

    vertices.append_array(fp_verts);
    vertices.append_array(bf_verts);
    vertices.append_array(tf_verts);
    vertices.append_array(botf_verts);
    vertices.append_array(rf_verts);
    vertices.append_array(lf_verts);

    indices.append_array(front_plane[1]);
    indices.append_array(back_plane[1]);
    indices.append_array(top_plane[1]);
    indices.append_array(bottom_plane[1]);
    indices.append_array(right_plane[1]);
    indices.append_array(left_plane[1]);

    normals.append_array(front_plane[2]);
    normals.append_array(back_plane[2]);
    normals.append_array(top_plane[2]);
    normals.append_array(bottom_plane[2]);
    normals.append_array(right_plane[2]);
    normals.append_array(left_plane[2]);

    uvs.append_array(front_plane[3]);
    uvs.append_array(back_plane[3]);
    uvs.append_array(top_plane[3]);
    uvs.append_array(bottom_plane[3]);
    uvs.append_array(right_plane[3]);
    uvs.append_array(left_plane[3]);

    out_arr.resize(ArrayMesh::ARRAY_MAX);
    out_arr[ArrayMesh::ARRAY_VERTEX] = vertices;
    out_arr[ArrayMesh::ARRAY_NORMAL] = normals;
    out_arr[ArrayMesh::ARRAY_TEX_UV] = uvs;
    out_arr[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNMeshCube::get_caption() const {
    return "Cube";
}

int HNMeshCube::get_input_port_count() const {
    return 4;
}

HNMeshCube::PortType HNMeshCube::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 1:
            return PortType::PORT_TYPE_SCALAR_UINT;
        case 2:
            return PortType::PORT_TYPE_SCALAR_UINT;
        case 3:
            return PortType::PORT_TYPE_SCALAR_UINT;
    }

    return PortType::PORT_TYPE_SCALAR;
}

String HNMeshCube::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Size";
        case 1:
            return "Vertices X";
        case 2:
            return "Vertices Y";
        case 3:
            return "Vertices Z";
        default:
            return "";
    }
}

int HNMeshCube::get_output_port_count() const {
    return 1;
}

HNMeshCube::PortType HNMeshCube::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNMeshCube::get_output_port_name(int p_port) const {
    return "Mesh";
}

const Variant HNMeshCube::get_output(int p_port) const {
    return Variant(out_arr);
}