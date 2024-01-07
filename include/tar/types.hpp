#pragma once

#include <climits>
#include <cstdint>
#include <filesystem>
#include <string>

namespace tar {

static_assert(8 == CHAR_BIT);

enum class file_type : char {
	regular    = '0',
	hard       = '1',
	symlink    = '2',
	character  = '3',
	block      = '4',
	directory  = '5',
	fifo       = '6',
	contiguous = '7',
};

struct header {
	std::filesystem::path  path;
	std::filesystem::perms permissions;

	std::uintmax_t uid;
	std::uintmax_t gid;
	std::uintmax_t size;

	std::filesystem::file_time_type last_write_time;

	file_type             type;
	std::filesystem::path link;

	std::string user_name;
	std::string group_name;

	std::uintmax_t device_number_major;
	std::uintmax_t device_number_minor;
};

}  // namespace tar
