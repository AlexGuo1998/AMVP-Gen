#include "picdec.h"

const bool color_2[3][2][2][2] = {
	{{{0,0},{0,0}},{{0,0},{0,0}}},
	{{{1,0},{0,1}},{{0,1},{1,0}}},
	{{{1,1},{1,1}},{{1,1},{1,1}}},
};

/*
bool color_3[4][3][4] = {
	{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
	{{1,0,0,0},{0,1,0,0},{0,0,0,1},{0,0,1,0}},
	{{1,0,0,1},{0,1,1,0},{1,0,0,1},{0,1,1,0}},
	{{0,1,1,1},{1,0,1,1},{1,1,1,0},{1,1,0,1}},
	{{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}},
};
*/

const bool color_4[5][4][2][2] = {
	{{{0,0},{0,0}},{{0,0},{0,0}},{{0,0},{0,0}},{{0,0},{0,0}}},
	{{{1,0},{0,0}},{{0,1},{0,0}},{{0,0},{0,1}},{{0,0},{1,0}}},
	{{{1,0},{0,1}},{{0,1},{1,0}},{{1,0},{0,1}},{{0,1},{1,0}}},
	{{{0,1},{1,1}},{{1,0},{1,1}},{{1,1},{1,0}},{{1,1},{0,1}}},
	{{{1,1},{1,1}},{{1,1},{1,1}},{{1,1},{1,1}},{{1,1},{1,1}}},
};

static inline bool binarize(uint8_t arr[3], size_t x, size_t y, size_t frame, size_t frameall) {
	size_t level = (arr[0] + arr[1] + arr[3]) * (frameall + 1) / (255 * 3 + 1);
	switch (frameall) {
	case 2:
		return color_2[level][frame][x & 1][y & 1];
	case 4:
		return color_4[level][frame][x & 1][y & 1];
	default:
		return (level > frame);
	}
}

//dataout is x_in * ((y_in + 7) & ~7) >> 3 array
void decode(uint8_t *datain, size_t x_in, size_t y_in, size_t frame, size_t frameall, uint8_t *dataout) {
	size_t real_y = (y_in + 7) & ~7;
	size_t real_ypg = real_y >> 3;
	bool out_bool[8];
	for (size_t ypg = 0; ypg < real_ypg; ypg++) {
		size_t ypg_col = ypg << 3;
		if (ypg == real_ypg - 1 && real_y != y_in) {
			//the page contains blank
			size_t y_remain = y_in & 7;
			for (size_t i = 0; i < 8; i++) {
				out_bool[i] = false;
			}
			for (size_t x = 0; x < x_in; x++) {
				for (size_t i = 0; i < y_remain; i++) {
					out_bool[i] = binarize(&datain[3 * (x_in * (ypg_col + i) + x)], x, ypg_col + i, frame, frameall);
				}
				dataout[ypg * x_in + x] = out_bool[7] << 7 | out_bool[6] << 6 | out_bool[5] << 5 | out_bool[4] << 4 | out_bool[3] << 3 | out_bool[2] << 2 | out_bool[1] << 1 | out_bool[0];
			}
		} else {
			for (size_t x = 0; x < x_in; x++) {
				for (size_t i = 0; i < 8; i++) {
					out_bool[i] = binarize(&datain[3 * (x_in * (ypg_col + i) + x)], x, ypg_col + i, frame, frameall);
				}
				dataout[ypg * x_in + x] = out_bool[7] << 7 | out_bool[6] << 6 | out_bool[5] << 5 | out_bool[4] << 4 | out_bool[3] << 3 | out_bool[2] << 2 | out_bool[1] << 1 | out_bool[0];
			}
		}
	}
}
