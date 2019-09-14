#include "global.h"
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
		return 0x5F20;
	else
		return 0xC;
}

// Modified from BlendPalette in util.c
u16 DoDayNightFilter(u16 inputColor)
{
	u16 color = GetDayNightFilterColorOrCoeff(TRUE);
	u16 mixedColor;

	struct PlttData *data1 = (struct PlttData *)&inputColor;
	s8 r = data1->r;
	s8 g = data1->g;
	s8 b = data1->b;
	struct PlttData *data2 = (struct PlttData *)&color;
	mixedColor = ((r + (((data2->r - r) * GetDayNightFilterColorOrCoeff(FALSE)) >> 4)) << 0)
			   | ((g + (((data2->g - g) * GetDayNightFilterColorOrCoeff(FALSE)) >> 4)) << 5)
			   | ((b + (((data2->b - b) * GetDayNightFilterColorOrCoeff(FALSE)) >> 4)) << 10);
	
	return mixedColor;
}
