#pragma once

#include <memory>
#include <string>

struct StringComponent_t {
	std::string string;
};

using StringComponentPtr_t = std::unique_ptr<StringComponent_t>;