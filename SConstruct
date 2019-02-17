import os


#VariantDir('build', 'src')

CCFLAGS=[
	"-g",
	"-O0",
	"-std=c++14",
	"-I./build",
	"-Wsign-compare",
	"-Wreturn-type",
	"-Wparentheses",
	"-Wpedantic",
	"-Wconversion-null",
	#"-ferror-limit=3",
	# pkg-config --cflags freetype2
	"-I/usr/include/freetype2",
	"-I/usr/include/libpng12",
]

LINKFLAGS=[
	# pkg-config --libs freetype2
	'-lfreetype'
]


env = Environment()

class Name:
	def __init__(self, path, name, ext):
		self.path = path
		self.name= name
		self.ext = ext

	def full(self):
		return os.path.join(self.path, self.name) + ext



def iter_source_files():
	for root, ds, fs in os.walk('./src'):
		for f in fs:
			if f.endswith('.cpp'):
				yield Name(root, f[:-4], '.cpp')


exttest_cpps = [
	'./src/ext/randm.cpp',
	'./src/ext/test.cpp',
	'./src/ext/filesys.cpp',
	'./src/ext/fail.cpp'
]


pixfont_cpps = [
	'./src/lodepng/lodepng.cpp',
	'./src/pixfont/main.cpp',
	'./src/ext/randm.cpp',
	'./src/ext/filesys.cpp',
	'./src/ext/fail.cpp'
]

exttest_objs = env.Object(exttest_cpps, CCFLAGS=CCFLAGS)
pixfont_objs = env.Object(pixfont_cpps, CCFLAGS=CCFLAGS)

env.Program('pixfont', pixfont_objs,
	CCFLAGS=CCFLAGS,
	LINKFLAGS=LINKFLAGS
)

env.Program('exttest', exttest_cpps,
	CCFLAGS=CCFLAGS,
	LINKFLAGS=LINKFLAGS
)
