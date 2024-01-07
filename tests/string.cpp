#include <catch2/catch_test_macros.hpp>

#include <tar/detail/string.hpp>

TEST_CASE("to_octal_string") {
	using tar::detail::to_octal_string;

	REQUIRE("0" == to_octal_string(0, 0));
	REQUIRE("0" == to_octal_string(0, 1));
	REQUIRE("00" == to_octal_string(0, 2));

	REQUIRE("52" == to_octal_string(42));
	REQUIRE("52" == to_octal_string(42, 0));
	REQUIRE("52" == to_octal_string(42, 1));
	REQUIRE("52" == to_octal_string(42, 2));
	REQUIRE("052" == to_octal_string(42, 3));
	REQUIRE("0052" == to_octal_string(42, 4));
}
