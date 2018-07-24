#include "randm.hpp"

namespace randm {
	std::default_random_engine e{std::random_device()()};
}
