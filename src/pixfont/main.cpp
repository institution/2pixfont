#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../lodepng/lodepng.h"
#include "../ext/ext.hpp"


using v2s = ext::v2<int16_t>;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::string;

template <class ... Args>
void fail(Args const& ... args) {
	print(std::cerr, "ERROR: %||\n", format(args...));
}


using v2f = aga2::Mv1<float>;
using v2i = aga2::Mv1<int>;


#define LODEPNG_CALL(FUNCTION_NAME, ...) if (auto error = FUNCTION_NAME(__VA_ARGS__)) fail(#FUNCTION_NAME ": %||", lodepng_error_text(error))

#define FT_CALL(FT_FUNCTION_NAME, ...) if (auto error = FT_FUNCTION_NAME(__VA_ARGS__)) fail(#FT_FUNCTION_NAME ": %||", error)


struct Buffer2D
{
	v2i dim;
	std::vector<uint8_t> buf;

	Buffer2D(v2i const& dim): dim(dim) {
		buf.resize(dim[0] * dim[1] * 4);
	}

};



void copy(Buffer2D & trg, v2i const& pos, FT_Bitmap const& bm)
{
	if (int(bm.pixel_mode) != 2) fail("required pixel_mode 2");
	if (int(bm.palette_mode) != 0) fail("required palette_mode 0");

	uint8_t * bp = bm.buffer;
	for (size_t j=0; j<bm.rows; ++j) {
		for (size_t i=0; i<bm.width; ++i) {

			uint8_t gray = *(bp + i);

			auto tp = 4 * ((pos[0] + i) + (pos[1] + j) * trg.dim[0]);

			trg.buf.at(tp+0) = 0;
			trg.buf.at(tp+1) = 0;
			trg.buf.at(tp+2) = 0;
			trg.buf.at(tp+3) = gray;

		}
		bp += bm.pitch;
	}

}



void copy_bitmap(ext::darray2<uint8_t, int16_t> & img, v2s pos, FT_Bitmap const& bm)
{
	if (int(bm.pixel_mode) != 2) fail("required pixel_mode 2");
	if (int(bm.palette_mode) != 0) fail("required palette_mode 0");

	uint8_t * bp = bm.buffer;
	for (size_t j=0; j<bm.rows; ++j) {
		for (size_t i=0; i<bm.width; ++i) {

			uint8_t gray = *(bp + i);

			img(pos + v2s(i,j)) = gray;
		}
		bp += bm.pitch;
	}

}

struct Color4{
	uint8_t r,g,b,a;
	Color4(int r, int g, int b, int a): r(r), g(g), b(b), a(a) {}
};


void fill(Buffer2D & trg, v2i const& pos, v2i const dim, Color4 const color) {


	for (int j=0; j<dim[1]; ++j) {
		for (int i=0; i<dim[0]; ++i) {

			auto tp = 4 * ((pos[0] + i) + (pos[1] + j) * trg.dim[0]);

			trg.buf.at(tp+0) = color.r;
			trg.buf.at(tp+1) = color.g;
			trg.buf.at(tp+2) = color.b;
			trg.buf.at(tp+3) = color.a;
			//trg.buf.at(tp+3) = gray;

		}

	}

}






