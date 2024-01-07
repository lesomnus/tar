#include "tar/io.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <system_error>

#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include <grp.h>
#include <pwd.h>

namespace tar {

file_type type_from_std(std::filesystem::file_type t) {
	using ft = std::filesystem::file_type;

	switch(t) {
	case ft::not_found:
		throw std::ios_base::failure("file not found");

	case ft::regular:
		return file_type::regular;
	case ft::symlink:
		return file_type::symlink;
	case ft::character:
		return file_type::character;
	case ft::block:
		return file_type::block;
	case ft::directory:
		return file_type::directory;
	case ft::fifo:
		return file_type::fifo;

	default:
		throw std::system_error(std::make_error_code(std::errc::not_supported), "unknown type of file");
	}
}

ostream& ostream::next(std::filesystem::path const& p, std::filesystem::path const& as) {
	auto const status = std::filesystem::status(p);
	auto const type   = type_from_std(status.type());

	struct ::stat info;
	if(auto const ret = ::lstat(p.c_str(), &info); ret < 0) {
		throw std::system_error(errno, std::generic_category(), std::strerror(errno));
	}

	std::filesystem::path link;
	if(type == file_type::symlink) {
		link = std::filesystem::read_symlink(p);
	}

	header h{
	    .path        = as.empty() ? p : as,
	    .permissions = status.permissions(),

	    .uid  = info.st_uid,
	    .gid  = info.st_gid,
	    .size = std::filesystem::file_size(p),

	    .last_write_time = std::filesystem::file_time_type(std::chrono::seconds(info.st_mtim.tv_sec)),

	    .type = type,
	    .link = link,

	    .device_number_major = major(info.st_dev),
	    .device_number_minor = minor(info.st_dev),
	};
	if(auto const* v = ::getpwuid(info.st_uid); v != nullptr) {
		h.user_name = v->pw_name;
	}
	if(auto const* v = ::getgrgid(info.st_gid); v != nullptr) {
		h.group_name = v->gr_name;
	}

	this->next(h);
	if(type == file_type::symlink) {
		return *this;
	}

	std::ifstream f(p, std::ios::binary);
	*this << f.rdbuf();

	return *this;
}

}  // namespace tar
