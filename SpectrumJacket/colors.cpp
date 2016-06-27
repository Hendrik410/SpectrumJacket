#include "colors.h"

RGBColor ColorUtil::HSL2RGB(HSLColor hsl) {
	RGBColor out;
	unsigned int var1, var2;

	if (hsl.hue > 359) hsl.hue = hsl.hue % 360;
	if (hsl.saturation > 100) hsl.saturation = 100;
	if (hsl.lightness > 100) hsl.lightness = 100;

	// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
	if (hsl.saturation == 0) {
		out.red = out.green = out.blue = hsl.lightness * 255 / 100;
	}
	else {
		if (hsl.lightness < 50) {
			var2 = hsl.lightness * (100 + hsl.saturation);
		}
		else {
			var2 = ((hsl.lightness + hsl.saturation) * 100) - (hsl.saturation * hsl.lightness);
		}
		var1 = hsl.lightness * 200 - var2;
		out.red = h2rgb(var1, var2, (hsl.hue < 240) ? hsl.hue + 120 : hsl.hue - 240) * 255 / 600000;
		out.green = h2rgb(var1, var2, hsl.hue) * 255 / 600000;
		out.blue = h2rgb(var1, var2, (hsl.hue >= 120) ? hsl.hue - 120 : hsl.hue + 240) * 255 / 600000;
	}
	return out;
}

unsigned int ColorUtil::h2rgb(unsigned int v1, unsigned int v2, unsigned int hue) {
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}