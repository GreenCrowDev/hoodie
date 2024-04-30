#include "hoodie_mesh.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/classes/material.hpp>

using namespace godot;

bool HoodieMesh::get_verbose_mode() const {
    return verbose_mode;
}

void HoodieMesh::set_verbose_mode(bool p_mode) {
    verbose_mode = p_mode;
}

TypedArray<Dictionary> HoodieMesh::_get_node_connections() const {
    const Graph *g = &graph;

    TypedArray<Dictionary> ret;
    for (const Connection &E : g->connections) {
        Dictionary d;
        d["l_node"] = E.l_node;
        d["l_port"] = E.l_port;
        d["r_node"] = E.r_node;
        d["r_port"] = E.r_port;
        ret.push_back(d);
    }

    return ret;
}

void HoodieMesh::get_node_connections(List<Connection> *r_connections) const {
    for (const Connection &E : graph.connections) {
        r_connections->push_back(E);
    }
}

void HoodieMesh::_queue_update() {
    if (dirty.is_set()) {
        return;
    }

    dirty.set();
    call_deferred(StringName("_update"));
}

void HoodieMesh::_update() {
    if (!dirty.is_set()) {
        return;
    }

    dirty.clear();

    _remove_orphan_surfaces();

    for (const KeyValue<id_t, Node> &E : graph.nodes) {
        E.value.node->set_status(HoodieNode::ProcessStatus::PENDING);
    }

    // Iterate on all output nodes
    for (const KeyValue<id_t, Node> &E : graph.nodes) {
        if (E.value.node->get_output_port_count() != 0) {
            continue;
        }

        bool changed = _update_node(E.key, E.value.node);

        if (!changed) {
            continue;
        }

        Ref<Material> material;

        String name = String::num_int64(E.key);
        int32_t surface_id = surface_find_by_name(name);
        if (surface_id != -1) {
            material = surface_get_material(surface_id);
            _remove_surface_dumb(surface_id);
        }

        // Recover stored material for undo redo purposes.
        if (material.is_null()) {
            material = materials[E.key];
        }

        Variant surface = E.value.node->get_output(0);

        if (surface.get_type() != Variant::ARRAY) {
            UtilityFunctions::push_warning("Output node is not returing a Variant::ARRAY");
        } else {
            Array arr = surface;

            if (arr.size() != ArrayMesh::ARRAY_MAX) {
                UtilityFunctions::push_warning("Output node is not returning a correctly sized array!\n");
            } else if (arr[ArrayMesh::ARRAY_VERTEX].get_type() != Variant::PACKED_VECTOR3_ARRAY) {
                UtilityFunctions::push_warning("Output node array ARRAY_VERTEX is not of POOL_VECTOR3_ARRAY type!\n");
            } else if (((PackedVector3Array)arr[ArrayMesh::ARRAY_VERTEX]).size() == 0) {
                UtilityFunctions::push_warning("No vertices in output node ARRAY_VERTEX array!");
            } else if (arr[ArrayMesh::ARRAY_INDEX].get_type() != Variant::PACKED_INT32_ARRAY) {
                UtilityFunctions::push_warning("No indices in final node ARRAY_INDEX array!");
            } else {
                // All requirements passed. Add valid surface.
                int32_t new_surface_id = get_surface_count();
                add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);
                surface_set_name(new_surface_id, name);
                if (material.is_valid()) {
                    surface_set_material(new_surface_id, material);
                    // Store material for undo redo purposes.
                    materials[E.key] = material;
                }
            }
        }
    }

#ifdef TOOLS_ENABLED
    if (verbose_mode) {
        UtilityFunctions::print("HoodieMesh update end. Surface count: " + itos(get_surface_count()) + ".");
    }
#endif // TOOLS_ENABLED
}

