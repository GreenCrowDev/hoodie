#ifndef HOODIE_HOODIE_ENUMS_H
#define HOODIE_HOODIE_ENUMS_H

#include <godot_cpp/variant/string.hpp>

namespace godot::greencrow::hoodie {

enum PropertyKeyword {
	INVALID,
	POINT,
	NORMAL,
	TANGENT,
	COLOR,
	UV,
	UV2,
};

inline PropertyKeyword name_to_property_keyword(const String &p_name) {
	if (p_name == "point") {
		return PropertyKeyword::POINT;
	} else if (p_name == "N") {
		return PropertyKeyword::NORMAL;
	} else if (p_name == "T") {
		return PropertyKeyword::TANGENT;
	} else if (p_name == "C") {
		return PropertyKeyword::COLOR;
	} else if (p_name == "UV") {
		return PropertyKeyword::UV;
	} else if (p_name == "UV2") {
		return PropertyKeyword::UV2;
	} else {
		return PropertyKeyword::INVALID;
	}
}

} // namespace godot::greencrow::hoodie

#endif // HOODIE_HOODIE_ENUMS_H