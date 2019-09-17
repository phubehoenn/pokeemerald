#ifndef GUARD_DAY_NIGHT_FILTER_H
#define GUARD_DAY_NIGHT_FILTER_H

void UpdateDayNightColors(void);
u16 MakeReflectionColor(u16 inputColor);
u16 DoDayNightFilter(u16 inputColor);
void ReloadSpritePalettes(void);

#define COLOR_DUSK 0x7818
#define COEFF_DUSK 0x28
#define COLOR_NIGHT 0x7C08
#define COEFF_NIGHT 0x40
#define COLOR_DAWN 0x7E40
#define COEFF_DAWN 0x28

#endif // GUARD_DAY_NIGHT_FILTER_H