// TODO: replace this with surface_remove in Godot 4.3
void HoodieMesh::_remove_orphan_surfaces() {
    // Basically you want to check if there's a node (usually a Output HoodieNode) that generates that surface, giving it its name
    // (e.g. Node name: 0 (it's the id), Surface name: 0)
    // If the node 0 is not there anymore, we need to remove that surface.

    // In this array we will store the arrays with which we will reconstruct the surfaces to keep
    Array surf_arrays_to_keep;
    Array surf_names;
    Array surf_materials;
    // Iterate over all surfaces
    for (int64_t s = get_surface_count() - 1; s >= 0; s--) {
        String surface_name = surface_get_name(s);
        bool node_found = false;
        // Iterate over all nodes
        for (const KeyValue<id_t, Node> &E : graph.nodes) {
            String node_name = String::num_int64(E.key);
            if ((node_name == surface_name) && (E.value.node->get_output_port_count() == 0)) {
                // Found the node with the same name of the surface
                node_found = true;
            }
        }
        if (node_found) {
            // We want to keep this surface, so we store its arrays and name
            surf_arrays_to_keep.push_back(surface_get_arrays(s));
            surf_names.push_back(surface_name);
            surf_materials.push_back(surface_get_material(s));
        }
    }
    // We stored all the surfaces to keep, time to clear...
    clear_surfaces();
    // ...and re-add them!
    for (int64_t i = 0; i < surf_arrays_to_keep.size(); i++) {
        add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surf_arrays_to_keep[i]);
        surface_set_name(i, surf_names[i]);
        surface_set_material(i, surf_materials[i]);
        materials[surf_names[i]] = surf_materials[i];
    }
}

// TODO: replace this with surface_remove in Godot 4.3
void HoodieMesh::_remove_surface_dumb(id_t p_id) {
    // Basically rebuilds all surface except the one selected
    Array surf_arrays_to_keep;
    Array surf_names;
    Array surf_materials;
    for (int64_t s = get_surface_count() - 1; s >= 0; s--) {
        if (s != p_id) {
            surf_arrays_to_keep.push_back(surface_get_arrays(s));
            surf_names.push_back(surface_get_name(s));
            surf_materials.push_back(surface_get_material(s));
        }
    }
    clear_surfaces();
    for (int64_t i = 0; i < surf_arrays_to_keep.size(); i++) {
        add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surf_arrays_to_keep[i]);
        surface_set_name(i, surf_names[i]);
        surface_set_material(i, surf_materials[i]);
        materials[surf_names[i]] = surf_materials[i];
    }
}

