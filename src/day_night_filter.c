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
		return 0x40;
}

// Makes color brighter if it's part of a reflection palette
u16 MakeReflectionColor(u16 inputColor)
{
	u16 retColor;
	struct PlttData *data = (struct PlttData *)&inputColor;
	s8 r = data->r;
	s8 g = data->g;
	s8 b = data->b;
	
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
	
	return retColor;
}

// Modified from BlendPalette in util.c
u16 DoDayNightFilter(u16 inputColor)
{
	u8 coeff = GetDayNightFilterColorOrCoeff(FALSE);
	u16 filtColor = GetDayNightFilterColorOrCoeff(TRUE);
	u16 retColor;
	
	struct PlttData *data1 = (struct PlttData *)&inputColor;
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
		return inputColor;
	}
	else
	{
		retColor = ((r + (((data2->r - r) * (coeff / 0x10)) >> 4)) << 0)
				   | ((g + (((data2->g - g) * (coeff / 0x10)) >> 4)) << 5)
				   | ((b + (((data2->b - b) * (coeff / 0x10)) >> 4)) << 10);
	}
	
	return retColor;
}

// Reloads reflection and overworld sprite palettes (if applicable)
void ReloadSpritePalettes(void)
{
	// i = palette, j = color
	int i, j;
	bool8 doFilter = FALSE;
	u16 color;
	
	// Loop through sprite palettes
	// Weather palette (pal 16) is reloaded seperately
	for (i = 0; i < 15; i++)
	{
		// Loop through palette colors
		for (j = 0; j < 16; j++)
		{
			// Copy color
			CpuCopy16(gPlttBufferUnfaded + (i * 16) + j + 0x100, &color, 2);
			
			// Is it the transparent color?
			if (j == 0)
			{
				// Is it tagged as a reflection palette?
				if (color == 0x1234)
					// Mark the rest of the palette to be filtered if it is
					doFilter = TRUE;
			}
			// It's a visible color
			else
			{
				// Filter color if palette is marked as such
				if (doFilter)
				{
					color = DoDayNightFilter(color);
					CpuFill16(color, gPlttBufferFaded + (i * 16) + j + 0x100, 2);
				}
			}
		}
		
		// Reset doFilter after every palette
		doFilter = FALSE;
	}
}
