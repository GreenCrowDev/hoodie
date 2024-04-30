#include "hoodie_editor_plugin.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
#include <godot_cpp/templates/hash_set.hpp>

using namespace godot;

void HoodieNodePlugin::set_editor(HoodieEditorPlugin *p_editor) {
    hmeditor = p_editor;
}

Control *HoodieNodePlugin::create_editor(const Ref<Resource> &p_parent_resource, const Ref<HoodieNode> &p_node) {
    Object *ret = nullptr;
    // TODO: GDVirtual_ca
    return Object::cast_to<Control>(ret);
}

void HoodieNodePlugin::_bind_methods() {
    // TODO: GDVIRTUAL_BIND
}

///////////////////

HoodieGraphPlugin::HoodieGraphPlugin() {
}

void HoodieGraphPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("update_node", "id"), &HoodieGraphPlugin::update_node);
    ClassDB::bind_method(D_METHOD("add_node", "id", "just_update"), &HoodieGraphPlugin::add_node);
    ClassDB::bind_method(D_METHOD("remove_node", "id", "just_update"), &HoodieGraphPlugin::remove_node);
    ClassDB::bind_method(D_METHOD("connect_nodes"), &HoodieGraphPlugin::connect_nodes);
    ClassDB::bind_method(D_METHOD("disconnect_nodes"), &HoodieGraphPlugin::disconnect_nodes);
    ClassDB::bind_method(D_METHOD("set_node_position", "id", "position"), &HoodieGraphPlugin::set_node_position);
}

void HoodieGraphPlugin::set_editor(HoodieEditorPlugin *p_editor) {
    editor = p_editor;
}

void HoodieGraphPlugin::register_hoodie_mesh(HoodieMesh *p_hoodie_mesh) {
    hoodie_mesh = Ref<HoodieMesh>(p_hoodie_mesh);
}

void HoodieGraphPlugin::set_connections(const List<HoodieMesh::Connection> &p_connections) {
    connections = p_connections;
}

void HoodieGraphPlugin::register_link(id_t p_id, HoodieNode *p_hoodie_node, GraphElement *p_graph_element) {
    links.insert(p_id, { p_hoodie_node, p_graph_element });
}

void HoodieGraphPlugin::clear_links() {
    links.clear();
}

void HoodieGraphPlugin::update_node(id_t p_id) {
    if (!links.has(p_id)) {
        return;
    }
    remove_node(p_id, true);
    add_node(p_id, true);
}

void HoodieGraphPlugin::update_node_deferred(id_t p_id) {
    call_deferred(StringName("update_node"), p_id);
}

void HoodieGraphPlugin::add_node(id_t p_id, bool p_just_update) {
    if (!hoodie_mesh.is_valid()) {
        return;
    }
    HoodieGraphPlugin *graph_plugin = editor->get_graph_plugin();
    if (!graph_plugin) {
        return;
    }

    // HoodieNode *hn = hoodie_node.ptr();
    Ref<HoodieNode> hoodie_node = hoodie_mesh->get_node(p_id);

	static const Color type_color[] = {
		Color(0.70, 0.70, 0.70), // scalar (float)
		Color(0.37, 0.62, 0.38), // scalar (int)
		Color(0.17, 0.66, 0.19), // scalar (uint)
		Color(0.46, 0.46, 1.00), // vector2
		Color(0.46, 0.46, 1.00), // vector3
		Color(0.46, 0.46, 1.00), // vector4
		Color(0.80, 0.65, 0.84), // boolean
		Color(0.78, 0.78, 0.16), // color
		Color(0.34, 0.65, 1.00), // string
		Color(0.00, 0.84, 0.64), // geometry
		Color(0.00, 0.00, 0.00), // array
		Color(0.00, 0.84, 0.64), // curve
		Color(1.0, 1.0, 1.0), // data
		Color(1.0, 1.0, 0.0), // hgeo
		Color(1.0, 1.0, 0.0), // max
	};

    GraphNode *graph_node = memnew(GraphNode);
    graph_node->set_title(String("[") + String::num_int64(p_id) + String("] ") + hoodie_node->get_caption());

    graph_node->connect("delete_request", callable_mp(editor, &HoodieEditorPlugin::_delete_node_request).bind(p_id), CONNECT_DEFERRED);

    if (p_just_update) {
        Link &link = links[p_id];

        link.graph_element = graph_node;
    } else {
        register_link(p_id, hoodie_node.ptr(), graph_node);
    }

    graph_node->set_resizable(false);
    graph_node->set_custom_minimum_size(Size2(200, 0));

    graph_node->set_position_offset(hoodie_mesh->get_node_position(p_id));

    graph_node->set_name(itos(p_id));

    graph_node->connect("dragged", callable_mp(editor, &HoodieEditorPlugin::_node_dragged).bind(p_id));

    graph_node->connect("node_selected", callable_mp(editor->hoodie_control, &HoodieControl::_node_selected).bind(p_id));
    graph_node->connect("node_deselected", callable_mp(editor->hoodie_control, &HoodieControl::_node_deselected).bind(p_id));

    // Adding Output and Input ports
    int j = 0;

    // Adding Output ports.
    for (int i = 0; i < hoodie_node->get_output_port_count(); i++)
    {
        HoodieNode::PortType port_output = hoodie_node->get_output_port_type(i);

        HBoxContainer *hb = memnew(HBoxContainer);
        Label *label = memnew(Label);
        label->set_text(hoodie_node->get_output_port_name(i));
        label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
        label->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
        label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        hb->add_child(label);
        graph_node->add_child(hb);

        int port_type = 0;
        graph_node->set_slot(i, false, port_type, type_color[0], true, port_type, type_color[port_output]);
        j++;
    }

    // Adding Input ports.
    for (int i = 0; i < hoodie_node->get_input_port_count(); i++)
    {
        HoodieNode::PortType port_input = hoodie_node->get_input_port_type(i);

        HBoxContainer *hb = memnew(HBoxContainer);
        Label *label = memnew(Label);
        label->set_text(hoodie_node->get_input_port_name(i));
        label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_LEFT);
        label->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
        label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        hb->add_child(label);
        graph_node->add_child(hb);

        int port_type = 0;
        graph_node->set_slot(j, true, port_type, type_color[port_input], false, port_type, type_color[0]);
        j++;
    }

    // Adding custom editors (e.g. Integer property)
    Control *custom_editor;
    int port_offset = 1;

    Control *content_offset = memnew(Control);
    content_offset->set_custom_minimum_size(Size2(0, 5));
    graph_node->add_child(content_offset);

    for (int i = 0; i < editor->plugins.size(); i++) {
        hoodie_node->set_meta("id", p_id);
        custom_editor = editor->plugins.write[i]->create_editor(hoodie_mesh, hoodie_node);
        hoodie_node->remove_meta("id");
        // if (custom_editor) {
        //     if (hoodie_node->is_show_prop_names())
        // }
    }

    if (custom_editor) {
        custom_editor->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        graph_node->add_child(custom_editor);
        // TODO: necessary if you need further use of custom_editor = nullptr;
    }

    // Adding Properties fields.
    VBoxContainer *props_vb = memnew(VBoxContainer);
    int slider_count = 0;
    for (int i = 0; i < hoodie_node->get_property_input_count(); i++) {
        switch (hoodie_node->get_property_input_type(i)) {
            case Variant::OBJECT:
                {
                    if (hoodie_node->get_class() == "HNInputCurve3D") {
                        Label *curve_lab = memnew(Label);
                        props_vb->add_child(curve_lab);
                        curve_lab->set_custom_minimum_size(Size2(65, 0));
                        curve_lab->set_h_size_flags(Control::SIZE_EXPAND_FILL);
                        curve_lab->set_text("Select Curve3D form the Inspector.");
                    }
                }
                break;
        }
    }
    graph_node->add_child(props_vb);

    editor->hoodie_control->graph_edit->add_child(graph_node);
}