int main(int argc, char * argv[])
{
	assert(argc == 3);
	// argv[1] -- font path
	// argv[2] -- font size



	int pixsize = atoi(argv[2]);

	std::cout << "INFO: pixsize " << pixsize << "\n";

	auto fe = filesys::filename(argv[1]);
	auto fn = fe.substr(0, fe.size()-4);



	FT_Library library;

	FT_CALL(FT_Init_FreeType, &library);

	FT_Face face;

	FT_CALL(FT_New_Face,
		library,
		argv[1], //"LiberationSans-Regular.ttf",
		//"ColTiny.ttf",
		0,
		&face
	);




	string fontname = format("%||-%||-%||", face->family_name, face->style_name, pixsize);
	string rc_name = format("%||.rc", fontname);
	string png_name = format("%||.png", fontname);

	std::ofstream rc_file(rc_name);


	if (0) {
		print("num_faces %||\n", face->num_faces);
		print("num_glyphs %||\n", face->num_glyphs);
		print("face_flags %||\n", face->face_flags);
		print("units_per_EM %||\n", face->units_per_EM);
		print("num_fixed_sizes %||\n", face->num_fixed_sizes);

		print("available_sizes");
		for (int i = 0; i < face->num_fixed_sizes; ++i)
		{
			FT_Bitmap_Size & a = face->available_sizes[i];
			print(" (%|| %||)", a.width, a.height);
		}
		print("\n");
    }

    /*
    1/72th of an inch

    DPI dot/inch * 16 1/72 inch

    16 * DPI/72 dot
    */


	FT_CALL(FT_Set_Char_Size,
		face,
		0,          // char_width in 1/64th of points
		pixsize*64, // char_height in 1/64th of points, 1 point = 1/72 inch
		72,         // x dpi
		72          // y dpi
	);



	auto& m = face->size->metrics;
	print("ascender %||\n", m.ascender >> 6);
	print("descender %||\n", m.descender >> 6);
	print("height %||\n", m.height >> 6);


	auto f_asc = m.ascender >> 6;
	auto f_desc = m.descender >> 6;
	auto f_height = m.height >> 6;
	auto f_width = f_height / 2;

	print(rc_file, "name \"%||\"\n", fontname);
	print(rc_file, "max_sprites 256\n");
	print(rc_file, "height %||\n", f_height);
	print(rc_file, "width %||\n", f_width);
	print(rc_file, "ascender %||\n", f_asc);
	print(rc_file, "descender %||\n", f_desc);
	print(rc_file, "pad 2 0\n");
	print(rc_file, "\n");


	/*FT_CALL(FT_Set_Pixel_Sizes,
		face,
		0,      // pix width
		16      // pix height
	);*/


    auto chars = std::string(
		" !\"#$%&'()*+,-./"
		"0123456789"
		":;<=>?@"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"[\\]^_`"
		"abcdefghijklmnopqrstuvwxyz"
		"{|}~"
	);
    /*auto chars = std::string(
		"abcd"
	);
    */


    std::ofstream fdesc(format("%||.csv", fontname));
    print(fdesc, "code,bearing_x,bearing_y,advance,width,height,pos_x,pos_y\n");


    Buffer2D image(v2i(pixsize * f_height, pixsize * f_height));
	fill(image, v2i(0,0), image.dim, Color4(100,100,100,255));

    v2i sep(4,4);
    v2i pos = sep;

	ext::darray2<uint8_t, int16_t> img;
	img.resize(v2s(pixsize * f_height, pixsize * f_height), 0);

	//FontHead fh;
	//fh.height = f_height;
	//fh.ascender = (m.ascender >> 6);
	//fh.descender = (m.descender >> 6);

	//write(ftab, fh);

    //write(ftab, (uint32_t)chars.size());
    for (auto charcode: chars)
    {
		auto glyph_index = FT_Get_Char_Index(face, charcode);


		FT_CALL(FT_Load_Glyph, face, glyph_index, FT_LOAD_DEFAULT);

		FT_CALL(FT_Render_Glyph, face->glyph, FT_RENDER_MODE_NORMAL);

		auto gly = face->glyph;
		auto *met = &gly->metrics;

		auto & bm = gly->bitmap;


		//unsigned char*  buffer;
		//void*           palette;


/*
		print("%||\n", charcode);
		print(" width %||\n", met->width);
		print(" height %||\n", met->height);
		print(" bearing_x %||\n", met->horiBearingX);
		print(" bearing_y %||\n", met->horiBearingY);
		print(" advance %||\n", met->horiAdvance);

		print(" bitmap_left %||\n", gly->bitmap_left);
		print(" bitmap_top %||\n", gly->bitmap_top);
		print(" advance[0] %||\n", gly->advance.x >> 6);
		print(" advance[1] %||\n", gly->advance.y >> 6);

		print(" bitmap\n");
		print("  rows %||\n", bm.rows);
		print("  width %||\n", bm.width);
		print("  pitch %||\n", bm.pitch);
		print("  num_grays %||\n", bm.num_grays);
		print("  pixel_mode %||\n", int(bm.pixel_mode));
		print("  palette_mode %||\n", int(bm.palette_mode));
*/
		/* now, draw to our target surface */
		/*my_draw_bitmap( &slot->bitmap,
                  pen_x + slot->bitmap_left,
                  pen_y - slot->bitmap_top );
		*/

		auto adv = gly->advance.x >> 6;
		auto dim = v2i(bm.width, bm.rows);

		if (pos[0] + adv + sep[0] >= image.dim[0])
		{
			pos[0] = sep[0];
			pos[1] += f_height + 8 + sep[1];
		}

		fill(image, pos, v2i(adv,f_height), Color4(0,0,0,0));


		auto gly_pos = pos + v2i(gly->bitmap_left, f_asc - gly->bitmap_top);

		copy(image, gly_pos, bm);
		copy_bitmap(img, v2s(gly_pos[0], gly_pos[1]), bm);

		pos[0] += adv + sep[0];


		print(fdesc, "%||,%||,%||,%||,%||,%||,%||,%||\n",
			int(charcode),
			gly->bitmap_left,
			gly->bitmap_top,
			(gly->advance.x >> 6),
			bm.width,
			bm.rows,
			gly_pos[0],
			gly_pos[1]
		);

		/*
		GlyphHead g;
		g.code = charcode;
		g.bearing_x = gly->bitmap_left;
		g.bearing_y = gly->bitmap_top;
		g.advance = (gly->advance.x >> 6);
		g.width = bm.width;
		g.height = bm.rows;
		g.pos_x = gly_pos[0];
		g.pos_y = gly_pos[1];
		*/

		print(rc_file, "sprite %||\n", int(charcode));

		if (charcode == '\\' or charcode == '\"') {
			print(rc_file, "name \"\\%||\"\n", charcode);
		}
		else {
			print(rc_file, "name \"%||\"\n", charcode);
		}

		print(rc_file, "box %|| %|| %|| %||\n", gly_pos[0], gly_pos[1], bm.width, bm.rows);
		print(rc_file, "delta %|| %||\n", gly->bitmap_left, -gly->bitmap_top + f_asc);
		print(rc_file, "adv %||\n", (gly->advance.x >> 6));
		print(rc_file, "\n");

	}

	rc_file.close();

	LODEPNG_CALL(lodepng::encode,
		png_name,
		image.buf,
		image.dim[0],
		image.dim[1]
	);


	return 0;
}






