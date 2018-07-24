#pragma once

#include <fstream>
#include "../ext/ext.hpp"

using ImageA = ext::darray2<uint8_t, int16_t>;


struct FontHead 
{
	int16_t height;
	int16_t ascender;
	int16_t descender;

};

struct GlyphHead
{
	uint32_t code;
	int16_t bearing_x;
	int16_t bearing_y;
	int16_t advance;
	int16_t width;
	int16_t height;
	int16_t pos_x;
	int16_t pos_y;
};


// read from little endian stream
template<class T>
void read_le(std::ifstream & f, T & r) 
{
	// use only for fixed size types acr platforms like: uint16_t
	T t;
	r = 0;
	unsigned i = 0;		
	while (i < sizeof(T)*8) {
		t = f.get();
		r = r | (t << i);
		i += 8;
	}
}

// write to little endian stream
template<class T>
void write_le(std::ofstream & f, T const& t) 
{
	// use with: int16_t uint32_t -- only types with same size on all platforms
	unsigned i = 0;	
	while (i < sizeof(T)*8) {
		f.put((t >> i) & 0xFF);
		i += 8;
	}	
}




void read(std::ifstream & f, int16_t & r);
void write(std::ofstream & f, int16_t const& r);

void read(std::ifstream & f, uint8_t & r);
void write(std::ofstream & f, uint8_t const& r);

void read(std::ifstream & f, uint32_t & r);
void write(std::ofstream & f, uint32_t const& r);


void read(std::ifstream & f, FontHead & g);
void write(std::ofstream & f, FontHead const& g);

void read(std::ifstream & f, GlyphHead & g);
void write(std::ofstream & f, GlyphHead const& g);

void read(std::ifstream & f, ImageA & img);
void write(std::ofstream & f, ImageA const& img);




