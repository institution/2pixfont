#pragma once


#include "format.hpp"
#include "geo2.hpp"
#include "geo3.hpp"
#include "aga2.hpp"
#include "filesys.hpp"
#include "randm.hpp"
#include "fail.hpp"
#include "bitop.hpp"

namespace ext {
	using namespace geo2;


	template <class T>
	using v2 = aga2::Mv1<T>;



	inline float clamp(float x, float x0, float x1)
	{
		if (x<x0) return x0;
		if (x>x1) return x1;
		return x;
	}

	template <class T>
	inline float cmp(T x, T y) 
	{
		return (x > y)?(+1):(
			(x < y)?(-1):(0)
		);
	}

	template <class T>
	inline T abs(T x) 
	{
		return (x >= 0)?(x):(-x);
	}
	


}


