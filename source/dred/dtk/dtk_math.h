// Copyright (C) 2019 David Reid. See included LICENSE file.

#define DTK_PI       3.14159265358979323846
#define DTK_PIF      3.14159265358979323846f

#define dtk_degrees(radians)  ((double)((radians) * 57.29577951308232087685))
#define dtk_degreesf(radians) ( (float)((radians) * 57.29577951308232087685f))
#define dtk_radians(degrees)  ((double)((degrees) *  0.01745329251994329577))
#define dtk_radiansf(degrees) ( (float)((degrees) *  0.01745329251994329577f))

DTK_INLINE dtk_uint32 dtk_next_power_of_2(dtk_uint32 value)
{
    --value;
    value = (value >> 1)  | value;
    value = (value >> 2)  | value;
    value = (value >> 4)  | value;
    value = (value >> 8)  | value;
    value = (value >> 16) | value;
    return value + 1;
}
