#include <array>
#include <iostream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include <tar/detail/streambuf.hpp>

TEST_CASE("scoped_streambuf") {
	using tar::detail::scoped_streambuf;

	std::stringstream input("0123456789");
	scoped_streambuf  buf(input.rdbuf());
	buf.reset(2, 7);

	std::istream i(&buf);

	std::array<char, 10> r = {0};
	i.read(r.data(), 2);
	REQUIRE(static_cast<bool>(i));
	REQUIRE(2 == i.gcount());
	REQUIRE("23" == std::string(r.begin(), r.begin() + 2));

	i.read(r.data(), 5);
	REQUIRE_FALSE(static_cast<bool>(i));
	REQUIRE(3 == i.gcount());
	REQUIRE("456" == std::string(r.begin(), r.begin() + 3));
}
