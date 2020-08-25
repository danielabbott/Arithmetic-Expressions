#include "Variable.h"
#include <cctype>

using namespace std;

namespace Arithmetic {

	unsigned int get_var_name_len(const char* name) {
		unsigned int var_name_len = 0;
		const char* s = name;
		while (s[var_name_len] && (isalnum(s[var_name_len]) || s[var_name_len] == '_')) {
			var_name_len++;
		}
		return var_name_len;
	}

	std::optional<FloatType> find_var(VariableSet const& variables, string const& name) {
		auto it = variables.find(name);
		if (it == variables.end()) {
			return nullopt;
		}

		return (*it).second;
	}
}