
bool dred_parse_bool(const char* value)
{
    if (_stricmp(value, "false") == 0 || _stricmp(value, "0") == 0) {
        return false;
    }

    return true;
}

drgui_font_weight dred_parse_font_weight(const char* weight)
{
    if (strcmp(weight, "medium") == 0 || strcmp(weight, "normal") == 0 || strcmp(weight, "default") == 0) {
        return drgui_font_weight_normal;
    }
    if (strcmp(weight, "thin") == 0) {
        return drgui_font_weight_thin;
    }
    if (strcmp(weight, "extra-light") == 0) {
        return drgui_font_weight_extra_light;
    }
    if (strcmp(weight, "light") == 0) {
        return drgui_font_weight_light;
    }
    if (strcmp(weight, "semi-light") == 0) {
        return drgui_font_weight_semi_light;
    }
    if (strcmp(weight, "book") == 0) {
        return drgui_font_weight_book;
    }
    if (strcmp(weight, "semi-bold") == 0) {
        return drgui_font_weight_semi_bold;
    }
    if (strcmp(weight, "bold") == 0) {
        return drgui_font_weight_bold;
    }
    if (strcmp(weight, "extra-bold") == 0) {
        return drgui_font_weight_extra_bold;
    }
    if (strcmp(weight, "heavy") == 0) {
        return drgui_font_weight_heavy;
    }
    if (strcmp(weight, "extra-heavy") == 0) {
        return drgui_font_weight_extra_heavy;
    }

    return drgui_font_weight_normal;
}

bool dred_font_weight_to_string(drgui_font_weight weight, char* strOut, size_t strOutSize)
{
    if (weight == drgui_font_weight_normal/* || weight == drgui_font_weight_medium || weight == drgui_font_weight_default*/) {
        return strcpy_s(strOut, strOutSize, "default") == 0;
    }
    if (weight == drgui_font_weight_thin) {
        return strcpy_s(strOut, strOutSize, "thin") == 0;
    }
    if (weight == drgui_font_weight_extra_light) {
        return strcpy_s(strOut, strOutSize, "extra-light") == 0;
    }
    if (weight == drgui_font_weight_light) {
        return strcpy_s(strOut, strOutSize, "light") == 0;
    }
    if (weight == drgui_font_weight_semi_light) {
        return strcpy_s(strOut, strOutSize, "semi-light") == 0;
    }
    if (weight == drgui_font_weight_book) {
        return strcpy_s(strOut, strOutSize, "book") == 0;
    }
    if (weight == drgui_font_weight_semi_bold) {
        return strcpy_s(strOut, strOutSize, "semi-bold") == 0;
    }
    if (weight == drgui_font_weight_bold) {
        return strcpy_s(strOut, strOutSize, "bold") == 0;
    }
    if (weight == drgui_font_weight_extra_bold) {
        return strcpy_s(strOut, strOutSize, "extra-bold") == 0;
    }
    if (weight == drgui_font_weight_heavy) {
        return strcpy_s(strOut, strOutSize, "heavy") == 0;
    }
    if (weight == drgui_font_weight_extra_heavy) {
        return strcpy_s(strOut, strOutSize, "extra-heavy") == 0;
    }

    return false;
}


drgui_font_slant dred_parse_font_slant(const char* slant)
{
    if (strcmp(slant, "none") == 0 || strcmp(slant, "default") == 0) {
        return drgui_font_slant_none;
    }
    if (strcmp(slant, "italic") == 0) {
        return drgui_font_slant_italic;
    }
    if (strcmp(slant, "oblique") == 0) {
        return drgui_font_slant_oblique;
    }

    return drgui_font_slant_none;
}

bool dred_font_slant_to_string(drgui_font_slant slant, char* strOut, size_t strOutSize)
{
    if (slant == drgui_font_slant_none) {
        return strcpy_s(strOut, strOutSize, "none") == 0;
    }
    if (slant == drgui_font_slant_italic) {
        return strcpy_s(strOut, strOutSize, "italic") == 0;
    }
    if (slant == drgui_font_slant_oblique) {
        return strcpy_s(strOut, strOutSize, "oblique") == 0;
    }

    return false;
}


