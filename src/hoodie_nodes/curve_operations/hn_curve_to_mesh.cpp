#include "hn_curve_to_mesh.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNCurveToMesh::set_flip(const bool p_value) {
    flip = p_value;
}

bool HNCurveToMesh::get_flip() const {
    return flip;
}

void HNCurveToMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_flip", "value"), &HNCurveToMesh::set_flip);
    ClassDB::bind_method(D_METHOD("get_flip"), &HNCurveToMesh::get_flip);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Flip", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_flip", "get_flip");
}

void HNCurveToMesh::_process(const Array &p_inputs) {
    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedFloat32Array tangents;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    mesh.clear();
    mesh.resize(ArrayMesh::ARRAY_MAX);

    // Input parsing
    curve = p_inputs[0];
    if (curve.size() < 4) {
        return;
    }
    PackedVector3Array curve_pos = curve[0] ? curve[0] : Variant();
    PackedVector3Array curve_tan = curve[1] ? curve[1] : Variant();
    PackedVector3Array curve_nor = curve[2] ? curve[2] : Variant();
    PackedFloat32Array curve_tilt = curve[3] ? curve[3] : Variant();
    profile = p_inputs[1];
    if (profile.size() < 4) {
        return;
    }
    PackedVector3Array profile_pos = profile[0] ? profile[0] : Variant();
    PackedVector3Array profile_tan = profile[1] ? profile[1] : Variant();
    PackedVector3Array profile_nor = profile[2] ? profile[2] : Variant();
    PackedFloat32Array profile_tilt = profile[3] ? profile[3] : Variant();
    // PackedVector3Array profile_pos = p_inputs[1].get_type() == get_input_port_type(1) ? p_inputs[1] : Variant();
    bool closed = false;
    if (p_inputs.size() > 2) {
        closed = (bool)p_inputs[2];
    }

    // PackedVector3Array shape = p_inputs[0].get_type() == get_input_connector_type(0) ? p_inputs[0] : Variant();
    // bool closed = p_inputs[1].get_type() == get_input_connector_type(1) ? p_inputs[1] : shape_is_closed;
    // PackedVector3Array curve_pos = p_inputs[2].get_type() == get_input_connector_type(2) ? p_inputs[2] : Variant();
    // PackedVector3Array curve_tan = p_inputs[3].get_type() == get_input_connector_type(3) ? p_inputs[3] : Variant();
    // PackedVector3Array curve_nor = p_inputs[4].get_type() == get_input_connector_type(4) ? p_inputs[4] : Variant();
    // PackedFloat32Array curve_tilt = p_inputs[5].get_type() == get_input_connector_type(5) ? p_inputs[5] : Variant();

    const int shape_size = profile_pos.size();
    const int shape_verts_size = shape_size + (closed ? 1 : 0);
    const int path_size = curve_pos.size();
    const int total_verts = shape_verts_size * path_size;
    // const int total_indices = 3 * (path_size - 1) * shape_verts_size;
    const int total_indices = 2 * 3 * (path_size - 1) * (shape_verts_size - 1);

    // Check stuff.
    if (shape_size <= 1) {
        UtilityFunctions::push_warning(get_class(), " node: Shape has not enough points (needs at least 2).");
        return;
    }
    if (closed && (shape_size == 2)) {
        UtilityFunctions::push_warning(get_class(), " node: Shape has not enough points to be closed (needs at least 3).");
        return;
    }
    if (curve_pos.size() <= 1) {
        UtilityFunctions::push_warning(get_class(), " node: Path has not enough points (needs at least 2).");
        return;
    }
    if (curve_tan.size() != curve_pos.size()) {
        UtilityFunctions::push_warning(get_class(), " node: Path size != Path Tangents size.");
        return;
    }
    if (curve_nor.size() != curve_pos.size()) {
        UtilityFunctions::push_warning(get_class(), " node: Path size != Path Normals size.");
        return;
    }
    if (curve_tilt.size() != curve_pos.size()) {
        UtilityFunctions::push_warning(get_class(), " node: Path size != Path Tilts size.");
        return;
    }

    /*
    // Calculation
    // Shape Normals: vectors that point to the next profile_pos point
    PackedVector3Array shape_normals;
    for (int i = 0; i < shape_size; i++)
    {
        Vector3 origin, next, n;
        if (closed) {
            origin = shape[i];
            next = shape[(i + 1) % shape_size];
        } else {
            origin = shape[i];
            // TODO: implement proper not closed profile_pos normal calculation
            next = shape[(i + 1) % shape_size];
        }
        n = (next - origin).normalized().cross(Vector3(0, 0, 1));
        shape_normals.push_back(n);
    }
    */
    
    // Surface Arrays resize
    vertices.resize(total_verts);
    normals.resize(total_verts);
    tangents.resize(total_verts * 4);
    uvs.resize(total_verts);
    indices.resize(total_indices);

    // Shape Length
    float shape_length = 0;
    {
        Vector3 prev_pt = profile_pos[0];
        for (int j = 1; j < shape_verts_size; j++)
        {
            Vector3 pt = profile_pos[j%shape_size];
            shape_length += (pt - prev_pt).length();
            prev_pt = pt;
        }
    }
    
    // Extrusion
    // This is basically a double loop: for each path point -> for each profile_pos point -> build verts, norms, indices etc.
    int triangle_index = 0;
    for (int p = 0; p < path_size; p++)
    {
        // Construct frame with vectors taken from the Curve3D and tilt.
        Transform3D frame;
        if (p < path_size - 1) {
            frame = Transform3D(-curve_tan[p].cross(curve_nor[p]).normalized(), curve_nor[p], curve_tan[p], curve_pos[p]);
            frame.rotate_basis(frame.basis.get_column(2), curve_tilt[p]);
        } else {
            // Not sure why, but the last row need to be mirrored.
            frame = Transform3D(curve_tan[p].cross(curve_nor[p]).normalized(), curve_nor[p], curve_tan[p], curve_pos[p]);
            frame.rotate_basis(frame.basis.get_column(2), -curve_tilt[p]);
        }
        // Populate arrays
        for (int s = 0; s < shape_verts_size; s++)
        {
            int index = p * shape_verts_size + s;
            vertices[index] = frame.xform(profile_pos[s]);
            normals[index] = curve_nor[p];
            uvs[index] = Vector2(p, s);
            if (p > path_size - 2) continue;
            if (s > shape_verts_size - 2) continue;

            int i = p * shape_verts_size + s;

            if (flip) {
                indices[triangle_index++] = i + shape_verts_size;
                indices[triangle_index++] = i + 1;
                indices[triangle_index++] = i;
                indices[triangle_index++] = i + shape_verts_size;
                indices[triangle_index++] = i + shape_verts_size + 1;
                indices[triangle_index++] = i + 1;
            } else {
                indices[triangle_index++] = i;
                indices[triangle_index++] = i + 1;
                indices[triangle_index++] = i + shape_verts_size;
                indices[triangle_index++] = i + 1;
                indices[triangle_index++] = i + shape_verts_size + 1;
                indices[triangle_index++] = i + shape_verts_size;
            }
        }
    }

    mesh[ArrayMesh::ARRAY_VERTEX] = vertices;
    mesh[ArrayMesh::ARRAY_NORMAL] = normals;
    mesh[ArrayMesh::ARRAY_TANGENT] = tangents;
    mesh[ArrayMesh::ARRAY_TEX_UV] = uvs;
    mesh[ArrayMesh::ARRAY_INDEX] = indices;
}

