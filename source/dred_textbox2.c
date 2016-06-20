
typedef struct
{
    // The text engine. This is where the real work is done for a text box.
    drte_engine engine;

} dred_textbox2_data;

DRTE_INLINE drte_rect drgui_to_drte_rect(drgui_rect rect)
{
    return drte_make_rect(rect.left, rect.top, rect.right, rect.bottom);
}

DRTE_INLINE drgui_rect drte_to_drgui_rect(drte_rect rect)
{
    return drgui_make_rect(rect.left, rect.top, rect.right, rect.bottom);
}

DRTE_INLINE drte_engine* dred_textbox2__get_engine(dred_textbox2* pTextBox)
{
    dred_textbox2_data* pData = (dred_textbox2_data*)dred_control_get_extra_data(pTextBox);
    assert(pData != NULL);

    return &pData->engine;
}

void dred_textbox2_engine__on_dirty(drte_engine* pEngine, drte_rect rect)
{
    dred_textbox2* pTextBox = (dred_textbox2*)pEngine->pUserData;
    assert(pTextBox != NULL);

    drgui_dirty(pTextBox, drte_to_drgui_rect(rect));
}

void dred_textbox2_engine__on_paint_rect(drte_engine* pEngine, drte_rect rect, drte_style_id styleID, void* pPaintData)
{
    dred_textbox2* pTextBox = (dred_textbox2*)pEngine->pUserData;
    assert(pTextBox != NULL);

    dred_context* pDred = dred_control_get_context(pTextBox);
    assert(pDred != NULL);

    drgui_draw_rect(pTextBox, drte_to_drgui_rect(rect), pDred->config.textEditorBGColor, pPaintData);
}

void dred_textbox2__on_paint(dred_textbox2* pTextBox, drgui_rect rect, void* pPaintData)
{
    // To paint the textbox all we need to do is paint the text engine. That will in turn call some painting callbacks which is where the
    // real work is done.
    drte_engine_paint(dred_textbox2__get_engine(pTextBox), drgui_to_drte_rect(rect), pPaintData);
}

void dred_textbox2__on_size(dred_textbox2* pTextBox, float newWidth, float newHeight)
{
    drte_engine_set_bounds(dred_textbox2__get_engine(pTextBox), newWidth, newHeight);
}


dred_textbox2* dred_textbox2_create(dred_context* pDred, dred_control* pParent)
{
    dred_textbox2* pTextBox = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_TEXTBOX2, sizeof(dred_textbox2_data));
    if (pTextBox == NULL) {
        return NULL;
    }

    dred_textbox2_data* pData = (dred_textbox2_data*)dred_control_get_extra_data(pTextBox);
    assert(pData != NULL);

    if (!drte_engine_init(&pData->engine)) {
        dred_control_delete(pTextBox);
        return NULL;
    }

    pData->engine.pUserData = pTextBox;
    pData->engine.onDirty = dred_textbox2_engine__on_dirty;
    pData->engine.onPaintRect = dred_textbox2_engine__on_paint_rect;
    pData->engine.boundsSizeX = dred_control_get_width(pTextBox);
    pData->engine.boundsSizeY = dred_control_get_height(pTextBox);
    

    drgui_set_cursor(pTextBox, drgui_cursor_text);


    // Events.
    dred_control_set_on_paint(pTextBox, dred_textbox2__on_paint);
    dred_control_set_on_size(pTextBox, dred_textbox2__on_size);

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


void dred_textbox2_set_text(dred_textbox2* pTextBox, const char* text)
{
    drte_engine_set_text(dred_textbox2__get_engine(pTextBox), text);
}