bool HoodieMesh::_update_node(id_t p_id, Ref<HoodieNode> p_node) {
    switch (p_node->get_status())
    {
        case HoodieNode::ProcessStatus::INPROGRESS: 
            UtilityFunctions::push_warning("Found circular dependency in procedural mesh!");
            return false;
        case HoodieNode::ProcessStatus::UNCHANGED:
#ifdef TOOLS_ENABLED
            if (verbose_mode) {
                UtilityFunctions::print("[" + itos(p_id) + "] " + p_node->get_caption() + " node unchanged.");
            }
#endif // TOOLS_ENABLED
            return false;
        case HoodieNode::ProcessStatus::CHANGED:
#ifdef TOOLS_ENABLED
            if (verbose_mode) {
                UtilityFunctions::print("[" + itos(p_id) + "] " + p_node->get_caption() + " node already processed.");
            }
#endif // TOOLS_ENABLED
            return true;
        case HoodieNode::ProcessStatus::PENDING:
#ifdef TOOLS_ENABLED
            if (verbose_mode) {
                UtilityFunctions::print("[" + itos(p_id) + "] " + p_node->get_caption() + " node status pending: updating the node...");
            }
#endif // TOOLS_ENABLED
            
            bool updated = false;
            Array inputs;

            p_node->set_status(HoodieNode::ProcessStatus::INPROGRESS);

            for (int i = 0; i < p_node->get_input_port_count(); i++)
            {
                Vector<HoodieMesh::NodePortPair> l_ports = get_left_ports(p_id, i);

                // TODO: implement more than one l_port
                if (l_ports.size() > 1) {
                    UtilityFunctions::push_warning("More than one l_port NOT IMPLEMENTED.");
                    continue;
                } else if (l_ports.size() == 0) {
                    inputs.push_back(Variant());
                    continue;
                }

                // Make sure l_node is valid
                RBMap<id_t, Node>::ConstIterator E = graph.nodes.find(l_ports[0].node);
                if (!E) {
                    UtilityFunctions::push_warning("l_node not found.");
                    continue;
                }

                id_t l_id = E->key;
                Ref<HoodieNode> l_node = E->value.node;

                // Make sure it has been updated
                if (_update_node(l_id, l_node)) {
                    updated = true;
                }

                Variant l_data = l_node->get_output(l_ports[0].port);

                HoodieNode::PortType l_type = l_node->get_output_port_type(l_ports[0].port);
                HoodieNode::PortType r_type = p_node->get_input_port_type(i);

                // TODO: implement data conversion

                if (!is_port_types_compatible((int)l_type, (int)r_type)) {
                    UtilityFunctions::push_warning("l_port type and r_port type type don't match!");
                    l_data = Variant();
                }

                inputs.push_back(l_data);
            }

            if (p_node->update(updated, inputs)) {
                updated = true;
            }

            p_node->set_status(updated ? HoodieNode::ProcessStatus::CHANGED : HoodieNode::ProcessStatus::UNCHANGED);
            
            return updated;
    }
}

void HoodieMesh::add_node(const Ref<HoodieNode> &p_node, const Vector2 &p_position, id_t p_id) {
    ERR_FAIL_COND(graph.nodes.has(p_id));
    Node n;
    n.node = p_node;
    n.position = p_position;

    n.node->connect("changed", callable_mp(this, &HoodieMesh::_queue_update));

    graph.nodes[p_id] = n;

    if (p_node->get_property_input_count() > 0) {
        notify_property_list_changed();
    }

    _queue_update();
}

void HoodieMesh::set_node_position(id_t p_id, const Vector2 &p_position) {
    ERR_FAIL_COND(!graph.nodes.has(p_id));
    graph.nodes[p_id].position = p_position;
}

void HoodieMesh::remove_node(id_t p_id) {
    // TODO: not sure why this avoids a out of bounds error. Check this with 4.3 and surface_remove()
    _remove_orphan_surfaces();

    int prop_count = graph.nodes[p_id].node->get_property_input_count();

    graph.nodes[p_id].node->disconnect("changed", callable_mp(this, &HoodieMesh::_queue_update));

    graph.nodes.erase(p_id);

    for (List<Connection>::Element *E = graph.connections.front(); E;) {
        List<Connection>::Element *N = E->next();
        const Connection &connection = E->get();
        if (connection.l_node == p_id || connection.r_node == p_id) {
            if (connection.l_node == p_id) {
                graph.nodes[connection.r_node].prev_connected_nodes.erase(p_id);
                graph.nodes[connection.r_node].node->set_input_port_connected(connection.r_port, false);
                // TODO: check if this is correct
                graph.nodes[connection.r_node].node->mark_dirty();
            } else if (connection.r_node == p_id) {
                graph.nodes[connection.l_node].next_connected_nodes.erase(p_id);
            }
            graph.connections.erase(E);
        }
        E = N;
    }

    if (prop_count > 0) {
        notify_property_list_changed();
    }

    _queue_update();
}

Ref<HoodieNode> HoodieMesh::get_node(id_t p_id) const {
    ERR_FAIL_COND_V(!graph.nodes.has(p_id), Ref<HoodieNode>());
    return graph.nodes[p_id].node;
}