void HoodieGraphPlugin::remove_node(id_t p_id, bool p_just_update) {
    if (links.has(p_id)) {
        Node *graph_edit_node = links[p_id].graph_element->get_parent();
        graph_edit_node->remove_child(links[p_id].graph_element);
        memdelete(links[p_id].graph_element);
        if (!p_just_update) {
            links.erase(p_id);
        }
    }
}

void HoodieGraphPlugin::connect_nodes(id_t p_l_node, vec_size_t p_l_port, id_t p_r_node, vec_size_t p_r_port) {
    GraphEdit *graph = editor->hoodie_control->graph_edit;
    if (!graph) {
        return;
    }

    if (hoodie_mesh.is_valid()) {
        graph->connect_node(itos(p_l_node), p_l_port, itos(p_r_node), p_r_port);
        connections.push_back({ p_l_node, p_l_port, p_r_node, p_r_port });
    }
}

void HoodieGraphPlugin::disconnect_nodes(id_t p_l_node, vec_size_t p_l_port, id_t p_r_node, vec_size_t p_r_port) {
    GraphEdit *graph = editor->hoodie_control->graph_edit;
    if (!graph) {
        return;
    }

    if (hoodie_mesh.is_valid()) {
        graph->disconnect_node(itos(p_l_node), p_l_port, itos(p_r_node), p_r_port);

        for (const List<HoodieMesh::Connection>::Element *E = connections.front(); E; E = E->next()) {
            if (E->get().l_node == p_l_node && E->get().l_port == p_l_port && E->get().r_node == p_r_node && E->get().r_port == p_r_port) {
                connections.erase(E);
                break;
            }
        }
    }
}

void HoodieGraphPlugin::set_node_position(id_t p_id, const Vector2 &p_position) {
    if (links.has(p_id)) {
        links[p_id].graph_element->set_position_offset(p_position);
    }
}

void HoodieGraphPlugin::_on_range_value_changed(double p_val, id_t p_id, vec_size_t p_prop_id) {
    Link &link = links[p_id];

    // link.hoodie_node->set_property_input(p_prop_id, Variant(p_val));
    // link.hoodie_node->mark_dirty();
    // editor->hoodie_mesh->_queue_update();

    editor->_change_node_property(p_id, p_prop_id, Variant(p_val));
}

void HoodieGraphPlugin::_on_vector_range_value_changed(double p_val, id_t p_id, vec_size_t p_prop_id, int p_xyzw) {
    Link &link = links[p_id];

    TypedArray<double> arr;
    Variant::Type t = link.hoodie_node->get_property_input_type(p_prop_id);
    Variant vector = link.hoodie_node->get_property_input(p_prop_id);
    switch (t) {
        case Variant::VECTOR2:
        {
            arr.push_back(((Vector2)vector).x);
            arr.push_back(((Vector2)vector).y);
        }
        break;
        case Variant::VECTOR3:
        {
            arr.push_back(((Vector3)vector).x);
            arr.push_back(((Vector3)vector).y);
            arr.push_back(((Vector3)vector).z);
        }
        break;
        case Variant::VECTOR4:
        {
            arr.push_back(((Vector4)vector).x);
            arr.push_back(((Vector4)vector).y);
            arr.push_back(((Vector4)vector).z);
            arr.push_back(((Vector4)vector).w);
        }
        break;
    }

    for (int i = 0; i < arr.size(); i++) {
        if (i == p_xyzw) {
            arr[i] = p_val;
        }
    }

    if (arr.size() == 2) {
        Vector2 v = Vector2(arr[0], arr[1]);
        link.hoodie_node->set_property_input(p_prop_id, Variant(v));
    } else if (arr.size() == 3) {
        Vector3 v = Vector3(arr[0], arr[1], arr[2]);
        link.hoodie_node->set_property_input(p_prop_id, Variant(v));
    } else if (arr.size() == 4) {
        Vector4 v = Vector4(arr[0], arr[1], arr[2], arr[3]);
        link.hoodie_node->set_property_input(p_prop_id, Variant(v));
    }

    link.hoodie_node->mark_dirty();
    editor->hoodie_mesh->_queue_update();
}

HoodieGraphPlugin::~HoodieGraphPlugin() {
}

///////////////////

Vector2 HoodieControl::selection_center;
List<HoodieControl::CopyItem> HoodieControl::copy_items_buffer;
List<HoodieMesh::Connection> HoodieControl::copy_connections_buffer;

void HoodieControl::_bind_methods() {
}

void HoodieControl::_notification(int p_what) {
    switch (p_what)
    {
        case NOTIFICATION_READY: {
            graph_edit->connect("connection_request", callable_mp(editor, &HoodieEditorPlugin::_connection_request), CONNECT_DEFERRED);
            graph_edit->connect("disconnection_request", callable_mp(editor, &HoodieEditorPlugin::_disconnection_request), CONNECT_DEFERRED);
            graph_edit->connect("scroll_offset_changed", callable_mp(editor, &HoodieEditorPlugin::_scroll_changed));
            graph_edit->connect("popup_request", callable_mp(this, &HoodieControl::_on_popup_request));
            graph_edit->connect("duplicate_nodes_request", callable_mp(this, &HoodieControl::_duplicate_nodes));
            graph_edit->connect("copy_nodes_request", callable_mp(this, &HoodieControl::_copy_nodes).bind(false));
            graph_edit->connect("paste_nodes_request", callable_mp(this, &HoodieControl::_paste_nodes).bind(false, Point2()));
            graph_edit->connect("delete_nodes_request", callable_mp(editor, &HoodieEditorPlugin::_delete_nodes_request));

            file_menu->get_popup()->connect("id_pressed", callable_mp(this, &HoodieControl::_menu_item_pressed));
            options_menu->get_popup()->connect("id_pressed", callable_mp(this, &HoodieControl::_menu_item_pressed));
            lock_button->connect("toggled", callable_mp(this, &HoodieControl::_on_lock_toggled));

            // Popup with search filter.
            node_filter->connect("text_changed", callable_mp(this, &HoodieControl::_member_filter_changed));
            node_filter->connect("gui_input", callable_mp(this, &HoodieControl::_sbox_input));
            members->connect("item_activated", callable_mp(this, &HoodieControl::_member_create));
            members->connect("item_selected", callable_mp(this, &HoodieControl::_member_selected));
            members->connect("nothing_selected", callable_mp(this, &HoodieControl::_member_unselected));
	        members_dialog->connect("popup_hide", callable_mp(this, &HoodieControl::_member_cancel));
        } break;
        case NOTIFICATION_THEME_CHANGED: {
            lock_button->set_button_icon(get_theme_icon("Lock", "EditorIcons"));
        } break;
    }
}

void HoodieControl::set_editor(HoodieEditorPlugin *p_editor) {
    editor = p_editor;
}

