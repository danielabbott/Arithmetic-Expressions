#pragma once

#include <string>
#include <cstdint>
#include "ArithTypes.h"

namespace Arithmetic {

	class StringReader {
	public:

		StringReader(const std::string s_) : s(s_) {}

		char next() {
			return s[position++];
		}

		bool end() {
			return position >= s.size();
		}

		const char* c_str_from_cur_pos() {
			return &s.c_str()[position];
		}

		const char* c_str_from_last_read_char() {
			return &s.c_str()[position == 0 ? 0 : position - 1];
		}

		// Gets float value (last read character is first digit, unless no character has been read)
		FloatType get_float() {
			char* end;
			const char* s_ = c_str_from_last_read_char();
			auto val = strtold(s_, &end);
			position = static_cast<uintptr_t>(end - s.c_str());
			return val;
		}

		void advance(uintptr_t x) {
			position += x;
		}

		uintptr_t current_position() {
			return position;
		}

	private:
		std::string s;
		uintptr_t position = 0;
	};
}