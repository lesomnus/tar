#include <fstream>
#include <iostream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include <tar/ustar.hpp>

TEST_CASE("example simple") {
	std::stringstream stream;

	{
		tar::ustar::ostream o(stream.rdbuf());
		o.next(tar::header{.path = "Burger"});
		o << "Royale with Cheese" << std::endl;
	}

	{
		tar::ustar::istream i(stream.rdbuf());

		tar::header h;
		i.next(h);
		CHECK("Burger" == h.path);

		std::stringstream body;
		body << i.rdbuf();
		CHECK("Royale with Cheese\n" == body.str());
	}
}
