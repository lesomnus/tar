#include <array>
#include <cstddef>
#include <numeric>
#include <string>
#include <system_error>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <tar/detail/marshal.hpp>

TEST_CASE("marshal string") {
	using tar::detail::marshal;

	std::array<char, 3> dst = {0};

	SECTION("") {
		auto const [given, expected] = GENERATE(table<std::string, std::array<char, 3>>({
		    {"a", {'a', '\0', '\0'}},
		    {"ab", {'a', 'b', '\0'}},
		    {"\0b", {'\0', '\0', '\0'}},
		}));

		marshal(given, dst);
		REQUIRE(expected == dst);
	}

	using namespace Catch::Matchers;
	SECTION("needs zero termination") {
		REQUIRE_THROWS_MATCHES(marshal("abc", dst), std::system_error, MessageMatches(ContainsSubstring("too long")));
	}
}

TEST_CASE("unmarshal string") {
	using tar::detail::unmarshal;

	std::string dst;

	SECTION("") {
		auto const [given, expected] = GENERATE(table<std::array<char, 3>, std::string>({
		    {{'\0', '\0', '\0'}, ""},
		    {{'\0', '\0', 'c'}, ""},
		    {{'\0', 'b', '\0'}, ""},
		    {{'\0', 'b', 'c'}, ""},

		    {{'a', '\0', '\0'}, "a"},
		    {{'a', '\0', 'c'}, "a"},
		    {{'a', 'b', '\0'}, "ab"},
		    {{'a', 'b', 'c'}, "abc"},
		}));

		CAPTURE(given);
		unmarshal(given, dst);
		REQUIRE(expected == dst);
	}
}

TEST_CASE("marshal number") {
	using tar::detail::marshal;

	std::array<char, 3> dst = {0};

	SECTION("") {
		auto const [given, expected] = GENERATE(table<std::size_t, std::array<char, 3>>({
		    {0, {'0', '0', '\0'}},
		    {001, {'0', '1', '\0'}},
		    {042, {'4', '2', '\0'}},
		    {0123, {'1', '2', '\0'}},
		    {01, {'0', '1', '\0'}},
		    {001, {'0', '1', '\0'}},
		    {042, {'4', '2', '\0'}},
		    {0123, {'1', '2', '\0'}},
		}));

		marshal(given, dst);
		REQUIRE(expected == dst);
	}
}

TEST_CASE("unmarshal number") {
	using tar::detail::unmarshal;

	std::size_t dst;

	SECTION("") {
		auto const [given, expected] = GENERATE(table<std::array<char, 3>, std::size_t>({
		    {{'\0', '\0', '\0'}, 0},
		    {{'\0', '\0', '3'}, 0},
		    {{'\0', '2', '\0'}, 0},
		    {{'\0', '2', '3'}, 0},

		    {{'1', '\0', '\0'}, 01},
		    {{'1', '\0', '3'}, 01},
		    {{'1', '2', '\0'}, 012},
		    {{'1', '2', '3'}, 0123},

		    {{'0', '2', '\0'}, 02},
		    {{'0', '0', '3'}, 03},
		}));

		CAPTURE(given);
		unmarshal(given, dst);
		REQUIRE(expected == dst);
	}
}

std::filesystem::path repeat(std::size_t n, std::string v) {
	std::filesystem::path p;
	for(std::size_t i = 0; i < n; ++i) {
		p /= v;
	}

	return p;
};

std::filesystem::path repeat_foo(std::size_t n) {
	return repeat(n, "foo");
}

std::filesystem::path repeat_bar(std::size_t n) {
	return repeat(n, "bar");
}

TEST_CASE("split_ustar_path") {
	using tar::detail::split_ustar_path;
	using std::filesystem::path;

	SECTION("") {
		auto const [given, expected] = GENERATE(table<path, std::tuple<path, path>>({
		    {"foo/bar", {"foo/bar", ""}},
		    {repeat_foo(24), {repeat_foo(24), ""}},
		    {repeat_foo(25), {repeat_foo(25), ""}},
		    {repeat_bar(1) / repeat_foo(25), {repeat_foo(25), repeat_bar(1)}},
		    {repeat_bar(2) / repeat_foo(25), {repeat_foo(25), repeat_bar(2)}},
		}));

		REQUIRE(expected == split_ustar_path(given));
	}

	using namespace Catch::Matchers;
	SECTION("single entry too long") {
		auto const p = path(std::string(100, 'a'));
		CHECK(100 == p.string().size());
		REQUIRE_THROWS_MATCHES(split_ustar_path(p), std::system_error, MessageMatches(ContainsSubstring("too long")));
	}
	SECTION("exceeds max length") {
		auto const fit = repeat_foo(63) / "a";
		CHECK(253 == fit.string().size());
		REQUIRE_NOTHROW(split_ustar_path(fit));

		auto const over = repeat_foo(63) / "aa";
		CHECK(254 == over.string().size());
		REQUIRE_THROWS_MATCHES(split_ustar_path(over), std::system_error, MessageMatches(ContainsSubstring("too long")));
	}
}
