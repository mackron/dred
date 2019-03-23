// Copyright (C) 2019 David Reid. See included LICENSE file.

dtk_bool32 dred_parse_bool(const char* value)
{
    if (_stricmp(value, "false") == 0 || _stricmp(value, "0") == 0) {
        return DTK_FALSE;
    }

    return DTK_TRUE;
}

dtk_font_weight dred_parse_font_weight(const char* weight)
{
    if (strcmp(weight, "medium") == 0 || strcmp(weight, "normal") == 0 || strcmp(weight, "default") == 0) {
        return dtk_font_weight_normal;
    }
    if (strcmp(weight, "thin") == 0) {
        return dtk_font_weight_thin;
    }
    if (strcmp(weight, "extra-light") == 0) {
        return dtk_font_weight_extra_light;
    }
    if (strcmp(weight, "light") == 0) {
        return dtk_font_weight_light;
    }
    if (strcmp(weight, "semi-light") == 0) {
        return dtk_font_weight_semi_light;
    }
    if (strcmp(weight, "book") == 0) {
        return dtk_font_weight_book;
    }
    if (strcmp(weight, "semi-bold") == 0) {
        return dtk_font_weight_semi_bold;
    }
    if (strcmp(weight, "bold") == 0) {
        return dtk_font_weight_bold;
    }
    if (strcmp(weight, "extra-bold") == 0) {
        return dtk_font_weight_extra_bold;
    }
    if (strcmp(weight, "heavy") == 0) {
        return dtk_font_weight_heavy;
    }
    if (strcmp(weight, "extra-heavy") == 0) {
        return dtk_font_weight_extra_heavy;
    }

    return dtk_font_weight_normal;
}

dtk_bool32 dred_font_weight_to_string(dtk_font_weight weight, char* strOut, size_t strOutSize)
{
    if (weight == dtk_font_weight_normal/* || weight == dtk_font_weight_medium || weight == dtk_font_weight_default*/) {
        return strcpy_s(strOut, strOutSize, "default") == 0;
    }
    if (weight == dtk_font_weight_thin) {
        return strcpy_s(strOut, strOutSize, "thin") == 0;
    }
    if (weight == dtk_font_weight_extra_light) {
        return strcpy_s(strOut, strOutSize, "extra-light") == 0;
    }
    if (weight == dtk_font_weight_light) {
        return strcpy_s(strOut, strOutSize, "light") == 0;
    }
    if (weight == dtk_font_weight_semi_light) {
        return strcpy_s(strOut, strOutSize, "semi-light") == 0;
    }
    if (weight == dtk_font_weight_book) {
        return strcpy_s(strOut, strOutSize, "book") == 0;
    }
    if (weight == dtk_font_weight_semi_bold) {
        return strcpy_s(strOut, strOutSize, "semi-bold") == 0;
    }
    if (weight == dtk_font_weight_bold) {
        return strcpy_s(strOut, strOutSize, "bold") == 0;
    }
    if (weight == dtk_font_weight_extra_bold) {
        return strcpy_s(strOut, strOutSize, "extra-bold") == 0;
    }
    if (weight == dtk_font_weight_heavy) {
        return strcpy_s(strOut, strOutSize, "heavy") == 0;
    }
    if (weight == dtk_font_weight_extra_heavy) {
        return strcpy_s(strOut, strOutSize, "extra-heavy") == 0;
    }

    return DTK_FALSE;
}


dtk_font_slant dred_parse_font_slant(const char* slant)
{
    if (strcmp(slant, "none") == 0 || strcmp(slant, "default") == 0) {
        return dtk_font_slant_none;
    }
    if (strcmp(slant, "italic") == 0) {
        return dtk_font_slant_italic;
    }
    if (strcmp(slant, "oblique") == 0) {
        return dtk_font_slant_oblique;
    }

    return dtk_font_slant_none;
}

dtk_bool32 dred_font_slant_to_string(dtk_font_slant slant, char* strOut, size_t strOutSize)
{
    if (slant == dtk_font_slant_none) {
        return strcpy_s(strOut, strOutSize, "none") == 0;
    }
    if (slant == dtk_font_slant_italic) {
        return strcpy_s(strOut, strOutSize, "italic") == 0;
    }
    if (slant == dtk_font_slant_oblique) {
        return strcpy_s(strOut, strOutSize, "oblique") == 0;
    }

    return DTK_FALSE;
}


#ifdef DRED_GTK
dtk_font_weight dred_font_weight_from_pango(PangoWeight weight)
{
    return dtk_font_weight_from_pango(weight);
}

dtk_font_slant dred_font_slant_from_pango(PangoStyle slant)
{
    return dtk_font_slant_from_pango(slant);
}
#endif // DRED_GTK