HoodieControl::HoodieControl() {
    main_split = memnew(HSplitContainer);
    main_split->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    main_split->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    add_child(main_split);

    VBoxContainer *vb = memnew(VBoxContainer);
    HBoxContainer *menu_hb = memnew(HBoxContainer);
    vb->add_child(menu_hb);

    file_menu = memnew(MenuButton);
    file_menu->set_text("File");
    file_menu->set_shortcut_context(main_split);
    file_menu->get_popup()->add_item("New", FILE_NEW);
    file_menu->get_popup()->add_item("Print debug", FILE_PRINTDEBUG);
    menu_hb->add_child(file_menu);

    options_menu = memnew(MenuButton);
    options_menu->set_text("Options");
    options_menu->set_shortcut_context(main_split);
    options_menu->get_popup()->add_check_item("Verbose mode", OPTIONS_VERBOSE);
    options_menu->get_popup()->set_item_checked(options_menu->get_popup()->get_item_index(OPTIONS_VERBOSE), verbose_mode);
    menu_hb->add_child(options_menu);

    lock_button = memnew(Button);
    // So that the button will be located to the right of the menu_hb.
    lock_button->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_END);
    lock_button->set_theme_type_variation("FlatButton");
    lock_button->set_button_icon(main_split->get_theme_icon("New", "EditorIcons"));
    lock_button->set_toggle_mode(true);
    lock_button->set_tooltip_text("Lock Hoodie Inspector");
    menu_hb->add_child(lock_button);

    // TabContainer for debug purposes.
    hn_inspector = memnew(TabContainer);
    hn_inspector->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    vb->add_child(hn_inspector);

    main_split->add_child(vb);
    vb->set_custom_minimum_size(Size2(200, 300));

    graph_edit = memnew(GraphEdit);
    main_split->add_child(graph_edit);
    graph_edit->set_right_disconnects(true);

    add_node = memnew(MenuButton);
    add_node->set_text("Add Node...");
    graph_edit->get_menu_hbox()->add_child(add_node);
    graph_edit->get_menu_hbox()->move_child(add_node, 0);

    add_popup = add_node->get_popup();

    // Add Node with LineEdit filter.

	VBoxContainer *members_vb = memnew(VBoxContainer);
	members_vb->set_v_size_flags(SIZE_EXPAND_FILL);

	HBoxContainer *filter_hb = memnew(HBoxContainer);
	members_vb->add_child(filter_hb);

	node_filter = memnew(LineEdit);
	filter_hb->add_child(node_filter);

	node_filter->set_h_size_flags(SIZE_EXPAND_FILL);
	node_filter->set_placeholder("Search");

    members = memnew(Tree);
	members_vb->add_child(members);
	// TODO: implement this?
    // SET_DRAG_FORWARDING_GCD(members, HoodieControl);
	members->set_h_size_flags(SIZE_EXPAND_FILL);
	members->set_v_size_flags(SIZE_EXPAND_FILL);
    members->set_focus_mode(FocusMode::FOCUS_NONE);
	members->set_hide_root(true);
	members->set_allow_reselect(true);
    members->set_allow_search(false);
	members->set_hide_folding(false);
	members->set_custom_minimum_size(Size2(180, 200));

	members_dialog = memnew(Popup);
	members_dialog->set_title("Create Hoodie Node");
	members_dialog->add_child(members_vb);
	add_child(members_dialog);

	///////////////////////////////////////
	// HOODIE NODES TREE OPTIONS
	///////////////////////////////////////

    // INPUT

    add_options.push_back(AddOption("Input Value", "Input/Constant", "HNInputValue"));
    add_options.push_back(AddOption("Input Integer", "Input/Constant", "HNInputInteger"));
    add_options.push_back(AddOption("Input Vector3D", "Input/Constant", "HNInputVector3D"));

    add_options.push_back(AddOption("Input Curve3D", "Input", "HNInputCurve3D"));

    // GEOMETRY

    add_options.push_back(AddOption("Transform Geometry", "Geometry/Operations", "HNTransformGeometry"));

    // CURVE

    add_options.push_back(AddOption("Points Curvature", "Curve/Analysis", "HNPointsCurvature"));
    add_options.push_back(AddOption("Points Distance", "Curve/Analysis", "HNPointsDistance"));

    add_options.push_back(AddOption("Connect Points", "Curve/Operations", "HNConnectPoints"));
    add_options.push_back(AddOption("Curve to Mesh", "Curve/Operations", "HNCurveToMesh"));
    add_options.push_back(AddOption("Curve to Points", "Curve/Operations", "HNCurveToPoints"));

    // MESH

    add_options.push_back(AddOption("Cube", "Mesh/Primitives", "HNMeshCube"));
    add_options.push_back(AddOption("Mesh Circle", "Mesh/Primitives", "HNMeshCircle"));
    add_options.push_back(AddOption("Mesh Grid", "Mesh/Primitives", "HNMeshGrid"));
    add_options.push_back(AddOption("Mesh Line", "Mesh/Primitives", "HNMeshLine"));
    add_options.push_back(AddOption("Mesh Rect", "Mesh/Primitives", "HNMeshRect"));

    // UTILITIES
  
    add_options.push_back(AddOption("Repeat Data", "Utilities/Data", "HNRepeatData"));
    add_options.push_back(AddOption("Shift Data", "Utilities/Data", "HNShiftData"));

    add_options.push_back(AddOption("Compose Hoodie Geo", "Utilities/Hoodie Geo", "HNComposeHoodieGeo"));

    add_options.push_back(AddOption("Derivative", "Utilities/Math", "HNMathDerivative"));
    add_options.push_back(AddOption("Less Than", "Utilities/Math", "HNMathLessThan"));
    add_options.push_back(AddOption("Multiply", "Utilities/Math", "HNMathMultiply"));
    add_options.push_back(AddOption("Noise Reduction", "Utilities/Math", "HNMathNoiseReduction"));
    add_options.push_back(AddOption("Sign", "Utilities/Math", "HNMathSign"));

    add_options.push_back(AddOption("Combine XYZ", "Utilities/Vector", "HNCombineXYZ"));
    add_options.push_back(AddOption("Separate XYZ", "Utilities/Vector", "HNSeparateXYZ"));
    add_options.push_back(AddOption("Combine XY", "Utilities/Vector", "HNCombineXY"));
    add_options.push_back(AddOption("Separate XY", "Utilities/Vector", "HNSeparateXY"));
    add_options.push_back(AddOption("Vector Cross", "Utilities/Vector", "HNVectorCross"));
    add_options.push_back(AddOption("Vector Multiply", "Utilities/Vector", "HNVectorMultiply"));
    add_options.push_back(AddOption("Vector Sum", "Utilities/Vector", "HNVectorSum"));

    add_options.push_back(AddOption("Compose Mesh", "Utilities/Mesh", "HNComposeMesh"));
    add_options.push_back(AddOption("Decompose Mesh", "Utilities/Mesh", "HNDecomposeMesh"));

    add_options.push_back(AddOption("Align Euler to Vector", "Utilities/Rotation", "HNAlignEulerToVector"));

    // OUTPUT

    add_options.push_back(AddOption("Output", "Output", "HNOutput"));

    ///////////////////////////////////////
}

void HoodieControl::_on_popup_request(const Vector2 &p_position) {
    editor->saved_node_pos = p_position;
    editor->saved_node_pos_dirty = true;

    _update_options_menu();
    members_dialog->popup_on_parent(Rect2(graph_edit->get_global_position() + p_position, Vector2(1, 1)));
    members_dialog->grab_focus();
    node_filter->call_deferred(StringName("grab_focus")); // Still not visible.
    node_filter->select_all();
}

void HoodieControl::_menu_item_pressed(int index) {
    switch (index) {
        case FILE_NEW: {
            UtilityFunctions::push_warning("FILE_NEW not implemented.");
        } break;
        case FILE_PRINTDEBUG: {
            editor->print_debug();
        } break;
        case OPTIONS_VERBOSE: {
            verbose_mode = !verbose_mode;
            options_menu->get_popup()->set_item_checked(options_menu->get_popup()->get_item_index(OPTIONS_VERBOSE), verbose_mode);
            editor->hoodie_mesh->set_verbose_mode(verbose_mode);
        } break;
    }
}

void HoodieControl::_on_lock_toggled(bool toggled_on) {
    lock_inspector = toggled_on;
}

