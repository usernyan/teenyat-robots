#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>

struct color_t {
	uint8_t r {0};
	uint8_t g {0};
	uint8_t b {0};
	uint8_t a {0};
};

uint32_t color_to_int(struct color_t c);
struct color_t hsv_to_color(double hue, double sat, double val);

float sin_360(const int x);
float cos_360(const int x);
float rad_360(const int x);
int mod(const int k, const int n);
float sqr(const float x);
float dist_sq(const float x1, const float y1, const float x2, const float y2);
float dist_sq(const float a[2], const float b[2]);
float find_angle(const float a[2], const float b[2]);
float find_angle(const float x1, const float y1, const float x2, const float y2);
bool angles_within_n(const int a, const int b, const int n);

#endif
