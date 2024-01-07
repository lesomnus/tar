#include "tar/detail/marshal.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <system_error>
#include <utility>

#include "tar/ustar.hpp"

namespace tar {
namespace detail {

std::tuple<std::filesystem::path, std::filesystem::path> split_ustar_path(std::filesystem::path const& p) {
	auto constexpr n_size = sizeof(ustar::header::name);
	auto constexpr p_size = sizeof(ustar::header::prefix);

	auto const  s = p.string();
	std::size_t l = s.size();
	if(l < n_size) [[likely]] {
		// No need to split.
		return std::make_tuple(s, "");
	}
	if(
	    (p.filename().string().size() >= n_size)  // Not split-able.
	    || (l > ((n_size - 1) + (p_size - 1)))    // Not fit.
	    ) [[unlikely]] {
		throw std::system_error(std::make_error_code(std::errc::filename_too_long));
	}

	std::filesystem::path n_path;
	std::filesystem::path p_path;
	for(auto const& entry: p) {
		if(l < n_size) {
			n_path /= entry;
		} else {
			p_path /= entry;
			l -= (entry.string().size() + 1);
		}
	}

	return std::make_tuple(std::move(n_path), std::move(p_path));
}

}  // namespace detail
}  // namespace tar
