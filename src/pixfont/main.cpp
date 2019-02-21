#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <string>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../lodepng/lodepng.h"
#include "../ext/ext.hpp"
#include "ref.hpp"

using namespace ext;

using std::string;

template <class ... Args>
void fail(Args const& ... args) {
	print(std::cerr, "ERROR: %||\n", format(args...));
}


#define LODEPNG_CALL(FUNCTION_NAME, ...) if (auto error = FUNCTION_NAME(__VA_ARGS__)) fail(#FUNCTION_NAME ": %||", lodepng_error_text(error))

#define FT_CALL(FT_FUNCTION_NAME, ...) if (auto error = FT_FUNCTION_NAME(__VA_ARGS__)) fail(#FT_FUNCTION_NAME ": %||", error)



Vec<i32,2> get_bitmap_dim(FT_Bitmap const& bm) {
	return {i32(bm.rows), i32(bm.width)};
}


template <class T, u8 N>
struct Box {
	Vec<T,N> pos;
	Vec<T,N> dim;
	//Vec<T,N> end() const { return pos + dim; }
};

void copy_bitmap_to(Ref<u8,i32,2> dst, Vec<i32,2> pos, FT_Bitmap const& bm)
{
	// FT_PIXEL_MODE_GRAY
	if (int(bm.pixel_mode) != FT_PIXEL_MODE_MONO) fail("required FT_PIXEL_MODE_MONO");
	if (int(bm.palette_mode) != 0) fail("required palette_mode 0");

	u8 * line_p = bm.buffer;
	u8 * byte_p = nullptr;

	for (size_t j = 0; j < bm.rows; ++j) {
		byte_p = line_p;
		size_t i = 0;
		while (i < bm.width) {
			u8 byte = *byte_p;
			for (size_t k = 0; k < 8 and i < bm.width; ++k)
			{
				bool mono = byte & u8(0B10000000);
				dst[pos[1] + j][pos[0] + i] = mono;
				++i;
				byte = byte << 1;
			}
			byte_p += 1;
		}
		line_p += bm.pitch;
	}
}


f32 get_dist(Vec<f32,2> a, Vec<f32,2> b) {
	return std::sqrt((a - b).pow(2).sum());
}
	

u8 get_distance_to(Vec<i32,2> pos, Ref<u8,i32,2> img, u8 value) 
{	
	// find distance
	f32 best_dist = std::numeric_limits<f32>::infinity();
	for (i32 j = 0; j < img.len(); ++j) {	
		auto img_j = img[j];
		for (i32 i = 0; i < img_j.len(); ++i) {
			if (img_j[i] == value) {
				auto dist = get_dist(Vec<f32,2>(pos), {f32(i),f32(j)});
				if (dist < best_dist) {
					best_dist = dist;					
				}
			}
		}
	}

	// map to signed discrete distance
	// value 1: inside,  0: outside
	// sign  +: outside, -: inside
	f32 const prec = 16;
	i8 d = value ? +1 : -1;
	d *= i8(((best_dist*prec) <= 127) ? (best_dist*prec) : f32(127));
	return u8(d);
}


void to_distance_field(Ref<u8,i32,2> dst, Ref<u8,i32,2> src) 
{
	for (i32 j = 0; j < src.dim[1]; ++j) {	
		for (i32 i = 0; i < src.dim[0]; ++i) {			
			auto s = src[j][i];			
			auto d = get_distance_to({i,j}, src, not s);
			dst[j][i] = d;	
		}
	}	
}

void copy_to(Ref<u8,i32,2> dst, Vec<i32,2> pos, Ref<u8,i32,2> src)
{
	for (i32 j = 0; j < src.len(); ++j) {	
		auto src_j = src[j];
		auto dst_pj = dst[pos[1] + j];
		for (i32 i = 0; i < src_j.len(); ++i) {
			dst_pj[pos[0] + i] = src_j[i];		
		}
	}
}




/*
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
*/



struct Color4{
	uint8_t r,g,b,a;
	Color4(int r, int g, int b, int a): r(r), g(g), b(b), a(a) {}
};





