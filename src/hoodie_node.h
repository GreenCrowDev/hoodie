#ifndef HOODIE_HOODIENODE_H
#define HOODIE_HOODIENODE_H

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "hoodie_geo.h"

namespace godot
{

class HoodieNode : public Resource {
    GDCLASS(HoodieNode, Resource)

    friend class HoodieGraphPlugin;
    friend class HoodieEditorPlugin;
    friend class HoodieMesh;

protected:
    // typedef uint32_t id_t;
    typedef int id_t;
    typedef uint32_t vec_size_t;

private:
    HashMap<vec_size_t, bool> connected_input_ports;
    HashMap<vec_size_t, bool> connected_output_ports;

// TODO: delete this
/*
protected:
    struct Property {
        Variant::Type type;
        String hint;

        Property() {}
        Property(Variant::Type p_type, String p_hint) : type(p_type), hint(p_hint) {}
    };

    struct Socket {
        enum SocketIO { INPUT, OUTPUT };

        SocketIO socket_io;
        Variant::Type type;
        String name;
        String description;

        Socket() {}
        Socket(SocketIO p_socket_io, Variant::Type p_type, String p_name, String p_description)
            : socket_io(p_socket_io),
              type(p_type),
              name(p_name),
              description(p_description) {}
    };
*/

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

    enum ProcessStatus {
		PENDING, // all nodes are cleared to this before we update our mesh
		INPROGRESS, // a node gets this status when we are in the middle of updating it, helps detect cyclic relationships
		UNCHANGED, // a node gets this status once we finish updating and find the node unchanged
		CHANGED, // a node gets this status once we finish updating at its contents has changed
    };

private:
    // TODO: delete this
    // Property property;
    // Vector<Socket> input_sockets;
    // Vector<Socket> output_sockets;

    ProcessStatus status;
    bool dirty = true;

protected:
    void mark_dirty();

protected:
    static void _bind_methods();

public:
    void set_status(const ProcessStatus &p_status);
    ProcessStatus get_status() const;

    // Property stuff
    // void set_property(const Property &p_property);
    // Variant::Type get_property_type() const;
    // String get_property_hint() const;
    // virtual void construct_property();
    // virtual void set_input(Variant p_input);
    // virtual Variant get_input();

    // void add_socket(Socket &p_socket);
    // Vector<Socket> get_input_sockets() const;
    // Vector<Socket> get_output_sockets() const;

    // virtual void construct_sockets();
    
    virtual bool update(bool p_inputs_updated, const Array &p_inputs);

    virtual void _process(const Array &p_inputs);

    virtual String get_caption() const;

	virtual int get_input_port_count() const;
	virtual PortType get_input_port_type(int p_port) const;
	virtual String get_input_port_name(int p_port) const;

    virtual int get_output_port_count() const;
	virtual PortType get_output_port_type(int p_port) const;
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

    virtual const Variant get_output(int p_port) const;

    bool is_output_port_connected(vec_size_t p_port) const;
	void set_output_port_connected(vec_size_t p_port, bool p_connected);
	bool is_input_port_connected(vec_size_t p_port) const;
	void set_input_port_connected(vec_size_t p_port, bool p_connected);

    HoodieNode();
};
    
} // namespace godot

#endif // HOODIE_HOODIENODE_H
