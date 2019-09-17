#ifndef GUARD_DAY_NIGHT_FILTER_H
#define GUARD_DAY_NIGHT_FILTER_H

void GetDayNightFilterColor(void);
u16 MakeReflectionColor(u16 inputColor);
u16 DoDayNightFilter(u16 inputColor);
void ReloadSpritePalettes(void);

#endif // GUARD_DAY_NIGHT_FILTER_H
