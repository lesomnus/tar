#pragma once

#include <filesystem>
#include <iosfwd>

#include "tar/types.hpp"

namespace tar {

class istream: public std::istream {
   public:
	virtual istream& next(header& header) = 0;
};

class ostream: public std::ostream {
   public:
	using std::ostream::ostream;

	virtual ostream& next(header const& header) = 0;

	ostream& next(std::filesystem::path const& p, std::filesystem::path const& as = "");
};

}  // namespace tar
