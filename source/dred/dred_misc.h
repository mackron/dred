// Copyright (C) 2019 David Reid. See included LICENSE file.

// This is where anything that doesn't belong to any particular category is placed.

dtk_bool32 dred_parse_bool(const char* value);
dtk_font_weight dred_parse_font_weight(const char* weight);
dtk_bool32 dred_font_weight_to_string(dtk_font_weight weight, char* strOut, size_t strOutSize);

dtk_font_slant dred_parse_font_slant(const char* slant);
dtk_bool32 dred_font_slant_to_string(dtk_font_slant slant, char* strOut, size_t strOutSize);

#ifdef DRED_GTK
dtk_font_weight dred_font_weight_from_pango(PangoWeight weight);
dtk_font_slant dred_font_slant_from_pango(PangoStyle slant);
#endif

dtk_color dred_parse_color(const char* color);



// Type conversion

DTK_INLINE dred_rect drte_rect_to_dred(drte_rect rect)
{
    return dred_make_rect(rect.left, rect.top, rect.right, rect.bottom);
}

DTK_INLINE drte_rect dred_rect_to_drte(dred_rect rect)
{
    return drte_make_rect(rect.left, rect.top, rect.right, rect.bottom);
}


// Strings

// Determines whether or not the given string is null or empty.
DTK_INLINE dtk_bool32 dred_string_is_null_or_empty(const char* str)
{
    return str == NULL || str[0] == '\0';
}