void HoodieControl::_update_options_menu() {
    add_popup->clear(true);
    members->clear();

    TreeItem *root = members->create_item();

    String filter = node_filter->get_text().strip_edges();
    bool use_filter = !filter.is_empty();

    bool is_first_item = true;

	HashMap<String, TreeItem *> folders;

	Vector<AddOption> embedded_options;

    for (int i = 0; i < add_options.size(); i++) {
        // E.g. Mesh/Primitive/Grid
        String path = add_options[i].category + String("/") + add_options[i].name;
        PackedStringArray subfolders = path.split("/");

        // Normal popup.
        PopupMenu *popup = add_popup;
        for (int j = 0; j < subfolders.size() - 1; j++) {
            String j_name = subfolders[j];
            if (!popup->has_node(j_name)) {
                PopupMenu *new_popup = memnew(PopupMenu);
                new_popup->set_name(j_name);
                new_popup->connect("id_pressed", callable_mp(editor, &HoodieEditorPlugin::_add_node));
                popup->add_child(new_popup);
                popup->add_submenu_item(j_name, j_name);
            }
            popup = popup->get_node<PopupMenu>(j_name);
        }
        popup->add_item(subfolders[subfolders.size() - 1], i);

        // Popup with search filter.
        if (!use_filter || add_options[i].name.findn(filter) != -1) {
            const_cast<AddOption &>(add_options[i]).temp_idx = i; // save valid id
            embedded_options.push_back(add_options[i]);
        }
    }

    Vector<AddOption> options;
    options.append_array(embedded_options);

    for (int i = 0; i < options.size(); i++) {
        String path = options[i].category;
        PackedStringArray subfolders = path.split("/");
        TreeItem *category = nullptr;

        if (!folders.has(path)) {
            category = root;
            String path_temp = "";
            for (int j = 0; j < subfolders.size(); j++) {
                path_temp += subfolders[j];
                if (!folders.has(path_temp)) {
					category = members->create_item(category);
					category->set_selectable(0, false);
					category->set_collapsed(!use_filter);
					category->set_text(0, subfolders[j]);
					folders.insert(path_temp, category);
				} else {
                    category = folders[path_temp];
                }
            }
        } else {
            category = folders[path];
        }

        TreeItem *item = members->create_item(category);

		item->set_text(0, options[i].name);
		if (is_first_item && use_filter) {
			item->select(0);
			is_first_item = false;
		}

		item->set_meta("id", options[i].temp_idx);
    }
}

void HoodieControl::_node_selected(id_t p_node) {
    _populate_hoodie_node_tab_inspector(p_node);
}

void HoodieControl::_node_deselected(id_t p_node) {
    _depopulate_hoodie_node_tab_inspector();
}

void HoodieControl::_dup_copy_nodes(List<CopyItem> &r_items, List<HoodieMesh::Connection> &r_connections) {
	selection_center.x = 0.0f;
	selection_center.y = 0.0f;

	HashSet<int> nodes;

	for (int i = 0; i < graph_edit->get_child_count(); i++) {
		GraphElement *graph_element = Object::cast_to<GraphElement>(graph_edit->get_child(i));
		if (graph_element) {
			int id = String(graph_element->get_name()).to_int();

			Ref<HoodieNode> node = editor->hoodie_mesh->get_node(id);

			if (node.is_valid() && graph_element->is_selected()) {
				Vector2 pos = editor->hoodie_mesh->get_node_position(id);
				selection_center += pos;

				CopyItem item;
				item.id = id;
				item.node = editor->hoodie_mesh->get_node(id)->duplicate();
				item.position = editor->hoodie_mesh->get_node_position(id);

				r_items.push_back(item);

				nodes.insert(id);
			}
		}
	}

	List<HoodieMesh::Connection> node_connections;
	editor->hoodie_mesh->get_node_connections(&node_connections);

	for (const HoodieMesh::Connection &E : node_connections) {
		if (nodes.has(E.l_node) && nodes.has(E.r_node)) {
			r_connections.push_back(E);
		}
	}

	selection_center /= (float)r_items.size();
}

void HoodieControl::_dup_paste_nodes(List<CopyItem> &r_items, const List<HoodieMesh::Connection> &p_connections, const Vector2 &p_offset, bool p_duplicate) {
	EditorUndoRedoManager *undo_redo = editor->get_undo_redo();
	if (p_duplicate) {
		undo_redo->create_action("Duplicate HoodieNode(s)");
	} else {
		bool copy_buffer_empty = true;
		for (const CopyItem &item : copy_items_buffer) {
			if (!item.disabled) {
				copy_buffer_empty = false;
				break;
			}
		}
		if (copy_buffer_empty) {
			return;
		}

		undo_redo->create_action("Paste HoodieNode(s)");
	}

	int base_id = editor->hoodie_mesh->get_valid_node_id();
	int id_from = base_id;
	HashMap<int, int> connection_remap;
	HashSet<int> unsupported_set;
	HashSet<int> added_set;

	for (CopyItem &item : r_items) {
		if (item.disabled) {
			unsupported_set.insert(item.id);
			continue;
		}
		connection_remap[item.id] = id_from;
		Ref<HoodieNode> node = item.node->duplicate();

		undo_redo->add_do_method(editor->hoodie_mesh.ptr(), "add_node", node, item.position + p_offset, id_from);
		undo_redo->add_do_method(editor->graph_plugin.ptr(), "add_node", id_from, false);

		added_set.insert(id_from);
		id_from++;
	}

	for (const HoodieMesh::Connection &E : p_connections) {
		if (unsupported_set.has(E.l_node) || unsupported_set.has(E.r_node)) {
			continue;
		}

		undo_redo->add_do_method(editor->hoodie_mesh.ptr(), "connect_nodes", connection_remap[E.l_node], E.l_port, connection_remap[E.r_node], E.r_port);
		undo_redo->add_do_method(editor->graph_plugin.ptr(), "connect_nodes", connection_remap[E.l_node], E.l_port, connection_remap[E.r_node], E.r_port);
		// TODO: uncomment this? undo_redo->add_undo_method(editor->hoodie_mesh.ptr(), "disconnect_nodes", connection_remap[E.l_node], E.l_port, connection_remap[E.r_node], E.r_port);
        undo_redo->add_undo_method(editor->graph_plugin.ptr(), "disconnect_nodes", connection_remap[E.l_node], E.l_port, connection_remap[E.r_node], E.r_port);
	}

	id_from = base_id;
	for (const CopyItem &item : r_items) {
		if (item.disabled) {
			continue;
		}
		undo_redo->add_undo_method(editor->hoodie_mesh.ptr(), "remove_node", id_from);
		undo_redo->add_undo_method(editor->graph_plugin.ptr(), "remove_node", id_from, false);
		id_from++;
	}

	undo_redo->commit_action();

	// reselect nodes by excluding the other ones
	for (int i = 0; i < graph_edit->get_child_count(); i++) {
		GraphElement *graph_element = Object::cast_to<GraphElement>(graph_edit->get_child(i));
		if (graph_element) {
			int id = String(graph_element->get_name()).to_int();
			if (added_set.has(id)) {
				graph_element->set_selected(true);
			} else {
				graph_element->set_selected(false);
			}
		}
	}
}

void HoodieControl::_duplicate_nodes() {
	List<CopyItem> items;
	List<HoodieMesh::Connection> node_connections;

	_dup_copy_nodes(items, node_connections);

	if (items.is_empty()) {
		return;
	}

	_dup_paste_nodes(items, node_connections, Vector2(10, 10), true);
}

void HoodieControl::_clear_copy_buffer() {
	copy_items_buffer.clear();
	copy_connections_buffer.clear();
}

