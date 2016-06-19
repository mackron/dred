
typedef struct
{
    int unused;
} dred_textbox2_data;

void dred_textbox2__on_paint(dred_textbox2* pTextBox, drgui_rect rect, void* pPaintData)
{
    assert(pTextBox != NULL);
    (void)rect;

    dred_context* pDred = dred_control_get_context(pTextBox);
    assert(pDred != NULL);

    


    //drgui_draw_rect_with_outline(pTextBox, drgui_get_local_rect(pTextBox), pDred->config.textEditorBGColor, 1, drgui_rgb(16, 16, 16), pPaintData);
    drgui_draw_rect(pTextBox, drgui_get_local_rect(pTextBox), pDred->config.textEditorBGColor, pPaintData);
}

dred_textbox2* dred_textbox2_create(dred_context* pDred, dred_control* pParent)
{
    dred_textbox2* pTextBox = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TEXTBOX2, sizeof(dred_textbox2_data));
    if (pTextBox == NULL) {
        return NULL;
    }

    dred_textbox2_data* pData = (dred_textbox2_data*)dred_control_get_extra_data(pTextBox);
    assert(pData != NULL);


    drgui_set_cursor(pTextBox, drgui_cursor_text);


    // Events.
    dred_control_set_on_paint(pTextBox, dred_textbox2__on_paint);
    

    return pTextBox;
}

void dred_textbox2_delete(dred_textbox2* pTextBox)
{
    if (pTextBox == NULL) {
        return;
    }

    dred_textbox2_data* pData = (dred_textbox2_data*)dred_control_get_extra_data(pTextBox);
    if (pData != NULL) {
        // Keyboard focus needs to be released first. If we don't do this we'll not delete the internal timer.
        if (drgui_has_keyboard_capture(pTextBox)) {
            drgui_release_keyboard(pTextBox->pContext);
        }
    }

    dred_control_delete(pTextBox);
}