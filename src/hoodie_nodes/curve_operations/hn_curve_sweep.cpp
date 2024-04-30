#include "hn_curve_sweep.h"

#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HNCurveSweep::set_flip(const bool p_value) {
    flip = p_value;
}

bool HNCurveSweep::get_flip() const {
    return flip;
}

void HNCurveSweep::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_flip", "value"), &HNCurveSweep::set_flip);
    ClassDB::bind_method(D_METHOD("get_flip"), &HNCurveSweep::get_flip);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Flip", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_flip", "get_flip");
}

void HNCurveSweep::_process(const Array &p_inputs) {
    Array curve_in = p_inputs[0].duplicate();
    Array normals_in = p_inputs[1].duplicate();
    Array profile_in = p_inputs[2].duplicate();
    Array closed_in = p_inputs[3].duplicate();

    Ref<HoodieGeo> curve;
    PackedVector3Array curve_nor;
    Ref<HoodieGeo> profile;
    bool closed = false;

    curve = curve_in[0];
    curve_nor = normals_in;
    profile = profile_in[0];
    closed = closed_in[0];

    const int shape_size = profile.ptr()->points.size();
    const int shape_verts_size = shape_size + (closed ? 1 : 0);
    const int path_size = curve.ptr()->points.size();
    const int total_verts = shape_verts_size * path_size;
    const int total_indices = 2 * 3 * (path_size - 1) * (shape_verts_size - 1);

    out.clear();
    out.resize(2);

    // Check stuff.
    if (shape_size <= 1) {
        UtilityFunctions::push_warning(get_class(), " node: Shape has not enough points (needs at least 2).");
        return;
    }
    if (closed && (shape_size == 2)) {
        UtilityFunctions::push_warning(get_class(), " node: Shape has not enough points to be closed (needs at least 3).");
        return;
    }

    curve_nor.resize(path_size);

    // Mesh values?
    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedFloat32Array tangents;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    // Surface Arrays resize
    vertices.resize(total_verts);
    normals.resize(total_verts);
    tangents.resize(total_verts * 4);
    uvs.resize(total_verts);
    indices.resize(total_indices);

    PackedVector3Array curve_pos = curve.ptr()->points;
    PackedVector3Array profile_pos = profile.ptr()->points;

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
    // Each quad is a primitive
    int triangle_index = 0;
    for (int p = 0; p < path_size; p++) {
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
            HoodieGeo::Primitive prim;
            PackedInt32Array prim_ids;

            int index = p * shape_verts_size + s;
            vertices[index] = frame.xform(profile_pos[s]);
            normals[index] = curve_nor[p];
            uvs[index] = Vector2(p, s);
            if (p > path_size - 2) continue;
            if (s > shape_verts_size - 2) continue;

            int i = p * shape_verts_size + s;

            if (flip) {
                // OLD
                // indices[triangle_index++] = i + shape_verts_size;
                // indices[triangle_index++] = i + 1;
                // indices[triangle_index++] = i;
                // indices[triangle_index++] = i + shape_verts_size;
                // indices[triangle_index++] = i + shape_verts_size + 1;
                // indices[triangle_index++] = i + 1;
                // NEW
                
            } else {
                // OLD
                // indices[triangle_index++] = i;
                // indices[triangle_index++] = i + 1;
                // indices[triangle_index++] = i + shape_verts_size;
                // indices[triangle_index++] = i + 1;
                // indices[triangle_index++] = i + shape_verts_size + 1;
                // indices[triangle_index++] = i + shape_verts_size;
                // NEW
            }
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

    // out[0] = vertices;
    // out[1] = normals;
}

String HNCurveSweep::get_caption() const {
    return "Curve Sweep";
}

int HNCurveSweep::get_input_port_count() const {
    return 4;
}

HoodieNode::PortType HNCurveSweep::get_input_port_type(int p_port) const {
    switch (p_port) {
        case 0:
            return PortType::PORT_TYPE_HGEO;
        case 1:
            return PortType::PORT_TYPE_VECTOR_3D;
        case 2:
            return PortType::PORT_TYPE_HGEO;
        case 3:
            return PortType::PORT_TYPE_BOOLEAN;
        default:
            return PortType::PORT_TYPE_SCALAR;
    }
}

String HNCurveSweep::get_input_port_name(int p_port) const {
    switch (p_port) {
        case 0:
            return "Curve";
        case 1:
            return "Normals";
        case 2:
            return "Profile";
        case 3:
            return "Closed";
        default:
            return "";
    }
}

int HNCurveSweep::get_output_port_count() const {
    return 1;
}

HoodieNode::PortType HNCurveSweep::get_output_port_type(int p_port) const {
    return PortType::PORT_TYPE_HGEO;
}

String HNCurveSweep::get_output_port_name(int p_port) const {
    return "Geo";
}

int HNCurveSweep::get_property_input_count() const {
    return 1;
}

Variant::Type HNCurveSweep::get_property_input_type(vec_size_t p_prop) const {
    return Variant::BOOL;
}

Variant HNCurveSweep::get_property_input(vec_size_t p_port) const {
    return Variant(flip);
}

void HNCurveSweep::set_property_input(vec_size_t p_prop, Variant p_input) {
    flip = (bool)p_input;
}

Vector<StringName> HNCurveSweep::get_editable_properties() const {
    Vector<StringName> props;
    props.push_back("Flip");
    // TODO: VisualShaderNodeIntParameter::get_editable_properties()
    return props;
}

HashMap<StringName, String> HNCurveSweep::get_editable_properties_names() const {
    return HashMap<StringName, String>();
}

const Variant HNCurveSweep::get_output(int p_port) const {
    return Variant(out);
}