void HoodieControl::_copy_nodes(bool p_cut) {
	_clear_copy_buffer();

	_dup_copy_nodes(copy_items_buffer, copy_connections_buffer);

	if (p_cut) {
		EditorUndoRedoManager *undo_redo = editor->get_undo_redo();
		undo_redo->create_action("Cut HoodieNode(s)");

		List<int> ids;
		for (const CopyItem &E : copy_items_buffer) {
			ids.push_back(E.id);
		}

		editor->_delete_nodes(ids);

		undo_redo->commit_action();
	}
}

void HoodieControl::_paste_nodes(bool p_use_custom_position, const Vector2 &p_custom_position) {
	if (copy_items_buffer.is_empty()) {
		return;
	}

	float scale = graph_edit->get_zoom();

	Vector2 mpos;
	if (p_use_custom_position) {
		mpos = p_custom_position;
	} else {
		mpos = graph_edit->get_local_mouse_position();
	}

	_dup_paste_nodes(copy_items_buffer, copy_connections_buffer, graph_edit->get_scroll_offset() / scale + mpos / scale - selection_center, false);
}

void HoodieControl::_populate_hoodie_node_tab_inspector(id_t p_node) {
    if (lock_inspector) {
        return;
    }

    // Show node data for debug purposes in the custom inspector panel
    Ref<HoodieNode> node = editor->hoodie_mesh->get_node(p_node);

    bool is_final_output = false;
    Array mesh_output;
    int count;

    if (node->get_output_port_count() == 0) {
        is_final_output = true;
        mesh_output = node->get_output(0);
        count  = 13;
    } else {
        count = node->get_output_port_count();
    }

    for (int i = 0; i < count; i++) {
        ScrollContainer *scroll_container = memnew(ScrollContainer);
        scroll_container->set_name(node->get_output_port_name(i));
        scroll_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        hn_inspector->add_child(scroll_container);

        Array output_data;

        if (is_final_output && mesh_output.size() > 0) {
            output_data = mesh_output[i];
        } else {
            output_data = node->get_output(i);
        }

        String data_string = "";

        for (int j = 0; j < output_data.size(); j++) {
            String j_data_string = "[" + itos(j) + "] ";

            switch (output_data[j].get_type()) {
                case Variant::Type::INT:
                    {
                        int val = (int)output_data[j];
                        j_data_string += itos(val);
                    }
                    break;
                case Variant::Type::FLOAT:
                    {
                        float f = (float)output_data[j];
                        j_data_string += String::num(f, 3);
                    }
                    break;
                case Variant::Type::VECTOR4:
                    {
                        Vector4 vec = output_data[j];
                        String vec_string = "( ";
                        vec_string += String::num(vec.x, 3) + " , ";
                        vec_string += String::num(vec.y, 3) + " , ";
                        vec_string += String::num(vec.z, 3) + " , ";
                        vec_string += String::num(vec.w, 3) + " )";
                        j_data_string += vec_string;
                    }
                    break;
                case Variant::Type::VECTOR3:
                    {
                        Vector3 vec = output_data[j];
                        String vec_string = "( ";
                        vec_string += String::num(vec.x, 3) + " , ";
                        vec_string += String::num(vec.y, 3) + " , ";
                        vec_string += String::num(vec.z, 3) + " )";
                        j_data_string += vec_string;
                    }
                    break;
                case Variant::Type::VECTOR2:
                    {
                        Vector2 vec = output_data[j];
                        String vec_string = "( ";
                        vec_string += String::num(vec.x, 3) + " , ";
                        vec_string += String::num(vec.y, 3) + " )";
                        j_data_string += vec_string;
                    }
                    break;
            }

            data_string += j_data_string + "\n";
        }

        Label *data_label = memnew(Label);
        data_label->set_text(data_string);
        scroll_container->add_child(data_label);
    }
}

void HoodieControl::_depopulate_hoodie_node_tab_inspector() {
    if (lock_inspector) {
        return;
    }

    // Remove inspector panel debug data nodes
    Vector<Node*> children;

    for (int i = 0; i < hn_inspector->get_child_count(); i++) {
        children.push_back(hn_inspector->get_child(i));
    }

    for (int i = 0; i < children.size(); i++) {
        memdelete(children[i]);
    }
}

void HoodieControl::_member_filter_changed(const String &p_text) {
	_update_options_menu();
}

// This is a dumbed down re-implementation of `Tree::_up` because this stuff is not exposed...
static void select_up(Tree &tree) {
	TreeItem *selected_item = tree.get_selected();

	if (selected_item == nullptr) {
		// UtilityFunctions::print("No item selected in tree, can't select down");
		return;
	}

	TreeItem *prev = selected_item->get_prev_visible();

	const int col = 0;
	while (prev != nullptr && !prev->is_selectable(col)) {
		prev = prev->get_prev_visible();
	}
	if (prev == nullptr) {
		return;
	}

	prev->select(col);

	tree.ensure_cursor_is_visible();
	// tree.accept_event();
}

// This is a dumbed down re-implementation of `Tree::_down` because this stuff is not exposed...
static void select_down(Tree &tree) {
	TreeItem *selected_item = tree.get_selected();

	if (selected_item == nullptr) {
		// UtilityFunctions::print("No item selected in tree, can't select down");
		return;
	}

	TreeItem *next = selected_item->get_next_visible();

	const int col = 0;

	while (next != nullptr && !next->is_selectable(col)) {
		next = next->get_next_visible();
	}
	if (next == nullptr) {
		return;
	}

	next->select(col);

	tree.ensure_cursor_is_visible();
	// tree.accept_event();
}

void HoodieControl::_sbox_input(const Ref<InputEvent> &p_ie) {
	Ref<InputEventKey> ie = p_ie;
	if (ie.is_valid() && (ie->get_keycode() == Key::KEY_UP || ie->get_keycode() == Key::KEY_DOWN || ie->get_keycode() == Key::KEY_ENTER || ie->get_keycode() == Key::KEY_KP_ENTER)) {
        // Zylann godot_voxel voxel_graph_node_dialog.cpp _on_filter_gui_input()
        if (ie->is_pressed()) {
            switch (ie->get_keycode()) {
                case Key::KEY_UP:
                    select_up(*members);
                    node_filter->accept_event();
                    break;
                case Key::KEY_DOWN:
                    select_down(*members);
                    node_filter->accept_event();
                    break;
                case Key::KEY_ENTER:
                    _member_create();
                    break;
                default:
                    break;
            }
        }
	}
}

void HoodieControl::_member_selected() {
	TreeItem *item = members->get_selected();

	if (item != nullptr && item->has_meta("id")) {
		// TODO: Implement custom behaviour
	} else {
		// TODO: Implement custom behaviour
	}
}

void HoodieControl::_member_unselected() {
}

void HoodieControl::_member_create() {
	TreeItem *item = members->get_selected();
	if (item != nullptr && item->has_meta("id")) {
		int idx = members->get_selected()->get_meta("id");
		// editor->_add_node(idx, add_options[idx].ops);
		editor->_add_node(idx);
	}
    members_dialog->hide();
}

void HoodieControl::_member_cancel() {
    node_filter->clear();

    // TODO: implement new node on drag connection into void graph
	// to_node = -1;
	// to_slot = -1;
	// from_node = -1;
	// from_slot = -1;
}

///////////////////

void HoodieEditorPlugin::_update_nodes() {
    // TODO: implement _update_nodes()
}

