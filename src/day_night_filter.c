#include "global.h"
#include "day_night_filter.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "constants/map_types.h"
#include "palette.h"
#include "play_time.h"
#include "constants/weather.h"

// Update day/night/season colors every second
// unless a palette fade is happening or a new map is loading
void UpdateDayNightColors(void)
{
	if (!gPaletteFade.active
	 && !gPaletteFade.y) // Hacky lil workaround here
	{
		// Update BG
		apply_map_tileset1_tileset2_palette(gMapHeader.mapLayout);
		// Update OW sprites
		ReloadSpritePalettes();
		// Update weather
		UpdateWeatherPal();
	}
}

// Returns TRUE if a custom filter color or coeff has been defined in SaveBlock2
static bool8 IsCustomFilterColorOrCoeffSet(void)
{
	u8 coeff = gSaveBlock2Ptr->screenFilterCoeff;
	u16 color = gSaveBlock2Ptr->screenFilterColor;
	
	if (coeff != 0 || color != 0)
		return TRUE;
	else
		return FALSE;
}

// Returns color to be used by the filter
static u16 GetDayNightFilterColor(void)
{
	// If a custom color has been set, return that
	if (IsCustomFilterColorOrCoeffSet())
	{
		return gSaveBlock2Ptr->screenFilterColor;
	}
	// Otherwise, calculate the color
	switch (gSaveBlock2Ptr->dayNightStatus)
	{
		case TIME_DUSK:
			return COLOR_DUSK;
		case TIME_NIGHT:
			return COLOR_NIGHT;
		case TIME_DAWN:
			return COLOR_DAWN;
		default:
			return 0;
	}
}

// Returns coefficient value to be used by the filter
static u8 GetDayNightFilterCoeff(void)
{
	// If a custom coeff has been set, return that
	if (IsCustomFilterColorOrCoeffSet())
	{
		return gSaveBlock2Ptr->screenFilterCoeff;
	}
	// Otherwise, calculate the coefficient
	switch (gSaveBlock2Ptr->dayNightStatus)
	{
		case TIME_DUSK:
			return COEFF_DUSK;
		case TIME_NIGHT:
			return COEFF_NIGHT;
		case TIME_DAWN:
			return COEFF_DAWN;
		default:
			return 0;
	}
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
	u8 coeff = GetDayNightFilterCoeff();
	u16 filtColor = GetDayNightFilterColor();
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
