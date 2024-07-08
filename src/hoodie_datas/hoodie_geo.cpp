#include "hoodie_geo.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void HoodieGeo::_bind_methods() {
}

void HoodieGeo::clear() {
    points.clear();
    attributes.clear();
    primitives.clear();
}

Vector<PackedVector3Array> HoodieGeo::pack_primitive_points() const {
    Vector<PackedVector3Array> ret;

    for (Primitive p : primitives) {
        PackedVector3Array pts;
        pts.resize(p.vertices.size());
        for (int i = 0; i < p.vertices.size(); i++) {
            pts[i] = points[p.vertices[i]];
        }
        ret.push_back(pts);
    }

    return ret;
}

void HoodieGeo::unpack_primitive_points(const TypedArray<PackedVector3Array> &p_array) {
    points.clear();
    primitives.clear();

    UtilityFunctions::print("Unpack array size: " + itos(p_array.size()));

    int verts_counter = 0;
    for (int i = 0; i < p_array.size(); i++) {
        PackedVector3Array pts = p_array[i];
        points.append_array(pts);

        PackedInt32Array verts;
        verts.resize(pts.size());
        for (int p = 0; p < verts.size(); p++) {
            verts[p] = verts_counter++;
        }
        primitives.append(Primitive(verts));

        UtilityFunctions::print("ua [" + itos(i) + "]: " + itos(verts.size()));
    }
}

Vector<HashMap<String, Array>> HoodieGeo::pack_primitive_attributes() const {
    Vector<HashMap<String, Array>> ret;

    for (Primitive p : primitives) {
        HashMap<String, Array> map;

        for (auto a : attributes) {
            Array arr;
            arr.resize(p.vertices.size());

            for (int i = 0; i < p.vertices.size(); i++) {
                arr[i] = map[a.key][p.vertices[i]];
            }

            map[a.key] = arr;
        }

        ret.push_back(map);
    }

    return ret;
}

Array HoodieGeo::to_mesh() const {
    Array mesh;
    mesh.resize(ArrayMesh::ARRAY_MAX);

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

    // Vertices
    if (points.size() > 0) {
        vertices = points;
    }

    // Normals
    if (attributes.has("N")) {
        normals = attributes["N"];
    } else {
        normals.resize(points.size());
    }

    // Tangents
    if (attributes.has("T")) {
        tangents = attributes["T"];
    } else {
        tangents.resize(points.size());
    }

    // Colors
    if (attributes.has("C")) {
        colors = attributes["C"];
    } else {
        colors.resize(points.size());
    }

    // UV
    if (attributes.has("UV")) {
        uv = attributes["UV"];
    } else {
        uv.resize(points.size());
    }

    // UV2
    if (attributes.has("UV2")) {
        uv2 = attributes["UV2"];
    } else {
        uv2.resize(points.size());
    }

    custom0.resize(vertices.size());
    custom1.resize(vertices.size());
    custom2.resize(vertices.size());
    custom3.resize(vertices.size());

    bones.resize(vertices.size());
    weights.resize(vertices.size());

    // Indices
    for (Primitive p : primitives) {
        switch (p.vertices.size())
        {
            case 4:
                indices.push_back(p.vertices[0]);
                indices.push_back(p.vertices[1]);
                indices.push_back(p.vertices[2]);
                indices.push_back(p.vertices[3]);
                indices.push_back(p.vertices[2]);
                indices.push_back(p.vertices[1]);
                break;
            default:
                indices.append_array(p.vertices);
        }
    }

    mesh[ArrayMesh::ARRAY_VERTEX] = vertices;
    mesh[ArrayMesh::ARRAY_NORMAL] = normals;
    // mesh[ArrayMesh::ARRAY_TANGENT] = tangents;
    // mesh[ArrayMesh::ARRAY_COLOR] = colors;
    mesh[ArrayMesh::ARRAY_TEX_UV] = uv;
    // mesh[ArrayMesh::ARRAY_TEX_UV2] = uv2;
    // mesh[ArrayMesh::ARRAY_CUSTOM0] = custom0;
    // mesh[ArrayMesh::ARRAY_CUSTOM1] = custom1;
    // mesh[ArrayMesh::ARRAY_CUSTOM2] = custom2;
    // mesh[ArrayMesh::ARRAY_CUSTOM3] = custom3;
    // mesh[ArrayMesh::ARRAY_BONES] = bones;
    // mesh[ArrayMesh::ARRAY_WEIGHTS] = weights;
    mesh[ArrayMesh::ARRAY_INDEX] = indices;

    return mesh;
}

HashMap<String, String> HoodieGeo::populate_tab_inspector() const {
    HashMap<String, String> tabs;

    String s_points;
    s_points = array_to_string(points);

    String s_primitives;
    for (int i = 0; i < primitives.size(); i++) {
        for (int j = 0; j < primitives[i].vertices.size(); j++) {
            s_primitives += itos(i) + ":" + itos(primitives[i].vertices[j]) + "\n";
        }
    }

    tabs["Points"] = s_points;
    tabs["Primitives"] = s_primitives;

    return tabs;
}

Ref<HoodieData> HoodieGeo::create_reference(const PackedVector3Array &p_points) {
    Ref<HoodieGeo> hgeo;
    hgeo.instantiate();

    hgeo->points = p_points;

    return hgeo;
}

Ref<HoodieData> HoodieGeo::create_reference(const PackedVector3Array &p_points, const Vector<Primitive> &p_primitives) {
    Ref<HoodieGeo> hgeo;
    hgeo.instantiate();

    hgeo->points = p_points;
    hgeo->primitives = p_primitives;

    return hgeo;
}

Ref<HoodieData> HoodieGeo::create_reference(const PackedVector3Array &p_points, const PackedInt32Array &p_vertices) {
    Ref<HoodieGeo> hgeo;
    hgeo.instantiate();

    hgeo->points = p_points;
    hgeo->primitives.append(Primitive(p_vertices));

    return hgeo;
}

HoodieGeo::HoodieGeo() {
}

HoodieGeo::HoodieGeo(PackedVector3Array p_points, Vector<Primitive> p_primitives) {
    points = p_points;
    primitives = p_primitives;
}

HoodieGeo::HoodieGeo(PackedVector3Array p_points, PackedInt32Array p_vertices) {
    points = p_points;
    primitives.append(Primitive(p_vertices));
}