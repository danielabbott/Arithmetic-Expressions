#pragma once

#include "ArithTypes.h"

#include <vector>
#include <string>
#include <optional>
#include <map>

namespace Arithmetic {

	unsigned int get_var_name_len(const char* name);

	// Variables names case sensitive
	using VariableSet = std::map<std::string, FloatType>;

	// name now points to the character after the variable name
	// Returns value of variable or nullopt
	std::optional<FloatType> find_var(VariableSet const& variables, std::string const& name);

}