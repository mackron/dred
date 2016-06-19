
// This is where anything that doesn't belong to any particular category is placed.

bool dred_parse_bool(const char* value);
drgui_font_weight dred_parse_font_weight(const char* weight);
drgui_font_slant dred_parse_font_slant(const char* slant);

#ifdef DRED_GTK
drgui_font_weight dred_font_weight_from_pango(PangoWeight weight);
drgui_font_slant dred_font_slant_from_pango(PangoStyle slant);
#endif

drgui_color dred_parse_color(const char* color);