void HoodieEditorPlugin::_update_graph() {
    // TODO: implement _update_graph()
    if (updating) {
        return;
    }

    if (hoodie_mesh.is_null()) {
        return;
    }

    GraphEdit *graph_edit = hoodie_control->graph_edit;

    graph_edit->set_scroll_offset(hoodie_mesh->get_graph_offset());

    graph_edit->clear_connections();
    // Remove all nodes.
    for (int i = 0; i < graph_edit->get_child_count(); i++) {
        if (Object::cast_to<GraphElement>(graph_edit->get_child(i))) {
            Node *node = graph_edit->get_child(i);
            graph_edit->remove_child(node);
            memdelete(node);
            i--;
        }
    }

    List<HoodieMesh::Connection> node_connections;
    hoodie_mesh->get_node_connections(&node_connections);
    graph_plugin->set_connections(node_connections);

    Vector<id_t> nodes = hoodie_mesh->get_nodes_id_list();

    graph_plugin->clear_links();
    
    for (int n_i = 0; n_i < nodes.size(); n_i++) {
        graph_plugin->add_node(nodes[n_i], false);
    }

    for (const HoodieMesh::Connection &E : node_connections) {
        id_t l_node = E.l_node;
        vec_size_t l_port = E.l_port;
        id_t r_node = E.r_node;
        vec_size_t r_port = E.r_port;

        graph_edit->connect_node(itos(l_node), l_port, itos(r_node), r_port);
    }
}

