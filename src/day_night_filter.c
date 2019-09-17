#include "global.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "constants/map_types.h"
#include "palette.h"
#include "play_time.h"

static u16 TryGetReflectionColor(u16 inputColor, u8 mode, u8 colorNum);
static u16 DoReflectionColor(struct PlttData *data);
static void ReloadReflectionPalette(u8 index);
static u16 GetReflectionTag(void);

// Makes color brighter if it's part of a reflection palette
static u16 TryGetReflectionColor(u16 inputColor, u8 mode, u8 colorNum)
{
	u16 retColor;
	struct PlttData *data = (struct PlttData *)&inputColor;
	
	// Detects a reflection palette tag
	if (inputColor >= 0x1230 && inputColor < 0x1250 && colorNum == 0)
	{
		int i;
		u16 tag;

		// Loop through OW palettes
		for (i = 0; i < 16; i++)
		{
			// Copy the transparent color
			CpuCopy16(&gPlttBufferUnfaded[i * 16 + 0x100], &tag, 2);
			
			// If tag exists, recalculate its reflection palette
			if (tag == inputColor)
			{
				ReloadReflectionPalette(i);
				return;
			}
		}
	}
	// Palette needs a tag
	else if (colorNum == 0)
	{
		retColor = GetReflectionTag();
	}
	// If day/night filter mode is set to reflections and if color isn't transparent
	else if (mode > 1 && colorNum > 0)
	{
		// Brighten the color
		retColor = DoReflectionColor(data);
	}
	else
		retColor = inputColor;
	
	return retColor;
}
	
// Lightens a color based on color provided
static u16 DoReflectionColor(struct PlttData *data)
{
	u16 retColor;
	s8 r = data->r;
	s8 g = data->g;
	s8 b = data->b;
	
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

// Reloads a whole reflection palette at once
static void ReloadReflectionPalette(u8 index)
{
	int i;
	u16 color;
	struct PlttData *data;
	s8 r, g, b;
	
	for (i = 1; i < 16; i++)
	{
		CpuCopy16(gPlttBufferUnfaded + 0x100 + (index * 16) + i, &color, 2);
		data = (struct PlttData *)&color;
		DoReflectionColor(data);
		CpuFill16(color, gPlttBufferFaded + 0x100 + (index * 16) + i, 2);
	}
}

// Finds the next free tag to be used as the transparent color in palettes that have reflections
static u16 GetReflectionTag(void)
{
	int i;
	u16 color;
	u16 tag = 0x1230; // Tags start at 0x1230
	
	// Loop through OW palettes
	for (i = 0; i < 16; i++)
	{
		// Copy the transparent color
		CpuCopy16(&gPlttBufferFaded[i * 16 + 0x100], &color, 2);
		
		// If tag exists, try the next one and reset loop
		if (color == tag)
		{
			tag++;
			i = 0;
		}
	}
	return tag;
}

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

// Reloads overworld sprite palettes (except for weather & palettes that aren't affected)
void ReloadSpritePalettes(void)
{
	int i;
	
	for (i = 0; i < 15; i++)
	{
		LoadPaletteWithDayNightFilter(gPlttBufferUnfaded + 0x100 + (i * 16), *gPlttBufferFaded + 0x100 + (i * 16), 1, 0, FALSE);
	}
}
