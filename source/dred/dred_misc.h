// Copyright (C) 2016 David Reid. See included LICENSE file.

// This is where anything that doesn't belong to any particular category is placed.

bool dred_parse_bool(const char* value);
dred_gui_font_weight dred_parse_font_weight(const char* weight);
bool dred_font_weight_to_string(dred_gui_font_weight weight, char* strOut, size_t strOutSize);

dred_gui_font_slant dred_parse_font_slant(const char* slant);
bool dred_font_slant_to_string(dred_gui_font_slant slant, char* strOut, size_t strOutSize);

#ifdef DRED_GTK
dred_gui_font_weight dred_font_weight_from_pango(PangoWeight weight);
dred_gui_font_slant dred_font_slant_from_pango(PangoStyle slant);
#endif

dred_color dred_parse_color(const char* color);



// Type conversion

static DRED_INLINE dred_rect drte_rect_to_drgui(drte_rect rect)
{
    return dred_make_rect(rect.left, rect.top, rect.right, rect.bottom);
}

static DRED_INLINE drte_rect dred_rect_to_drte(dred_rect rect)
{
    return drte_make_rect(rect.left, rect.top, rect.right, rect.bottom);
}