import os
#VariantDir('build', 'src')
CCFLAGS=[
	"-g",
	"-O0",
	"-std=c++17",
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
env.Program('pixfont', [
		'./src/pixfont/main.cpp',
		'./src/pixfont/ext.cpp',
		'./src/pixfont/png.cpp',
		'./src/pixfont/lodepng/lodepng.cpp',
	],
	CCFLAGS=CCFLAGS,
	LINKFLAGS=LINKFLAGS
)
env.Program('test_ext', [
		'./src/pixfont/test_ext.cpp',
		'./src/pixfont/ext.cpp',
	],
	CCFLAGS=CCFLAGS,
	LINKFLAGS=LINKFLAGS
)