void HoodieEditorPlugin::_add_node(int idx) {
    // TODO: godot source code visual_shader_editor_plugin.cpp _add_node()
    GraphEdit *graph_edit = hoodie_control->graph_edit;

    Point2 position = graph_edit->get_scroll_offset();

    if (saved_node_pos_dirty) {
        position += saved_node_pos;
    } else {
        position += graph_edit->get_size() * 0.5;
    }
    position /= graph_edit->get_zoom();
    saved_node_pos_dirty = false;

    Ref<HoodieNode> hnode;
    Variant v = ClassDB::instantiate(StringName(hoodie_control->add_options[idx].type));
    HoodieNode *hn = Object::cast_to<HoodieNode>(v);
    hnode = Ref<HoodieNode>(hn);
    id_t valid_id = hoodie_mesh->get_valid_node_id();

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("Add Node to Hoodie Mesh");
    undo_redo->add_do_method(hoodie_mesh.ptr(), "add_node", hnode, position, valid_id);
    undo_redo->add_undo_method(hoodie_mesh.ptr(), "remove_node", valid_id);
    undo_redo->add_do_method(graph_plugin.ptr(), "add_node", valid_id, false);
    undo_redo->add_undo_method(graph_plugin.ptr(), "remove_node", valid_id, false);
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_node_dragged(const Vector2 &p_from, const Vector2 &p_to, id_t p_node) {
    drag_buffer.push_back({ p_node, p_from, p_to });
    if (!drag_dirty) {
        call_deferred(StringName("_nodes_dragged"));
    }
    drag_dirty = true;
}

void HoodieEditorPlugin::_nodes_dragged() {
    drag_dirty = false;

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("Node(s) Moved");

    for (const DragOp &E : drag_buffer) {
        undo_redo->add_do_method(hoodie_mesh.ptr(), "set_node_position", E.node, E.to);
        undo_redo->add_undo_method(hoodie_mesh.ptr(), "set_node_position", E.node, E.from);
        undo_redo->add_do_method(graph_plugin.ptr(), "set_node_position", E.node, E.to);
        undo_redo->add_undo_method(graph_plugin.ptr(), "set_node_position", E.node, E.from);
    }

    drag_buffer.clear();
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_connection_request(const String &p_from, int p_from_index, const String &p_to, int p_to_index) {
    id_t l_node = p_from.to_int();
    vec_size_t l_port = p_from_index;
    id_t r_node = p_to.to_int();
    vec_size_t r_port = p_to_index;

    if (!hoodie_mesh->can_connect_nodes(l_node, l_port, r_node, r_port)) {
        return;
    }

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("HoodieNode(s) Connected");

    List<HoodieMesh::Connection> conns;
    hoodie_mesh->get_node_connections(&conns);

    for (const HoodieMesh::Connection &E : conns) {
        if (E.r_node == r_node && E.r_port == r_port) {
            undo_redo->add_do_method(hoodie_mesh.ptr(), "disconnect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
            undo_redo->add_undo_method(hoodie_mesh.ptr(), "connect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
            undo_redo->add_do_method(graph_plugin.ptr(), "disconnect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
            undo_redo->add_undo_method(graph_plugin.ptr(), "connect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
        }
    }

    undo_redo->add_do_method(hoodie_mesh.ptr(), "connect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_undo_method(hoodie_mesh.ptr(), "disconnect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_do_method(graph_plugin.ptr(), "connect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_undo_method(graph_plugin.ptr(), "disconnect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_do_method(graph_plugin.ptr(), "update_node", r_node);
    undo_redo->add_undo_method(graph_plugin.ptr(), "update_node", r_node);
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_disconnection_request(const String &p_from, int p_from_index, const String &p_to, int p_to_index) {
    hoodie_control->graph_edit->disconnect_node(p_from, p_from_index, p_to, p_to_index);

    id_t l_node = p_from.to_int();
    vec_size_t l_port = p_from_index;
    id_t r_node = p_to.to_int();
    vec_size_t r_port = p_to_index;

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("HoodieNode(s) Disconnected");
    undo_redo->add_do_method(hoodie_mesh.ptr(), "disconnect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_undo_method(hoodie_mesh.ptr(), "connect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_do_method(graph_plugin.ptr(), "disconnect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_undo_method(graph_plugin.ptr(), "connect_nodes", l_node, l_port, r_node, r_port);
    undo_redo->add_do_method(graph_plugin.ptr(), "update_node", r_node);
    undo_redo->add_undo_method(graph_plugin.ptr(), "update_node", r_node);
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_delete_nodes(const List<id_t> &p_nodes) {
    List<HoodieMesh::Connection> conns;
    hoodie_mesh->get_node_connections(&conns);

    EditorUndoRedoManager *undo_redo = get_undo_redo();

    for (const id_t &F : p_nodes) {
        Ref<HoodieNode> node = hoodie_mesh->get_node(F);

        undo_redo->add_do_method(hoodie_mesh.ptr(), "remove_node", F);
        undo_redo->add_do_method(graph_plugin.ptr(), "remove_node", F, false);
        undo_redo->add_undo_method(hoodie_mesh.ptr(), "add_node", node, hoodie_mesh->get_node_position(F), F);
        undo_redo->add_undo_method(graph_plugin.ptr(), "add_node", F, false);
    }

    for (const id_t &F : p_nodes) {
        for (const HoodieMesh::Connection &E : conns) {
            if (E.l_node == F || E.r_node == F) {
                undo_redo->add_do_method(graph_plugin.ptr(), "disconnect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
            }
        }
    }

    List<HoodieMesh::Connection> used_conns;
    for (const id_t &F : p_nodes) {
        for (const HoodieMesh::Connection &E : conns) {
            if (E.l_node == F || E.r_node == F) {
                bool cancel = false;
                for (List<HoodieMesh::Connection>::Element *R = used_conns.front(); R; R = R->next()) {
                    if (R->get().l_node == E.l_node && R->get().l_port == E.l_port && R->get().r_node == E.r_node && R->get().r_port == E.r_port) {
                        cancel = true; // to avoid ERR_ALREADY_EXISTS warning
                        break;
                    }
                }
                if (!cancel) {
                    undo_redo->add_undo_method(hoodie_mesh.ptr(), "connect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
                    undo_redo->add_undo_method(graph_plugin.ptr(), "connect_nodes", E.l_node, E.l_port, E.r_node, E.r_port);
                    used_conns.push_back(E);
                }
            }
        }
    }
}

void HoodieEditorPlugin::_delete_node_request(id_t p_node) {
    Ref<HoodieNode> node = hoodie_mesh->get_node(p_node);

    List<id_t> to_erase;
    to_erase.push_back(p_node);

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("Delete HoodieNode");
    _delete_nodes(to_erase);
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_delete_nodes_request(const TypedArray<StringName> &p_nodes) {
    List<id_t> to_erase;

    if (!p_nodes.is_empty()) {
        for (int i = 0; i < p_nodes.size(); i++) {
            id_t id = p_nodes[i].operator String().to_int();
            to_erase.push_back(id);
        }
    }

    if (to_erase.is_empty()) {
        return;
    }

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("Delete HoodieNode(s)");
    _delete_nodes(to_erase);
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_change_node_property(id_t p_id, vec_size_t p_prop_id, Variant p_val) {
    Variant old_val = hoodie_mesh->graph.nodes[p_id].node->get_property_input(p_prop_id);

    EditorUndoRedoManager *undo_redo = get_undo_redo();
    undo_redo->create_action("Change Hoodie Node property value");
    undo_redo->add_do_method(hoodie_mesh->graph.nodes[p_id].node.ptr(), "set_property_input", p_prop_id, p_val);
    undo_redo->add_undo_method(hoodie_mesh->graph.nodes[p_id].node.ptr(), "set_property_input", p_prop_id, old_val);
    undo_redo->add_do_method(hoodie_mesh->graph.nodes[p_id].node.ptr(), "mark_dirty");
    undo_redo->add_undo_method(hoodie_mesh->graph.nodes[p_id].node.ptr(), "mark_dirty");
    undo_redo->add_do_method(hoodie_mesh.ptr(), "_queue_update");
    undo_redo->add_undo_method(hoodie_mesh.ptr(), "_queue_update");
    undo_redo->commit_action();
}

void HoodieEditorPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_nodes_dragged"), &HoodieEditorPlugin::_nodes_dragged);
}

void HoodieEditorPlugin::_notification(int what) {
    switch (what) {
        case NOTIFICATION_POSTINITIALIZE: {
            hoodie_control->_update_options_menu();
        } break;
    }
}

void HoodieEditorPlugin::_scroll_changed(const Vector2 &p_scroll) {
    if (updating) {
        return;
    }
    if (hoodie_mesh.is_null() || !hoodie_mesh.is_valid()) {
        return;
    }
    updating = true;
    hoodie_mesh->set_graph_offset(p_scroll);
    updating = false;
}

void HoodieEditorPlugin::add_plugin(const Ref<HoodieNodePlugin> &p_plugin) {
    if (plugins.has(p_plugin)) {
        return;
    }
    plugins.push_back(p_plugin);
}

void HoodieEditorPlugin::remove_plugin(const Ref<HoodieNodePlugin> &p_plugin) {
    plugins.erase(p_plugin);
}

void HoodieEditorPlugin::_make_visible(bool visible) {
    if (visible) {
        button->show();
        make_bottom_panel_item_visible(hoodie_control);
    } else {
        if (hoodie_control->is_visible_in_tree()) {
            hide_bottom_panel();
        }
        
        button->hide();
    }
}

void HoodieEditorPlugin::_edit(Object *object) {
    if (!object) {
        return;
    }

    HoodieMesh *hm = Object::cast_to<HoodieMesh>(object);

    bool changed = false;
    if (hm) {
        if (hoodie_mesh.is_null()) {
            changed = true;
        } else {
            if (hoodie_mesh.ptr() != hm) {
                changed = true;
            }
        }
        hoodie_mesh = Ref<HoodieMesh>(hm);
        graph_plugin->register_hoodie_mesh(hoodie_mesh.ptr());

        hoodie_mesh->set_graph_offset(hoodie_control->graph_edit->get_scroll_offset());
        hoodie_mesh->set_verbose_mode(hoodie_control->verbose_mode);

        _update_nodes();
    } else {
        if (hoodie_mesh.is_valid()) {
            // hoodie_mesh->disconnect_changed
        }
        hoodie_mesh.unref();
    }

    if (hoodie_mesh.is_null()) {
        // hide();
    } else {
        if (changed) {
            // _update_varying_tree();
			// _update_options_menu();
			// _update_preview();
			_update_graph();
        }
    }
}

bool HoodieEditorPlugin::_handles(Object *object) const {
    return Object::cast_to<HoodieMesh>(object) != nullptr;
}

void HoodieEditorPlugin::print_debug() {
    HoodieMesh *hm = hoodie_mesh.ptr();
    String debug = "Hoodie Debug Print - ";
    debug += "hoodie_nodes.size() = " + String::num_int64(hm->graph.nodes.size());
    debug += "; ";
    debug += "connections.size() = " + String::num_int64(hm->graph.connections.size());
    UtilityFunctions::print(debug);
    UtilityFunctions::print("HoodieMesh get_graph_offset() " + hoodie_mesh->get_graph_offset());
}

HoodieEditorPlugin::HoodieEditorPlugin() {
    hoodie_control = memnew(HoodieControl);
    // hoodie_control->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    // hoodie_control->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    hoodie_control->set_editor(this);

    button = add_control_to_bottom_panel(hoodie_control, "Hoodie");
    // button = add_control_to_bottom_panel(hoodie_control->main_split, "Hoodie");

    _make_visible(false);

    Ref<HoodieNodePluginDefault> default_plugin;
    default_plugin.instantiate();
    default_plugin->set_editor(this);
    add_plugin(default_plugin);

    graph_plugin.instantiate();
    graph_plugin->set_editor(this);
}

////////////////

void HoodieNodePluginInputEditor::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            // TODO: connect("item_selected")
        } break;
    }
}

void HoodieNodePluginInputEditor::setup(HoodieEditorPlugin *p_editor) {
    editor = p_editor;
    // parameter_ref = p_parameter_ref;

    set_text("HOODIE NODE PLUGIN INPUT EDITOR");
}

void HoodieNodePluginInputEditor::_bind_methods() {
}

////////////////

void HoodieNodePluginDefaultEditor::_property_changed(const Variant &p_value, const String &p_property, Control *p_property_control, const String &p_field = "", bool p_changing = false) {
    if (p_changing) {
        return;
    }

    if (!editor) {
        return;
    }

    EditorUndoRedoManager *undo_redo = editor->get_undo_redo();

    updating = true;
    undo_redo->create_action(vformat("Edit Hoodie Node Property: %s", p_property), UndoRedo::MERGE_ENDS);

    undo_redo->add_do_property(node.ptr(), p_property, p_value);
    undo_redo->add_undo_property(node.ptr(), p_property, node->get(p_property));

    // Change values of the UI Control nodes
    if (p_property_control->is_class("EditorSpinSlider")) {
        undo_redo->add_do_method(p_property_control, "set_value_no_signal", p_value);
        undo_redo->add_undo_method(p_property_control, "set_value_no_signal", node->get(p_property));
    }
    if (p_property_control->is_class("CheckButton")) {
        undo_redo->add_do_method(p_property_control, "set_pressed_no_signal", p_value);
        undo_redo->add_undo_method(p_property_control, "set_pressed_no_signal", node->get(p_property));
    }

    undo_redo->add_do_method(node.ptr(), "mark_dirty");
    undo_redo->add_undo_method(node.ptr(), "mark_dirty");
    undo_redo->add_do_method(parent_resource.ptr(), "_queue_update");
    undo_redo->add_undo_method(parent_resource.ptr(), "_queue_update");

    // TODO: if (p_value.get_type() == Variant::OBJECT)
    // TODO: if (p_property != "constant")

    undo_redo->commit_action();
    updating = false;
}

void HoodieNodePluginDefaultEditor::_node_changed() {
    if (updating) {
        return;
    }
    for (int i = 0; i < properties.size(); i++) {
        // properties[i]->update_property();
    }
}

void HoodieNodePluginDefaultEditor::_resource_selected(const String &p_path, Ref<Resource> p_resource) {
    _open_inspector(p_resource);
}

void HoodieNodePluginDefaultEditor::_open_inspector(Ref<Resource> p_resource) {
    // TODO: InspectorDock class is not exposed in GDExtension
    // InspectorDock::get_inspector_singleton()->edit(p_resource.ptr());
    UtilityFunctions::push_warning("_open_inspector NOT IMPLEMENTED!");
}

void HoodieNodePluginDefaultEditor::_show_prop_names(bool p_show) {
    for (int i = 0; i < prop_names.size(); i++) {
        prop_names[i]->set_visible(p_show);
    }
}

void HoodieNodePluginDefaultEditor::setup(HoodieEditorPlugin *p_editor, Ref<HoodieMesh> p_parent_resource, Vector<Control *> p_properties, const Vector<StringName> &p_names, const HashMap<StringName, String> &p_overrided_names, Ref<HoodieNode> p_node) {
    editor = p_editor;
    parent_resource = p_parent_resource;
    updating = false;
    node = p_node;
    properties = p_properties;

    node_id = (int)p_node->get_meta("id");
    
    for (int i = 0; i < p_properties.size(); i++) {
        HBoxContainer *hbox = memnew(HBoxContainer);
        hbox->set_h_size_flags(SIZE_EXPAND_FILL);
        add_child(hbox);

        Label *prop_name = memnew(Label);
        String prop_name_str = p_names[i];
        if (p_overrided_names.has(p_names[i])) {
            prop_name_str = String(p_overrided_names[p_names[i]]) + ":";
        } else {
            prop_name_str = prop_name_str.capitalize() + ":";
        }
        prop_name->set_text(prop_name_str);
        prop_name->set_visible(false);
        hbox->add_child(prop_name);
        prop_names.push_back(prop_name);

        p_properties[i]->set_h_size_flags(SIZE_EXPAND_FILL);
        hbox->add_child(p_properties[i]);

        // TODO: necessary?
        // bool res_prop = Object::cast_to<EditorPropertyResource>(p_properties[i]);
        // if (res_prop) {
        // 	p_properties[i]->connect("resource_selected", callable_mp(this, &VisualShaderNodePluginDefaultEditor::_resource_selected));
        // }

        // properties[i]->connect("property_changed", callable_mp(this, &HoodieNodePluginDefaultEditor::_property_changed));

        if (properties[i]->is_class("EditorSpinSlider")) {
            // properties[i]->connect("value_changed", callable_mp(this, &HoodieNodePluginDefaultEditor::_property_changed).bind("int_value", properties[i], "", false));
            properties[i]->connect("value_changed", callable_mp(this, &HoodieNodePluginDefaultEditor::_property_changed).bind(p_names[i], properties[i], "", false));
        } else if (properties[i]->is_class("CheckButton")) {
            properties[i]->connect("toggled", callable_mp(this, &HoodieNodePluginDefaultEditor::_property_changed).bind(p_names[i], properties[i], "", false));
        } else if (properties[i]->is_class("OptionButton")) {
            properties[i]->connect("item_selected", callable_mp(this, &HoodieNodePluginDefaultEditor::_property_changed).bind(p_names[i], properties[i], "", false));
        }

        // properties[i]->set_object_and_property(node.ptr(), p_names[i]);
        // properties[i]->update_property();
    }
    node->connect("changed", callable_mp(this, &HoodieNodePluginDefaultEditor::_node_changed));
}

void HoodieNodePluginDefaultEditor::_bind_methods() {
    // TODO: ClassDB::bind_method("_show_prop_names", &HoodieNodePluginDefaultEditor::_show_prop_names); // Used with call_deferred.
}

////////////////

Control *HoodieNodePluginDefault::create_editor(const Ref<Resource> &p_parent_resource, const Ref<HoodieNode> &p_node) {
    // TODO: necessary?
    // Ref<HoodieNode> p_parent_node = Ref<HoodieNode>(p_parent_resource.ptr());

    // if (!p_parent_node.is_valid()) {
    //     return nullptr;
    // }

    // TODO: necessary for our case?
    // if (p_node->is_class("HoodieNode")) {
    //     UtilityFunctions::print("HoodieNodePluginDefault::create_editor is_class HoodieNode");
    //     HoodieNodePluginInputEditor *editor = memnew(HoodieNodePluginInputEditor);
    //     // ditor->setup(hmeditor, p_node);
    //     editor->setup(hmeditor);
    //     return editor;
    // }

    // Every HoodieNode with input properties needs to override get_editable_properties()
    Vector<StringName> properties = p_node->get_editable_properties();
    if (properties.size() == 0) {
        return nullptr;
    }

    // property_list will contain unexpected values, not only your own ADD_PROPERTYs...
    TypedArray<Dictionary> property_list = p_node->get_property_list();
    List<PropertyInfo> props;
    for (int i = 0; i < property_list.size(); i++) {
        Dictionary d = property_list[i];
        PropertyInfo pi;
        pi.name = d["name"];
        pi.class_name = d["class_name"];
        pi.type = (Variant::Type)(int)d["type"];
        pi.hint = d["hint"];
        pi.hint_string = d["hint_string"];
        pi.usage = d["usage"];
        props.push_back(pi);
    }

    Vector<PropertyInfo> pinfo;

    // ...that's why we need to search for the right ones.
    for (const PropertyInfo &E : props) {
        for (int i = 0; i < properties.size(); i++) {
            if (E.name == String(properties[i])) {
                pinfo.push_back(E);
            }
        }
    }

    if (pinfo.size() == 0) {
        return nullptr;
    }

    // Clear properties as we will push back properties again later.
    properties.clear();

    Ref<HoodieNode> node = p_node;
    Vector<Control *> editors;

    for (int i = 0; i < pinfo.size(); i++) {
        if (pinfo[i].type == Variant::BOOL) {
            CheckButton *cb = memnew(CheckButton);
            cb->set_text(p_node->get_editable_properties()[i]);
            cb->set_h_size_flags(Control::SIZE_EXPAND_FILL);
            cb->set_pressed_no_signal(p_node->get_property_input(i));
            editors.push_back(cb);
        } else if (pinfo[i].type == Variant::Type::INT) {
            if (pinfo[i].hint == PROPERTY_HINT_ENUM) {
                OptionButton *ob = memnew(OptionButton);
                ob->set_custom_minimum_size(Size2(65, 0));
                ob->set_h_size_flags(Control::SIZE_EXPAND_FILL);
                String hint_string = pinfo[i].hint_string;
                PackedStringArray enum_values = hint_string.split(",", false);
                for (int i = 0; i < enum_values.size(); i++) {
                    ob->add_item(enum_values[i], i);
                }
                ob->select(p_node->get_property_input(i));
                editors.push_back(ob);
            } else {
                EditorSpinSlider *ess = memnew(EditorSpinSlider);
                ess->set_custom_minimum_size(Size2(65, 0));
                ess->set_h_size_flags(Control::SIZE_EXPAND_FILL);
                ess->set_step(1);
                ess->set_hide_slider(true);
                ess->set_allow_greater(true);
                ess->set_allow_lesser(true);
                ess->set_value(p_node->get_property_input(i));
                editors.push_back(ess);
            }
        } else if (pinfo[i].type == Variant::Type::FLOAT) {
            EditorSpinSlider *ess = memnew(EditorSpinSlider);
            ess->set_custom_minimum_size(Size2(65, 0));
            ess->set_h_size_flags(Control::SIZE_EXPAND_FILL);
            ess->set_step(0.01);
            ess->set_hide_slider(true);
            ess->set_allow_greater(true);
            ess->set_allow_lesser(true);
            ess->set_value(p_node->get_property_input(i));
            editors.push_back(ess);
        }
        properties.push_back(pinfo[i].name);
    }
    HoodieNodePluginDefaultEditor *editor = memnew(HoodieNodePluginDefaultEditor);
    editor->setup(hmeditor, p_parent_resource, editors, properties, p_node->get_editable_properties_names(), p_node);
    return editor;
}