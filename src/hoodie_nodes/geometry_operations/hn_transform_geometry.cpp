#include "hn_transform_geometry.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

// Euler constructor expects a vector containing the Euler angles in the format
// (ax, ay, az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses YXZ convention (Z is the first rotation).
Quaternion HNTransformGeometry::from_euler(const Vector3 &p_euler) {
	real_t half_a1 = p_euler.y * 0.5f;
	real_t half_a2 = p_euler.x * 0.5f;
	real_t half_a3 = p_euler.z * 0.5f;

	// R = Y(a1).X(a2).Z(a3) convention for Euler angles.
	// Conversion to quaternion as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-6)
	// a3 is the angle of the first rotation, following the notation in this reference.

	real_t cos_a1 = Math::cos(half_a1);
	real_t sin_a1 = Math::sin(half_a1);
	real_t cos_a2 = Math::cos(half_a2);
	real_t sin_a2 = Math::sin(half_a2);
	real_t cos_a3 = Math::cos(half_a3);
	real_t sin_a3 = Math::sin(half_a3);

	return Quaternion(
			sin_a1 * cos_a2 * sin_a3 + cos_a1 * sin_a2 * cos_a3,
			sin_a1 * cos_a2 * cos_a3 - cos_a1 * sin_a2 * sin_a3,
			-sin_a1 * sin_a2 * cos_a3 + cos_a1 * cos_a2 * sin_a3,
			sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
}

void HNTransformGeometry::_process(const Array &p_inputs) {
    out.clear();

    if (p_inputs.size() == 0) {
        return;
    }

    out = p_inputs[0].duplicate();

    if (out.size() == 0) {
        return;
    }

    PackedVector3Array mesh_verts = out[ArrayMesh::ARRAY_VERTEX];
    PackedVector3Array new_verts;
    PackedVector3Array mesh_norms = out[ArrayMesh::ARRAY_NORMAL];
    PackedVector3Array new_norms;
    PackedVector2Array mesh_uvs = out[ArrayMesh::ARRAY_TEX_UV];
    PackedVector2Array new_uvs;
    PackedInt32Array mesh_idx = out[ArrayMesh::ARRAY_INDEX];
    PackedInt32Array new_idx;

    // Translate, Rotate, Scale
    Array translations = p_inputs[1].duplicate();
    Array rotations = p_inputs[2].duplicate();
    Array scales = p_inputs[3].duplicate();

    Transform3D transform;
    Basis basis;

    PackedVector3Array t_vecs = translations;
    PackedVector3Array r_vecs = rotations;
    PackedVector3Array s_vecs = scales;

    Vector3 translation_vector = Vector3(0, 0, 0);
    Quaternion rotation_quaternion;
    Vector3 scale_vector = Vector3(1, 1, 1);

    if (translations.size() > 0) {
        translation_vector = translations[0];
    }

    if (rotations.size() > 0) {
        Vector3 rotation_euler = rotations[0];
        rotation_quaternion = from_euler(rotation_euler);
    }

    if (scales.size() > 0) {
        scale_vector = scales[0];
    }

    for (int t = 0; t < t_vecs.size(); t++) {
        if (r_vecs.size() > 0) {
            rotation_quaternion = from_euler(r_vecs[MIN(t, r_vecs.size()-1)]);
        }
        if (s_vecs.size() > 0) {
            scale_vector = s_vecs[MIN(t, s_vecs.size()-1)];
        }

        basis = Basis(rotation_quaternion, scale_vector);
        transform = Transform3D(basis, t_vecs[t]);

        for (int i = 0; i < mesh_verts.size(); i++) {
            Vector3 v = mesh_verts[i];
            new_verts.push_back(transform.xform(v));

            if (mesh_norms.size() > 0) {
                Vector3 n = mesh_norms[i];
                Basis norm_basis = Basis(rotation_quaternion);
                Transform3D norm_transform = Transform3D(norm_basis);
                new_norms.push_back(norm_transform.xform(n));
            }
            if (mesh_uvs.size() > 0) {
                Vector2 uv = mesh_uvs[i];
                new_uvs.push_back(uv);
            }
        }

        for (int i = 0; i < mesh_idx.size(); i++) {
            new_idx.push_back(mesh_idx[i] + mesh_verts.size() * t);
        }
    }

    if (t_vecs.size() > 0) {
        mesh_verts = new_verts;
        mesh_norms = new_norms;
        mesh_uvs = new_uvs;
        mesh_idx = new_idx;
    }
    
    out[ArrayMesh::ARRAY_VERTEX] = mesh_verts;
    out[ArrayMesh::ARRAY_NORMAL] = mesh_norms;
    out[ArrayMesh::ARRAY_TEX_UV] = mesh_uvs;
    out[ArrayMesh::ARRAY_INDEX] = mesh_idx;
}

String HNTransformGeometry::get_caption() const {
    return "Transform Mesh";
}

int HNTransformGeometry::get_input_port_count() const {
    return 4;
}

HNTransformGeometry::PortType HNTransformGeometry::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_MESH;
        case 1:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 2:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 3:
            return PortType::PORT_TYPE_VECTOR_3D;
    }

    return PortType::PORT_TYPE_SCALAR;
}

String HNTransformGeometry::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Geometry";
        case 1:
            return "Translation";
        case 2:
            return "Rotation";
        case 3:
            return "Scale";
    }

    return "";
}

int HNTransformGeometry::get_output_port_count() const {
    return 1;
}

HNTransformGeometry::PortType HNTransformGeometry::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNTransformGeometry::get_output_port_name(int p_port) const {
    return "Geometry";
}

const Variant HNTransformGeometry::get_output(int p_port) const {
    return Variant(out);
}