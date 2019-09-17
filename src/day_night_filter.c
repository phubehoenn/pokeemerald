#include "global.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "constants/map_types.h"
#include "palette.h"
#include "play_time.h"

// Returns the opacity to be used by the filter
// Returns color if retColor = TRUE, coeff if false
static u16 GetDayNightFilterColorOrCoeff(bool8 retColor)
{
	u8 coeff = gSaveBlock2Ptr->screenFilterCoeff;
	u16 color = gSaveBlock2Ptr->screenFilterColor;
	
	// If a custom color has been set, return the custom coeff instead
	if (color != 0 && coeff != 0)
	{
		if (retColor)
			return color;
		else
			return coeff;
	}
	if (retColor)
		return 0x7C08;
	else
		return 0x4;
}

// Makes color brighter if it's part of a reflection palette
static u16 TryGetReflectionColor(u16 inputColor, bool8 isReflection)
{
	u16 retColor;
	struct PlttData *data = (struct PlttData *)&inputColor;
	s8 r = data->r;
	s8 g = data->g;
	s8 b = data->b;
	
	if (isReflection)
	{
		// Brighten the color
		if (r + 3 > 31)
		r = 31;
		else
			r += 3;
		if (b + 3 > 31)
			b = 31;
		else
			b += 3;
		if (g + 3 > 31)
			g = 31;
		else
		g += 3;
	
		retColor = ((r + (((31 - r) * 4) >> 4)) << 0)
				   | ((g + (((31 - g) * 4) >> 4)) << 5)
				   | ((b + (((31 - b) * 4) >> 4)) << 10);
	}
	else
		retColor = inputColor;
	
	return retColor;
}

// Modified from BlendPalette in util.c
u16 DoDayNightFilter(u16 inputColor, bool8 isReflection)
{
	u8 coeff = GetDayNightFilterColorOrCoeff(FALSE);
	u16 filtColor = GetDayNightFilterColorOrCoeff(TRUE);
	u16 retColor = TryGetReflectionColor(inputColor, isReflection);
	
	struct PlttData *data1 = (struct PlttData *)&retColor;
	s8 r = data1->r;
	s8 g = data1->g;
	s8 b = data1->b;
	struct PlttData *data2 = (struct PlttData *)&filtColor;
	
	if ((gMapHeader.mapType == MAP_TYPE_UNDERGROUND //is player inside?
	 || gMapHeader.mapType == MAP_TYPE_UNDERWATER
	 || gMapHeader.mapType == MAP_TYPE_INDOOR
	 || gMapHeader.mapType == MAP_TYPE_SECRET_BASE)
	 && (gSaveBlock2Ptr->screenFilterCoeff == 0 //is a custom filter being applied?
	 && gSaveBlock2Ptr->screenFilterColor == 0))
	{
		if (!isReflection)
			return inputColor;
	}
	else
	{
		retColor = ((r + (((data2->r - r) * coeff) >> 4)) << 0)
				   | ((g + (((data2->g - g) * coeff) >> 4)) << 5)
				   | ((b + (((data2->b - b) * coeff) >> 4)) << 10);
	}
	
	return retColor;
}
