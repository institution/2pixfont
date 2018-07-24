
#include "filesys.hpp"

//#include <linux/limits.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace filesys{


	Path self_path()
	{
		char buf[PATH_MAX];
		
		if (readlink("/proc/self/exe", buf, PATH_MAX) <= 0) 
		{
			ext::fail("error while reading /proc/self/exe\n");
		}

		buf[PATH_MAX-1] = '\0';
		return Path(buf);
	}


}