String HNCurveToMesh::get_caption() const {
    return "Curve to Mesh";
}

int HNCurveToMesh::get_input_port_count() const {
    return 3;
}

HNCurveToMesh::PortType HNCurveToMesh::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_CURVE;
        case 1:
            return PortType::PORT_TYPE_MESH;
        case 2:
            return PortType::PORT_TYPE_BOOLEAN;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNCurveToMesh::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Curve3D";
        case 1:
            return "Profile";
        case 2:
            return "Closed";
        default:
            return "";
    }
}

int HNCurveToMesh::get_output_port_count() const {
    return 1;
}

HNCurveToMesh::PortType HNCurveToMesh::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_MESH;
}

String HNCurveToMesh::get_output_port_name(int p_port) const {
    return "Mesh";
}

int HNCurveToMesh::get_property_input_count() const {
    return 1;
}

Variant::Type HNCurveToMesh::get_property_input_type(vec_size_t p_prop) const {
    return Variant::BOOL;
}

Variant HNCurveToMesh::get_property_input(vec_size_t p_port) const {
    return Variant(flip);
}

void HNCurveToMesh::set_property_input(vec_size_t p_prop, Variant p_input) {
    flip = (bool)p_input;
}

Vector<StringName> HNCurveToMesh::get_editable_properties() const {
    Vector<StringName> props;
    props.push_back("Flip");
    // TODO: VisualShaderNodeIntParameter::get_editable_properties()
    return props;
}

HashMap<StringName, String> HNCurveToMesh::get_editable_properties_names() const {
    return HashMap<StringName, String>();
}

const Variant HNCurveToMesh::get_output(int p_port) const {
    return Variant(mesh);
}