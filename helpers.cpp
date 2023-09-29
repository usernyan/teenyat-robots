#include "helpers.h"
#include <cmath>
#include <cstdlib> //has int abs(int X)


struct color_t hsv_to_color(double hue, double sat, double val) {
	double c = val * sat;
	double m = val - c;
	int hue_i = std::floor(hue * 6);
	double x = c * (1 - std::abs( std::fmod(hue * 6, 2) - 1 ));
	double r;
	double g;
	double b;
	//c, 
	switch (hue_i) {
		case 0:
			r = c;
			g = x;
			b = 0;
		break;
		case 1:
			r = x;
			g = c;
			b = 0;
		break;
		case 2:
			r = 0;
			g = c;
			b = x;
		break;
		case 3:
			r = 0;
			g = x;
			b = c;
		break;
		case 4:
			r = x;
			g = 0;
			b = c;
		break;
		case 5:
			r = c;
			g = 0;
			b = x;
		break;
	}
	r += m;
	g += m;
	b += m;
	return {
		(uint8_t)(0xFF * r),
		(uint8_t)(0xFF * g),
		(uint8_t)(0xFF * b),
		0xFF
	};
}

//used for sending colors to certain SDL_gfx functions
uint32_t color_to_int(struct color_t c) {
	return c.a << 24 | c.b << 16 | c.g << 8 | c.r;
}

float sin_360(const int x) {
    return std::sin(x / 360.0 * 2 * M_PI);
}
float cos_360(const int x) {
    return std::cos(x / 360.0 * 2 * M_PI);
}
float rad_360(const int x) {
    return x / 360.0 * 2 * M_PI;
}
int mod(const int k, const int n) {
    return (( k % n ) + n) % n;
}
float sqr(float x) {
    return x*x;
}

float dist_sq(const float x1, const float y1, const float x2, const float y2) {
    const float r1 = x2 - x1;
    const float r2 = y2 - y1;
    return r1*r1 + r2*r2;
}
float dist_sq(const float a[2], const float b[2]) {
    const float r1 = b[0] - a[0];
    const float r2 = b[1] - a[1];
    return r1*r1 + r2*r2;
}

float find_angle(const float a[2], const float b[2]) {
    float x = std::atan2(b[1] - a[1], b[0] - a[0]) * 180 / M_PI;
    if (x < 0) {
        x = 360 + x;
	}
	return x;
}
float find_angle(const float x1, const float y1, const float x2, const float y2) {
    float x = std::atan2(y2 - y1, x2 - x1) * 180 / M_PI;
    if (x < 0) {
    	x = 360 + x;
	}
    return x;
}
//Check if 2 angles are withing n degrees of one another
bool angles_within_n(const int16_t a, const int16_t b, const int16_t n) {
    const int16_t ab_dist = std::abs(a - b);
    return (ab_dist <= n) || (360 - ab_dist <= n);
}
