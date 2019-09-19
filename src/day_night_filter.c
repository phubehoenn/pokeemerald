#include "global.h"
#include "day_night_filter.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "constants/map_types.h"
#include "palette.h"
#include "play_time.h"
#include "constants/weather.h"

#define COLOR_NIGHT 0x7C08
#define COEFF_NIGHT 64

// Yeesh. I really tried to do this more efficiently
const u8 gPreDuskCoeffs[180] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20 };
const u8 gDusk1Coeffs[180] = { 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40 };
const u8 gDusk2Coeffs[180] = { 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63, 64, 64, 64 };
const u8 gPreDawnCoeffs[180] = { 64, 64, 64, 63, 63, 63, 63, 63, 63, 63, 63, 62, 62, 62, 62, 62, 62, 62, 61, 61, 61, 61, 61, 61, 61, 61, 60, 60, 60, 60, 60, 60, 60, 59, 59, 59, 59, 59, 59, 59, 59, 58, 58, 58, 58, 58, 58, 58, 57, 57, 57, 57, 57, 57, 57, 57, 56, 56, 56, 56, 56, 56, 56, 55, 55, 55, 55, 55, 55, 55, 55, 54, 54, 54, 54, 54, 54, 54, 53, 53, 53, 53, 53, 53, 53, 53, 52, 52, 52, 52, 52, 52, 52, 51, 51, 51, 51, 51, 51, 51, 51, 50, 50, 50, 50, 50, 50, 50, 49, 49, 49, 49, 49, 49, 49, 49, 48, 48, 48, 48, 48, 48, 48, 47, 47, 47, 47, 47, 47, 47, 47, 46, 46, 46, 46, 46, 46, 46, 45, 45, 45, 45, 45, 45, 45, 45, 44, 44, 44, 44, 44, 44, 44, 43, 43, 43, 43, 43, 43, 43, 43, 42, 42, 42, 42, 42, 42, 42, 41, 41, 41, 41, 41, 41, 41, 41, 40, 40, 40, 40 };
const u8 gDawn1Coeffs[180] = { 40, 40, 40, 40, 39, 39, 39, 39, 39, 39, 39, 39, 39, 38, 38, 38, 38, 38, 38, 38, 38, 38, 37, 37, 37, 37, 37, 37, 37, 37, 37, 36, 36, 36, 36, 36, 36, 36, 36, 36, 35, 35, 35, 35, 35, 35, 35, 35, 35, 34, 34, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 26, 26, 26, 26, 26, 26, 26, 26, 26, 25, 25, 25, 25, 25, 25, 25, 25, 25, 24, 24, 24, 24, 24, 24, 24, 24, 24, 23, 23, 23, 23, 23, 23, 23, 23, 23, 22, 22, 22, 22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20 };
const u8 gDawn2Coeffs[180] = { 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

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

static u16 GetFilterStage()
{
	// Get clock seconds and minute
	u8 seconds = gSaveBlock2Ptr->timeSeconds;
	u8 minute = gSaveBlock2Ptr->timeMinute * 60;
	// Add minute * 60 and seconds together to get a total time
	u8 time = minute + seconds;
	
	return time;
}

// Returns color to be used by the filter
// Add cave lighting colors here too eventually
static u16 GetDayNightFilterColor(void)
{
	// If a custom color has been set, return that
	if (IsCustomFilterColorOrCoeffSet())
		return gSaveBlock2Ptr->screenFilterColor;
	else
		return COLOR_NIGHT;
}

// Returns coefficient value to be used by the filter
static u8 GetDayNightFilterCoeff(void)
{
	// Get the current hour
	u8 hour = gSaveBlock2Ptr->timeHour;
	
	// If a custom coeff has been set, return that
	if (IsCustomFilterColorOrCoeffSet())
	{
		return gSaveBlock2Ptr->screenFilterCoeff;
	}
	
	// Is it the last hour of day?
	if (hour == GetSunsetTime() - 2)
		return gPreDuskCoeffs[GetFilterStage()];
	// Is it the first hour of dusk?
	else if (hour == GetSunsetTime() - 1)
		return gDusk1Coeffs[GetFilterStage()];
	// Is it the second hour of dusk?
	else if (hour == GetSunsetTime())
		return gDusk2Coeffs[GetFilterStage()];
	// Is it the last hour of night?
	else if (hour == GetSunriseTime() - 1)
		return gPreDawnCoeffs[GetFilterStage()];
	// Is it the first hour of dawn?
	else if (hour == GetSunriseTime())
		return gDawn1Coeffs[GetFilterStage()];
	// Is it the second hour of dawn?
	else if (hour == GetSunriseTime() + 1)
		return gDawn2Coeffs[GetFilterStage()];
	// Is it night?
	else if (gSaveBlock2Ptr->dayNightStatus == TIME_NIGHT)
	{
		return COEFF_NIGHT;
	}
	// It must be day - return 0 coeff
	else
	{
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
