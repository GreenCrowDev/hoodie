#ifndef HOODIE_HOODIENODE_H
#define HOODIE_HOODIENODE_H

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "hoodie_datas/hoodie_data.h"
#include "hoodie_datas/hoodie_geo.h"
#include "hoodie_datas/hoodie_array_mesh.h"
#include "hoodie_datas/hoodie_variant.h"

namespace godot
{

class HoodieNode : public Resource {
    GDCLASS(HoodieNode, Resource)

    friend class HoodieGraphPlugin;
    friend class HoodieEditorPlugin;
    friend class HoodieMesh;

protected:
    typedef int id_t;
    typedef uint32_t vec_size_t;

private:
    HashMap<vec_size_t, bool> connected_input_ports;
    HashMap<vec_size_t, bool> connected_output_ports;

public:
    enum PortType {
		PORT_TYPE_SCALAR,
		PORT_TYPE_SCALAR_INT,
		PORT_TYPE_SCALAR_UINT,
		PORT_TYPE_VECTOR_2D,
		PORT_TYPE_VECTOR_3D,
		PORT_TYPE_VECTOR_4D,
		PORT_TYPE_BOOLEAN,
        PORT_TYPE_COLOR,
        PORT_TYPE_STRING,
        PORT_TYPE_MESH,
        PORT_TYPE_ARRAY,
        PORT_TYPE_CURVE,
        PORT_TYPE_DATA,
        PORT_TYPE_HGEO,
        PORT_TYPE_MAX,
    };

    enum PortHint {
        PORT_HINT_ITEM, // Iterate over each item separately (e.g. point).
        PORT_HINT_LIST, // Iterate over the whole list as a single pack (e.g. curve composed by points).
        PORT_HINT_HGEO_POINT, // Iterate over each point from a HoodieGeo (returns a Hoodiegeo with only one point and its attributes).
        PORT_HINT_HGEO_PRIMITIVE, // Iterate over each primitive from a HoodieGeo
        PORT_HINT_HGEO_ALL, // Pass the whole HoodieGeo object
        PORT_HINT_OBJECT,
    };

    enum ProcessStatus {
		PENDING, // all nodes are cleared to this before we update our mesh
		INPROGRESS, // a node gets this status when we are in the middle of updating it, helps detect cyclic relationships
		UNCHANGED, // a node gets this status once we finish updating and find the node unchanged
		CHANGED, // a node gets this status once we finish updating at its contents has changed
    };

private:
    ProcessStatus status;
    bool dirty = true;

protected:
    void mark_dirty();

    // Store here the data outputs of the node.
    Array outputs;

protected:
    static void _bind_methods();

public:
    void set_status(const ProcessStatus &p_status);
    ProcessStatus get_status() const;
    
    virtual bool update(bool p_inputs_updated, const Array &p_inputs);

    virtual void _process(const Array &p_inputs);

    virtual String get_caption() const;

	virtual int get_input_port_count() const;
	virtual PortType get_input_port_type(int p_port) const;
    virtual PortHint get_input_port_hint(int p_port) const;
	virtual String get_input_port_name(int p_port) const;

    virtual int get_output_port_count() const;
	virtual PortType get_output_port_type(int p_port) const;
	virtual PortHint get_output_port_hint(int p_port) const;
	virtual String get_output_port_name(int p_port) const;

    // If you want to implement properties for a node, remember that:
    // * UI visualization is managed in HoodieNodePluginDefault::create_editor()
    // * signal connection is managed in HoodieNodePluginDefaultEditor::setup()
    // * undo redo is managed in HoodieNodePluginDefaultEditor::_property_changed()

    virtual int get_property_input_count() const;
    virtual Variant::Type get_property_input_type(vec_size_t p_prop) const;
    virtual String get_property_input_hint(vec_size_t p_prop) const;
    virtual Variant get_property_input(vec_size_t p_port) const;
    virtual void set_property_input(vec_size_t p_port, Variant p_input);

    virtual Vector<StringName> get_editable_properties() const;
    virtual HashMap<StringName, String> get_editable_properties_names() const;

    Array _package_inputs(const Array &p_inputs) const;

    const Variant get_output(int p_port) const;
    void set_output(int p_port, const Variant &p_data);

    bool is_output_port_connected(vec_size_t p_port) const;
	void set_output_port_connected(vec_size_t p_port, bool p_connected);
	bool is_input_port_connected(vec_size_t p_port) const;
	void set_input_port_connected(vec_size_t p_port, bool p_connected);

    HoodieNode();
};
    
} // namespace godot

#endif // HOODIE_HOODIENODE_H
