#pragma once

#include <array>
#include <cstddef>

#include "tar/detail/streambuf.hpp"
#include "tar/io.hpp"

namespace tar {
namespace ustar {

std::size_t constexpr BlockSize = 512;

struct header {
	static header from(tar::header const& header);

	std::array<char, 100> name;
	std::array<char, 8>   mode;
	std::array<char, 8>   uid;  // User ID.
	std::array<char, 8>   gid;  // Group ID.
	std::array<char, 12>  size;
	std::array<char, 12>  mtime;  // Modification time.
	std::array<char, 8>   chksum = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
	tar::file_type        typeflag;
	std::array<char, 100> linkname;

	std::array<char, 6>   magic   = {'u', 's', 't', 'a', 'r', '\0'};
	std::array<char, 2>   version = {'0', '0'};
	std::array<char, 32>  uname;  // User name.
	std::array<char, 32>  gname;  // Group name.
	std::array<char, 8>   devmajor;
	std::array<char, 8>   devminor;
	std::array<char, 155> prefix = {0};

	std::array<char, BlockSize - 500> pad = {0};

	operator tar::header();
};

static_assert(BlockSize == sizeof(header));

class istream: public tar::istream {
   public:
	using tar::istream::istream;
	using tar::istream::next;

	istream(std::streambuf* buf);

	~istream();

	istream& next(tar::header& h) override {
		header h_;
		this->next(h_);

		h = h_;
		return *this;
	}

	istream& next(header& h);

   private:
	pos_type header_next_;

	detail::scoped_streambuf buf_;
};

class ostream: public tar::ostream {
   public:
	using tar::ostream::ostream;
	using tar::ostream::next;

	~ostream();

	ostream& next(tar::header const& h) override {
		return this->next(header::from(h));
	}

	ostream& next(header const& h);

   private:
	void seal_();

	header   header_cur_;
	pos_type header_pos_ = -1;
};

}  // namespace ustar
}  // namespace tar
