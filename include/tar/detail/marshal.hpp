#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <tuple>

#include "tar/detail/string.hpp"

namespace tar {
namespace detail {

template<std::size_t N>
void marshal(std::string const& v, std::array<char, N>& dst) {
	if(v.size() >= N) {
		throw std::system_error(std::make_error_code(std::errc::filename_too_long));
	}

	v.copy(reinterpret_cast<char*>(dst.data()), v.size());
}

template<std::size_t N>
void unmarshal(std::array<char, N> const& dst, std::string& v) {
	if(dst[0] == '\0') {
		v.clear();
		return;
	}

	v = std::string(dst.begin(), dst.end());
	if(auto const p = v.find_first_of('\0'); p != std::string::npos) {
		v.erase(p);
	}
}

template<std::integral T, std::size_t N>
void marshal(T v, std::array<char, N>& dst, std::size_t w = N - 1) {
	detail::to_octal_string(v, w)
	    .copy(reinterpret_cast<char*>(dst.data()), w);
}

template<std::integral T, std::size_t N>
void unmarshal(std::array<char, N> const& dst, T& v) {
	std::string s;
	unmarshal(dst, s);

	try {
		v = s.empty() ? 0 : std::stoul(s, 0, 8);
	} catch(...) {
		v = 0;
	}
}

// Splits given path into two parts N and P by '/'.
// N be the longest suffix of given path but shorter than length 100.
// P is rest prefix of given path.
std::tuple<std::filesystem::path, std::filesystem::path> split_ustar_path(std::filesystem::path const& p);

}  // namespace detail
}  // namespace tar