#ifdef DRED_GTK
drgui_font_weight dred_font_weight_from_pango(PangoWeight weight)
{
    if (weight == PANGO_WEIGHT_THIN) {
        return drgui_font_weight_thin;
    } else if (weight == PANGO_WEIGHT_ULTRALIGHT) {
        return drgui_font_weight_extra_light;
    } else if (weight == PANGO_WEIGHT_LIGHT) {
        return drgui_font_weight_light;
    } else if (weight == PANGO_WEIGHT_SEMILIGHT) {
        return drgui_font_weight_semi_light;
    } else if (weight == PANGO_WEIGHT_BOOK) {
        return drgui_font_weight_book;
    } else if (weight == PANGO_WEIGHT_NORMAL) {
        return drgui_font_weight_normal;
    } else if (weight == PANGO_WEIGHT_MEDIUM) {
        return drgui_font_weight_medium;
    } else if (weight == PANGO_WEIGHT_SEMIBOLD) {
        return drgui_font_weight_semi_bold;
    } else if (weight == PANGO_WEIGHT_BOLD) {
        return drgui_font_weight_bold;
    } else if (weight == PANGO_WEIGHT_ULTRABOLD) {
        return drgui_font_weight_extra_bold;
    } else if (weight == PANGO_WEIGHT_HEAVY) {
        return drgui_font_weight_heavy;
    } else if (weight == PANGO_WEIGHT_ULTRAHEAVY) {
        return drgui_font_weight_extra_heavy;
    } else {
        return drgui_font_weight_normal;
    }
}

drgui_font_slant dred_font_slant_from_pango(PangoStyle slant)
{
    if (slant == PANGO_STYLE_OBLIQUE) {
        return drgui_font_slant_oblique;
    } else if (slant == PANGO_STYLE_ITALIC) {
        return drgui_font_slant_italic;
    } else {
        return drgui_font_slant_none;
    }
}
#endif // DRED_GTK

drgui_color dred_parse_color(const char* color)
{
    if (color != NULL) {
        color = dr_first_non_whitespace(color);
        if (color[0] == '0' && (color[1] == 'x' || color[1] == 'X')) {
            // HTML style. Support both #RRGGBB and #RGB format.
            color += 2;

            unsigned int hexvals[6];

            int len = 0;
            for (int i = 0; i < 6; ++i) {
                if (!dr_hex_char_to_uint(color[i], &hexvals[i])) {
                    break;
                }
                len += 1;
            }

            drgui_byte r = 0;
            drgui_byte g = 0;
            drgui_byte b = 0;

            if (len == 3) {
                // #RGB -> #RRGGBB
                hexvals[5] = hexvals[2]; hexvals[4] = hexvals[2];
                hexvals[3] = hexvals[1]; hexvals[2] = hexvals[1];
                hexvals[1] = hexvals[0]; hexvals[0] = hexvals[0];
            }

            r = (drgui_byte)((hexvals[0] << 4) | hexvals[1]);
            g = (drgui_byte)((hexvals[2] << 4) | hexvals[3]);
            b = (drgui_byte)((hexvals[4] << 4) | hexvals[5]);

            return drgui_rgb(r, g, b);
        } else {
            // R G B style (0 .. 255 per component)
            drgui_byte r = 0;
            drgui_byte g = 0;
            drgui_byte b = 0;

            char c[4];
            color = dr_next_token(color, c, sizeof(c));
            if (color != NULL) {
                r = (drgui_byte)atoi(c);
            }

            color = dr_next_token(color, c, sizeof(c));
            if (color != NULL) {
                g = (drgui_byte)atoi(c);
            }

            color = dr_next_token(color, c, sizeof(c));
            if (color != NULL) {
                b = (drgui_byte)atoi(c);
            }

            return drgui_rgb(r, g, b);
        }
    }

    return drgui_rgb(0, 0, 0);
}