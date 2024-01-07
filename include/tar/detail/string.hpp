#pragma once

#include <concepts>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace tar {
namespace detail {

template<std::integral T>
std::string to_octal_string(T v, std::size_t w = 0) {
	std::ostringstream os;
	os << std::setw(w) << std::setfill('0') << std::oct << v;

	return std::move(os).str();
}

}  // namespace detail
}  // namespace tar
