
typedef struct
{
    drgui_element* pInternalTB;
} dred_textbox_data;


void dred_textbox__on_capture_keyboard(dred_textbox* pTextBox, drgui_element* pPrevCapturedElement)
{
    (void*)pPrevCapturedElement;

    // The internal text box needs to be given the keyboard focus whenever a dred_textbox receives it.
    dred_textbox_data* data = dred_control_get_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_capture_keyboard(data->pInternalTB);
}


void dred_textbox_innertb__on_key_down(drgui_element* pInternalTB, drgui_key key, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onKeyDown) {
        pTextBox->onKeyDown(pTextBox, key, stateFlags);
    } else {
        drgui_textbox_on_key_down(pInternalTB, key, stateFlags);
    }
}

void dred_textbox_innertb__on_key_up(drgui_element* pInternalTB, drgui_key key, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onKeyUp) {
        pTextBox->onKeyUp(pTextBox, key, stateFlags);
    } else {
        drgui_textbox_on_key_up(pInternalTB, key, stateFlags);
    }
}

void dred_textbox_innertb__on_printable_key_down(drgui_element* pInternalTB, uint32_t utf32, int stateFlags)
{
    (void)stateFlags;

    dred_textbox* pTextBox = dred_control_get_parent(pInternalTB);
    assert(pTextBox != NULL);

    if (pTextBox->onPrintableKeyDown) {
        pTextBox->onPrintableKeyDown(pTextBox, utf32, stateFlags);
    } else {
        drgui_textbox_on_printable_key_down(pInternalTB, utf32, stateFlags);
    }
}


dred_textbox* dred_textbox_create(dred_context* pDred, dred_control* pParent)
{
    dred_textbox* pTextBox = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TEXTBOX, sizeof(dred_textbox_data));
    if (pTextBox == NULL) {
        return NULL;
    }

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_data(pTextBox);
    assert(data != NULL);

    data->pInternalTB = drgui_create_textbox(pDred->pGUI, pTextBox, 0, NULL);
    if (data->pInternalTB == NULL) {
        dred_control_delete(pTextBox);
        return NULL;
    }

    drgui_textbox_set_background_color(data->pInternalTB, drgui_rgb(64, 64, 64));
    drgui_textbox_set_active_line_background_color(data->pInternalTB, drgui_rgb(64, 64, 64));
    drgui_textbox_set_text_color(data->pInternalTB, drgui_rgb(224, 224, 224));
    drgui_textbox_set_cursor_color(data->pInternalTB, drgui_rgb(224, 224, 224));
    drgui_textbox_set_font(data->pInternalTB, drgui_create_font(pDred->pGUI, "Courier New", 13, drgui_font_weight_default, drgui_font_slant_none, 0, 0));
    drgui_textbox_set_border_width(data->pInternalTB, 0);
    drgui_textbox_disable_horizontal_scrollbar(data->pInternalTB);
    drgui_textbox_disable_vertical_scrollbar(data->pInternalTB);

    // Events.
    dred_control_set_on_size(pTextBox, drgui_on_size_fit_children_to_parent);
    dred_control_set_on_capture_keyboard(pTextBox, dred_textbox__on_capture_keyboard);

    // Internal text box overrides.
    drgui_set_on_key_down(data->pInternalTB, dred_textbox_innertb__on_key_down);
    drgui_set_on_key_up(data->pInternalTB, dred_textbox_innertb__on_key_up);
    drgui_set_on_printable_key_down(data->pInternalTB, dred_textbox_innertb__on_printable_key_down);

    return pTextBox;
}

void dred_textbox_delete(dred_textbox* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox_data* data = (dred_textbox_data*)dred_control_get_data(pTextBox);
    if (data != NULL) {
        drgui_delete_textbox(data->pInternalTB);
    }

    dred_control_delete(pTextBox);
}


void dred_textbox_set_text(dred_textbox* pTextBox, const char* text)
{
    dred_textbox_data* data = dred_control_get_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_set_text(data->pInternalTB, text);
}

size_t dred_textbox_get_text(dred_textbox* pTextBox, char* textOut, size_t textOutSize)
{
    dred_textbox_data* data = dred_control_get_data(pTextBox);
    if (data == NULL) {
        return 0;
    }

    return drgui_textbox_get_text(data->pInternalTB, textOut, textOutSize);
}


void dred_textbox_on_key_down(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_key_down(data->pInternalTB, key, stateFlags);
}

void dred_textbox_on_key_up(dred_textbox* pTextBox, drgui_key key, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_key_up(data->pInternalTB, key, stateFlags);
}

void dred_textbox_on_printable_key_down(dred_textbox* pTextBox, uint32_t utf32, int stateFlags)
{
    dred_textbox_data* data = dred_control_get_data(pTextBox);
    if (data == NULL) {
        return;
    }

    drgui_textbox_on_printable_key_down(data->pInternalTB, utf32, stateFlags);
}