void lode_save_grayscale(Ref<u8,i32,2> src, char const* fname)
{
	auto s = src.lin();

	Own<u8,size_t,1> png; png.alloc({s.size() * 4}, u8(0));

	for (size_t i = 0; i < s.size(); ++i)
	{
		size_t t = 4*i;
		png[t+0] = s[i];
		png[t+1] = s[i];
		png[t+2] = s[i];
		png[t+3] = 255;		
	}	

	LODEPNG_CALL(lodepng_encode32_file,
		fname,
		png.ptr,
		src.dim[0],
		src.dim[1]
	);
}

int main(int argc, char * argv[])
{
	int const verbose = 1;
	int const pixsize = 128;
	int const extra_margin = 16;


	assert(argc == 2);
	// argv[1] -- font path

	auto fe = filesys::filename(argv[1]);
	auto fn = fe.substr(0, fe.size()-4);

	std::cout << "INFO: pixsize " << pixsize << "\n";

	FT_Library library;
	FT_CALL(FT_Init_FreeType, &library);
	FT_Face face;
	FT_CALL(FT_New_Face,
		library,
		argv[1], //"LiberationSans-Regular.ttf",
		0,
		&face
	);

	string fontname = format("%||-%||", face->family_name, face->style_name);
	string rc_name = format("%||.rc", fontname);
	string png_name = format("%||.png", fontname);

	std::ofstream rc_file(rc_name);

	if (verbose > 1) {
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

	auto & m = face->size->metrics;
	if (verbose > 0) {
		print("ascender %||\n", m.ascender >> 6);
		print("descender %||\n", m.descender >> 6);
		print("height %||\n", m.height >> 6);
	}

	auto f_asc = m.ascender >> 6;
	auto f_desc = m.descender >> 6;
	auto f_height = m.height >> 6;
	auto f_width = f_height / 2;

	print(rc_file, "font _\n");
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

	
    /*auto chars = std::string(
		" !\"#$%&'()*+,-./"
		"0123456789"
		":;<=>?@"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"[\\]^_`"
		"abcdefghijklmnopqrstuvwxyz"
		"{|}~"
	);*/

    auto chars = std::string(
		"HIJKLMNO"
	);


	// csv file
	std::ofstream fdesc(format("%||.csv", fontname));
	print(fdesc, "code,bearing_x,bearing_y,advance,width,height,pos_x,pos_y\n");

	// out image
	i32 xx = (chars.size()/8 + 1) * (pixsize + 64);
	i32 yy = (pixsize + 64) * 10;
	print("img.dim %|| %||\n", xx, yy);
    Own<u8,i32,2> out; out.alloc({xx,yy}, 0);

	// loop 
    Vec<i32,2> sep{4,4};
	Vec<i32,2> pos = sep; // cursor position
	for (auto charcode: chars)
    {
		// render glyph
		print("rendering %||\n", charcode);
		auto glyph_index = FT_Get_Char_Index(face, charcode);
		FT_CALL(FT_Load_Glyph, face, glyph_index, FT_LOAD_DEFAULT);
		FT_CALL(FT_Render_Glyph, face->glyph, FT_RENDER_MODE_MONO);

		// glyph
		auto gly = face->glyph;
		auto * met = &gly->metrics;
		auto & bm = gly->bitmap;
		auto adv = gly->advance.x >> 6;
		
		

		// create distance field
		i32 const mar = 16;
		auto dim = Vec<i32,2>{
			i32(adv + mar + mar),
			i32(f_height + mar + mar)
		};

		auto bm_pos = Vec<i32,2>{
			i32(gly->bitmap_left + mar),
			i32(f_asc - gly->bitmap_top + mar)
		};

		Own<u8,i32,2> img; img.alloc(dim, 0);
		copy_bitmap_to(img, bm_pos, bm);
		
		Own<u8,i32,2> sdf; sdf.alloc(dim, 0);
		to_distance_field(sdf, img);

		// paste to output
		if (pos[0] + sep[0] + sdf.dim[0] >= out.dim[0]) {
			pos[0] = sep[0];
			pos[1] += sep[1] + sdf.dim[1];
		}

		auto gly_pos = pos + Vec<i32,2>{i32(gly->bitmap_left - mar), i32(f_asc - gly->bitmap_top - mar)};
		
		copy_to(out, pos, sdf);
		pos[0] += sdf.dim[0] + sep[0];

		// csv file
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
		

		// rc file
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

		// show debug info
		if (verbose >= 2) {
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
		}
	}

	rc_file.close();
	
	print("writing output image... %||\n", png_name);
	lode_save_grayscale(out.ref(), png_name.c_str());

	return 0;
}




