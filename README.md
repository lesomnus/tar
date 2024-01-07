# tar

C++ implementation of [Tape ARchive (TAR)](https://en.wikipedia.org/wiki/Tar_(computing)).

## Features

- Unix Standard TAR

## Example

### Write

```cpp
#include <fstream>
#include <iostream>

#include <tar/ustar.hpp>

int main(int argc, char* argv[]) {
	std::ofstream dst("foo.tar");

	tar::ustar::ostream o(dst.rdbuf());
	o.next(tar::header{.path = "Burger"});
	o << "Royale with Cheese" << std::endl;

	return 0;
}
```

### Read

```cpp
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include <tar/ustar.hpp>

int main(int argc, char* argv[]) {
	std::ifstream src("foo.tar");

	tar::ustar::istream i(src.rdbuf());

	tar::header h;
	i.next(h);
	assert("Burger" == h.path);

	std::stringstream body;
	body << i.rdbuf();
	assert("Royale with Cheese\n" == body.str());

	return 0;
}
```
