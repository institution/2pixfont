#pragma once

#include <stdlib.h>
#include <cassert>
#include "format.hpp"

namespace ext{

	extern bool FAIL_THROWS;

	template <class... Args>
	[[noreturn]] 
	void fail(char const* fmt, Args... args) {
		print(std::cerr, fmt, args...);
		if (FAIL_THROWS) {
			throw std::exception();
		}
		else {
			assert(0);
		}
		//exit(-1); 
	}
	
	
}
