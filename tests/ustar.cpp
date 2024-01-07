#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <tar/detail/marshal.hpp>
#include <tar/ustar.hpp>

template<std::size_t N>
std::string to_string(std::array<char, N> v) {
	std::string s;
	tar::detail::unmarshal(v, s);
	return s;
}

TEST_CASE("header::from") {
	tar::header given{
	    .path        = "foo/bar/baz",
	    .permissions = std::filesystem::perms(0644),

	    .uid  = 01234567,
	    .gid  = 02461357,
	    .size = 036251473625,

	    .last_write_time = std::filesystem::file_time_type(std::chrono::seconds(014545443667)),

	    .type = tar::file_type::regular,
	    .link = "over/the/rainbow",

	    .user_name  = "O-Ren Ishii",
	    .group_name = "DeVAS",

	    .device_number_major = 053,
	    .device_number_minor = 064,
	};

	SECTION("basic") {
		auto h = tar::ustar::header::from(given);

		CHECK("foo/bar/baz" == to_string(h.name));
		CHECK(std::string("0000644") == to_string(h.mode));
		CHECK("1234567" == to_string(h.uid));
		CHECK("2461357" == to_string(h.gid));
		CHECK("36251473625" == to_string(h.size));
		CHECK("14545443667" == to_string(h.mtime));
		CHECK("        " == to_string(h.chksum));
		CHECK(tar::file_type::regular == h.typeflag);
		CHECK("over/the/rainbow" == to_string(h.linkname));

		CHECK("ustar" == to_string(h.magic));
		CHECK("00" == to_string(h.version));
		CHECK("O-Ren Ishii" == to_string(h.uname));
		CHECK("DeVAS" == to_string(h.gname));
		// CHECK("0000053" == to_string(h.devmajor));
		// CHECK("0000064" == to_string(h.devminor));
		CHECK("" == to_string(h.prefix));
	}

	SECTION("long name") {
		std::string const name(60, 'a');
		std::string const prefix(60, 'b');

		given.path = prefix + "/" + name;

		auto h = tar::ustar::header::from(given);
		CHECK(name == to_string(h.name));
		CHECK(prefix == to_string(h.prefix));
	}
}

TEST_CASE("header::operator tar::header") {
	tar::ustar::header given{
	    .name     = {'f', 'o', 'o', '\0'},
	    .mode     = {'0', '0', '0', '0', '6', '4', '4', '\0'},
	    .uid      = {'1', '2', '3', '4', '5', '6', '7', '\0'},
	    .gid      = {'2', '4', '6', '1', '3', '5', '7', '\0'},
	    .size     = {'3', '6', '2', '5', '1', '4', '7', '3', '6', '2', '5', '\0'},
	    .mtime    = {'1', '4', '5', '4', '5', '4', '4', '3', '6', '6', '7', '\0'},
	    .chksum   = {'2', '7', '1', '1', '4', '2', ' ', '\0'},
	    .typeflag = tar::file_type::block,
	    .linkname = {'b', 'a', 'z', '\0'},

	    .magic    = {'u', 's', 't', 'a', 'r', '\0'},
	    .version  = {'0', '0'},
	    .uname    = {'O', '-', 'R', 'e', 'n', ' ', 'I', 's', 'h', 'i', 'i', '\0'},
	    .gname    = {'D', 'e', 'V', 'A', 'S', '\0'},
	    .devmajor = {'6', '3', '4', '3', '4', '5', '5', '\0'},
	    .devminor = {'5', '7', '2', '3', '3', '2', '3', '\0'},
	    .prefix   = {'b', 'a', 'r'},
	};

	SECTION("basic") {
		tar::header h = given;

		CHECK("bar/foo" == h.path.string());
		CHECK(std::filesystem::perms(0644) == h.permissions);
		CHECK(01234567 == h.uid);
		CHECK(02461357 == h.gid);
		CHECK(036251473625 == h.size);
		CHECK(std::chrono::seconds(014545443667) == h.last_write_time.time_since_epoch());
		CHECK("baz" == h.link.string());

		CHECK("O-Ren Ishii" == h.user_name);
		CHECK("DeVAS" == h.group_name);
		// CHECK(06343455 == h.device_number_major);
		// CHECK(05723323 == h.device_number_minor);
	}
}

TEST_CASE("ostream") {
	// test fails if environment changes (e.g. uid, gid, mtime, and etc)
	// so skip this test at this time.
	return;

	using std::filesystem::path;

	SECTION("") {
		auto const data_root = path(__FILE__).parent_path() / "data";

		auto const [expected_p, given_files] = GENERATE(table<path, std::vector<path>>({
		    {
		        "Quentin Tarantino.tar",
		        {
		            "Quentin Tarantino",
		        },
		    },
		    {
		        "Django Unchained.tar",
		        {
		            "Quentin Tarantino",
		            "Christoph Waltz",
		            "Jamie Foxx",
		            "Samuel Jackson",
		            "Leonardo DiCaprio",
		        },
		    },
		}));

		std::stringstream expected;
		{
			std::ifstream f(data_root / expected_p, std::ios::binary);
			expected << f.rdbuf();
		}

		std::stringstream result;
		{
			tar::ustar::ostream o(result.rdbuf());
			for(auto const p: given_files) {
				o.next(data_root / p, p);
			}
		}
		REQUIRE(expected.str() == result.str());
	}
}

TEST_CASE("istream") {
	using std::filesystem::path;
	auto const data_root = std::filesystem::path(__FILE__).parent_path() / "data";

	std::vector<char> buf;

	SECTION("") {
		auto const [given, expected_files] = GENERATE(table<path, std::vector<std::pair<path, std::string>>>({
		    {
		        "Quentin Tarantino.tar",
		        {
		            {
		                "Quentin Tarantino",
		                "March 27, 1963\n",
		            },
		        },
		    },
		    {
		        "Django Unchained.tar",
		        {
		            {
		                "Quentin Tarantino",
		                "March 27, 1963\n",
		            },
		            {
		                "Christoph Waltz",
		                "October 4, 1956\n",
		            },
		            {
		                "Jamie Foxx",
		                "December 13, 1967\n",
		            },
		            {
		                "Samuel Jackson",
		                "December 21, 1948\n",
		            },
		            {
		                "Leonardo DiCaprio",
		                "November 11, 1974\n",
		            },
		        },
		    },
		}));

		std::ifstream input(data_root / given, std::ios::binary);

		tar::ustar::istream i(input.rdbuf());
		for(auto const& [name, body]: expected_files) {
			CAPTURE(name);

			tar::header h;
			i.next(h);
			REQUIRE(static_cast<bool>(i));

			std::stringstream ss;
			ss << i.rdbuf();
			REQUIRE(body == ss.str());

			char c;
			i.read(&c, 1);
			REQUIRE(i.eof());
		}
	}
}
