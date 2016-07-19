// Copyright (C) 2016 David Reid. See included LICENSE file.

// This is where anything that doesn't belong to any particular category is placed.

bool dred_parse_bool(const char* value);
drgui_font_weight dred_parse_font_weight(const char* weight);
bool dred_font_weight_to_string(drgui_font_weight weight, char* strOut, size_t strOutSize);

drgui_font_slant dred_parse_font_slant(const char* slant);
bool dred_font_slant_to_string(drgui_font_slant slant, char* strOut, size_t strOutSize);

#ifdef DRED_GTK
drgui_font_weight dred_font_weight_from_pango(PangoWeight weight);
drgui_font_slant dred_font_slant_from_pango(PangoStyle slant);
#endif

drgui_color dred_parse_color(const char* color);