Vector2 HoodieMesh::get_node_position(id_t p_id) const {
    ERR_FAIL_COND_V(!graph.nodes.has(p_id), Vector2());
    return graph.nodes[p_id].position;
}

Vector<HoodieMesh::id_t> HoodieMesh::get_nodes_id_list() const {
    Vector<id_t> ret;
    for (const KeyValue<id_t, HoodieMesh::Node> &E : graph.nodes) {
        ret.push_back(E.key);
    }

    return ret;
}

HoodieMesh::id_t HoodieMesh::get_valid_node_id() const {
    return graph.nodes.size() ? MAX(1, graph.nodes.back()->key() + 1) : 1;
}

const Vector<HoodieMesh::NodePortPair> HoodieMesh::get_left_ports(id_t p_r_node, vec_size_t p_r_port) const {
    size_t max = graph.connections.size();
    if (max == 0) {
        return Vector<HoodieMesh::NodePortPair>();
    }

    Vector<HoodieMesh::NodePortPair> vec;

    for (size_t i = max - 1; i >= 0 && i < max; i--) {
        Connection c = graph.connections[i];
        if ((c.r_node == p_r_node) && (c.r_port == p_r_port)) {
            vec.push_back(HoodieMesh::NodePortPair(c.l_node, c.l_port));
        }
    }
    
    return vec;
}

void HoodieMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_node", "node", "position", "id"), &HoodieMesh::add_node);
    ClassDB::bind_method(D_METHOD("remove_node", "id"), &HoodieMesh::remove_node);

    ClassDB::bind_method(D_METHOD("get_node_position", "id"), &HoodieMesh::get_node_position);
    ClassDB::bind_method(D_METHOD("set_node_position", "id", "position"), &HoodieMesh::set_node_position);

    ClassDB::bind_method(D_METHOD("connect_nodes", "l_node", "l_port", "r_node", "r_port"), &HoodieMesh::connect_nodes);
    ClassDB::bind_method(D_METHOD("disconnect_nodes", "l_node", "l_port", "r_node", "r_port"), &HoodieMesh::disconnect_nodes);
    ClassDB::bind_method(D_METHOD("connect_nodes_forced", "l_node", "l_port", "r_node", "r_port"), &HoodieMesh::connect_nodes_forced);

    ClassDB::bind_method(D_METHOD("set_graph_offset", "offset"), &HoodieMesh::set_graph_offset);
    ClassDB::bind_method(D_METHOD("get_graph_offset"), &HoodieMesh::get_graph_offset);

    ClassDB::bind_method(D_METHOD("_queue_update"), &HoodieMesh::_queue_update);
    ClassDB::bind_method(D_METHOD("_update"), &HoodieMesh::_update);

    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "graph_offset", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_graph_offset", "get_graph_offset");
}

