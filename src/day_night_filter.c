#include "global.h"
#include "field_effect_helpers.h"
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
static u16 TryGetReflectionColor(u16 inputColor, u8 mode, u8 colorNum)
{
	u16 retColor;
	struct PlttData *data = (struct PlttData *)&inputColor;
	s8 r = data->r;
	s8 g = data->g;
	s8 b = data->b;
	
	// If day/night filter mode is set to reflections
	if (mode > 1)
	{
		if (colorNum == 0 && inputColor != 0x1234)
		{
			// Make tag for transparent color
			retColor = GetReflectionTag();
		}
		else
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
	}
	else
		retColor = inputColor;
	
	return retColor;
}

// Modified from BlendPalette in util.c
u16 DoDayNightFilter(u16 inputColor, u8 mode, u8 colorNum)
{
	u8 coeff = GetDayNightFilterColorOrCoeff(FALSE);
	u16 filtColor = GetDayNightFilterColorOrCoeff(TRUE);
	u16 retColor = TryGetReflectionColor(inputColor, mode, colorNum); // Stores base reflection color if applicable
	
	struct PlttData *data1 = (struct PlttData *)&retColor;
	s8 r = data1->r;
	s8 g = data1->g;
	s8 b = data1->b;
	struct PlttData *data2 = (struct PlttData *)&filtColor;
	
	if (colorNum == 0)
		return retColor;
	
	if ((gMapHeader.mapType == MAP_TYPE_UNDERGROUND //is player inside?
	 || gMapHeader.mapType == MAP_TYPE_UNDERWATER
	 || gMapHeader.mapType == MAP_TYPE_INDOOR
	 || gMapHeader.mapType == MAP_TYPE_SECRET_BASE)
	 && (gSaveBlock2Ptr->screenFilterCoeff == 0 //is a custom filter being applied?
	 && gSaveBlock2Ptr->screenFilterColor == 0))
	{
		// It's a standard overworld sprite, return unfiltered color
		if (mode == 0)
			return inputColor;
	}
	else
	{
		// it's a bg palette, a reflection or a field effect, so apply filter
		retColor = ((r + (((data2->r - r) * coeff) >> 4)) << 0)
				   | ((g + (((data2->g - g) * coeff) >> 4)) << 5)
				   | ((b + (((data2->b - b) * coeff) >> 4)) << 10);
	}
	
	return retColor;
}

// Reloads overworld sprite palettes (except for weather)
void ReloadSpritePalettes(void)
{
	LoadPaletteWithDayNightFilter(gPlttBufferUnfaded + 0x100, *gPlttBufferFaded + 0x100, 15, 0, FALSE);
}