bool HoodieMesh::_set(const StringName &p_name, const Variant &p_value) {
    String prop_name = p_name;
    if (prop_name.begins_with("nodes/")) {
        String index = prop_name.get_slicec('/', 1);
        if (index == "connections") {
            TypedArray<id_t> conns = p_value;
            if (conns.size() % 4 == 0) {
                for (int i = 0; i < conns.size(); i += 4) {
                    connect_nodes_forced(conns[i + 0], conns[i + 1], conns[i + 2], conns[i + 3]);
                }
            }
            return true;
        }

        id_t id = index.to_int();
        String what = prop_name.get_slicec('/', 2);

        if (what == "node") {
            add_node(p_value, Vector2(), id);
            return true;
        } else if (what == "position") {
            set_node_position(id, p_value);
            return true;
        }
    } else if (prop_name.begins_with("inputs/")) {
        String input_prop_name = prop_name.get_slicec('/', 1);
        String index = input_prop_name.get_slicec('[', 1).get_slicec(']', 0);
        String prop_index = input_prop_name.get_slicec('[', 2).get_slicec(']', 0);

        id_t id = index.to_int();
        int prop_id = prop_index.to_int();

        for (const KeyValue<id_t, Node> &E : graph.nodes) {
            if (E.key == id) {
                for (int i = 0; i < E.value.node->get_property_input_count(); i++) {
                    if (i == prop_id) {
                        E.value.node->set_property_input(i, p_value);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool HoodieMesh::_get(const StringName &p_name, Variant &r_ret) const {
    String prop_name = p_name;
    if (prop_name.begins_with("nodes/")) {
        String index = prop_name.get_slicec('/', 1);
        if (index == "connections") {
            TypedArray<id_t> conns;
            for (const Connection &E : graph.connections) {
                conns.push_back(E.l_node);
                conns.push_back(E.l_port);
                conns.push_back(E.r_node);
                conns.push_back(E.r_port);
            }

            r_ret = conns;
            return true;
        }

        id_t id = index.to_int();
        String what = prop_name.get_slicec('/', 2);

        if (what == "node") {
            r_ret = get_node(id);
            return true;
        } else if (what == "position") {
            r_ret = get_node_position(id);
            return true;
        }
    } else if (prop_name.begins_with("inputs/")) {
        String input_prop_name = prop_name.get_slicec('/', 1);
        String index = input_prop_name.get_slicec('[', 1).get_slicec(']', 0);
        String prop_index = input_prop_name.get_slicec('[', 2).get_slicec(']', 0);

        id_t id = index.to_int();
        int prop_id = prop_index.to_int();

        for (const KeyValue<id_t, Node> &E : graph.nodes) {
            if (E.key == id) {
                for (int i = 0; i < E.value.node->get_property_input_count(); i++) {
                    if (i == prop_id) {
                        r_ret = Variant(E.value.node->get_property_input(i));
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void HoodieMesh::_get_property_list(List<PropertyInfo> *p_list) const {
    // Save HoodieNodes positions and connections.
    for (const KeyValue<id_t, Node> &E : graph.nodes) {
        String prop_name = "nodes/";
        prop_name += itos(E.key);

        p_list->push_back(PropertyInfo(Variant::OBJECT, prop_name + "/node", PROPERTY_HINT_RESOURCE_TYPE, "HoodieNode", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_ALWAYS_DUPLICATE));
        p_list->push_back(PropertyInfo(Variant::VECTOR2, prop_name + "/position", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR));
    }
    p_list->push_back(PropertyInfo(Variant::PACKED_INT32_ARRAY, "nodes/connections", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR));

    // Display input properties.
    // String name will be something like "inputs/[0] Node Caption [0] Curve3D"
    for (const KeyValue<id_t, Node> &E : graph.nodes) {
        String node_name = "[" + itos(E.key) + "] " + E.value.node->get_caption();

        for (int i = 0; i < E.value.node->get_property_input_count(); i++) {
            String prop_name = "[" + itos(i) + "] " + E.value.node->get_property_input_hint(i);
            Variant::Type type = E.value.node->get_property_input_type(i);

            if (type != Variant::OBJECT || type == Variant::NIL) {
                continue;
            }

            String name = "inputs/";
            name += node_name + " " + prop_name;

            if (type == GDEXTENSION_VARIANT_TYPE_OBJECT) {
                p_list->push_back(PropertyInfo(type, name, PROPERTY_HINT_RESOURCE_TYPE, E.value.node->get_property_input_hint(i)));
            }
        }
    }
}

bool HoodieMesh::is_nodes_connected_relatively(const Graph *p_graph, id_t p_node, id_t p_target) const {
    bool result = false;

    const HoodieMesh::Node &node = p_graph->nodes[p_node];

    for (const id_t &E : node.prev_connected_nodes) {
        if (E == p_target) {
            return true;
        }

        result = is_nodes_connected_relatively(p_graph, E, p_target);
        if (result) {
            break;
        }
    }
    return result;
}

bool HoodieMesh::can_connect_nodes(id_t p_from_node, vec_size_t p_from_port, id_t p_to_node, vec_size_t p_to_port) const {
    if (!graph.nodes.has(p_from_node)) {
        return false;
    }

    if (p_from_node == p_to_node) {
        return false;
    }

    if (p_from_port < 0 || p_from_port >= graph.nodes[p_from_node].node->get_output_port_count()) {
        return false;
    }

    if (!graph.nodes.has(p_to_node)) {
        return false;
    }

    if (p_to_port < 0 || p_to_port >= graph.nodes[p_to_node].node->get_input_port_count()) {
        return false;
    }

    HoodieNode::PortType l_port_type = graph.nodes[p_from_node].node->get_output_port_type(p_from_port);
    HoodieNode::PortType r_port_type = graph.nodes[p_to_node].node->get_input_port_type(p_to_port);

    if (!is_port_types_compatible(l_port_type, r_port_type)) {
        return false;
    }

    for (const Connection &E : graph.connections) {
        if (E.l_node == p_from_node && E.l_port == p_from_node && E.r_node == p_to_node && E.r_port == p_to_port) {
            return false;
        }
    }

    if (is_nodes_connected_relatively(&graph, p_from_node, p_to_node)) {
        return false;
    }

    return true;
}

void HoodieMesh::connect_nodes_forced(id_t p_from_node, vec_size_t p_from_port, id_t p_to_node, vec_size_t p_to_port) {
    ERR_FAIL_COND(!graph.nodes.has(p_from_node));
    ERR_FAIL_INDEX(p_from_port, graph.nodes[p_from_node].node->get_output_port_count());
    ERR_FAIL_COND(!graph.nodes.has(p_to_node));
    ERR_FAIL_INDEX(p_to_port, graph.nodes[p_to_node].node->get_input_port_count());

    for (const Connection &E : graph.connections) {
        if (E.l_node == p_from_node && E.l_port == p_from_port && E.r_node == p_to_node && E.r_port == p_to_port) {
            return;
        }
    }

    Connection c;
    c.l_node = p_from_node;
    c.l_port = p_from_port;
    c.r_node = p_to_node;
    c.r_port = p_to_port;
    graph.connections.push_back(c);
    graph.nodes[p_from_node].next_connected_nodes.push_back(p_to_node);
    graph.nodes[p_to_node].prev_connected_nodes.push_back(p_from_node);
    graph.nodes[p_from_node].node->set_output_port_connected(p_from_port, true);
    graph.nodes[p_to_node].node->set_input_port_connected(p_to_port, true);

    graph.nodes[p_to_node].node->mark_dirty();
    _queue_update();
}

Error HoodieMesh::connect_nodes(id_t p_from_node, vec_size_t p_from_port, id_t p_to_node, vec_size_t p_to_port) {
    ERR_FAIL_COND_V(!graph.nodes.has(p_from_node), ERR_INVALID_PARAMETER);
    ERR_FAIL_INDEX_V(p_from_port, graph.nodes[p_from_node].node->get_output_port_count(), ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(!graph.nodes.has(p_to_node), ERR_INVALID_PARAMETER);
    ERR_FAIL_INDEX_V(p_to_port, graph.nodes[p_to_node].node->get_input_port_count(), ERR_INVALID_PARAMETER);

    HoodieNode::PortType l_port_type = graph.nodes[p_from_node].node->get_output_port_type(p_from_port);
    HoodieNode::PortType r_port_type = graph.nodes[p_to_node].node->get_input_port_type(p_to_port);

    ERR_FAIL_COND_V_MSG(!is_port_types_compatible(l_port_type, r_port_type), ERR_INVALID_PARAMETER, "Incompatible port types.");

    for (const Connection &E : graph.connections) {
        if (E.l_node == p_from_node && E.l_port == p_from_port && E.r_node == p_to_node && E.r_port == p_to_port) {
            ERR_FAIL_V(ERR_ALREADY_EXISTS);
        }
    }

    Connection c;
    c.l_node = p_from_node;
    c.l_port = p_from_port;
    c.r_node = p_to_node;
    c.r_port = p_to_port;
    graph.connections.push_back(c);
    graph.nodes[p_from_node].next_connected_nodes.push_back(p_to_node);
    graph.nodes[p_to_node].prev_connected_nodes.push_back(p_from_node);
    graph.nodes[p_from_node].node->set_output_port_connected(p_from_port, true);
    graph.nodes[p_to_node].node->set_input_port_connected(p_to_port, true);

    graph.nodes[p_to_node].node->mark_dirty();
    _queue_update();
    return OK;
}

void HoodieMesh::disconnect_nodes(id_t p_from_node, vec_size_t p_from_port, id_t p_to_node, vec_size_t p_to_port) {
    for (const List<Connection>::Element *E = graph.connections.front(); E; E = E->next()) {
        if (E->get().l_node == p_from_node && E->get().l_port == p_from_port && E->get().r_node == p_to_node && E->get().r_port == p_to_port) {
            graph.connections.erase(E);
            graph.nodes[p_from_node].next_connected_nodes.erase(p_to_node);
            graph.nodes[p_to_node].prev_connected_nodes.erase(p_from_node);
            graph.nodes[p_from_node].node->set_output_port_connected(p_from_port, false);
            graph.nodes[p_to_node].node->set_input_port_connected(p_to_port, false);

            // TODO: check if this is right
            graph.nodes[p_to_node].node->mark_dirty();
            _queue_update();
            return;
        }
    }
}

bool HoodieMesh::is_port_types_compatible(int p_a, int p_b) const {
    // return MAX(0, p_a - (int)HoodieNode::PORT_TYPE_BOOLEAN) == (MAX(0, p_b - (int)HoodieNode::PORT_TYPE_BOOLEAN));

    HoodieNode::PortType a = (HoodieNode::PortType)p_a;
    HoodieNode::PortType b = (HoodieNode::PortType)p_b;

    if (a == HoodieNode::PortType::PORT_TYPE_CURVE && b == HoodieNode::PortType::PORT_TYPE_MESH) {
        UtilityFunctions::push_warning("Curve type and Geometry type are NOT compatible.");
    }

    if (a == HoodieNode::PortType::PORT_TYPE_MESH && b == HoodieNode::PortType::PORT_TYPE_CURVE) {
        UtilityFunctions::push_warning("Curve type and Geometry type are NOT compatible.");
    }

    if (a == b) {
        return true;
    }

    // DATA
    if (a == HoodieNode::PortType::PORT_TYPE_DATA || b == HoodieNode::PortType::PORT_TYPE_DATA) {
        return true;
    }

    // INT UINT
    if (a == HoodieNode::PortType::PORT_TYPE_SCALAR_INT && b == HoodieNode::PortType::PORT_TYPE_SCALAR_UINT) {
        return true;
    }
    if (a == HoodieNode::PortType::PORT_TYPE_SCALAR_UINT && b == HoodieNode::PortType::PORT_TYPE_SCALAR_INT) {
        return true;
    }

    // VECTOR2D VECTOR3D
    if (a == HoodieNode::PortType::PORT_TYPE_VECTOR_2D && b == HoodieNode::PortType::PORT_TYPE_VECTOR_3D) {
        return true;
    }
    if (a == HoodieNode::PortType::PORT_TYPE_VECTOR_3D && b == HoodieNode::PortType::PORT_TYPE_VECTOR_2D) {
        return true;
    }

    return false;
}

void HoodieMesh::set_graph_offset(const Vector2 &p_offset) {
    graph_offset = p_offset;
}

Vector2 HoodieMesh::get_graph_offset() const {
    return graph_offset;
}

HoodieMesh::HoodieMesh() {
}
