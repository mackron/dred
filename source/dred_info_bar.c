
#define DRED_INFO_BAR_TYPE_NONE             0
#define DRED_INFO_BAR_TYPE_TEXT_EDITOR      1
#define DRED_INFO_BAR_TYPE_HEX_EDITOR       2
#define DRED_INFO_BAR_TYPE_IMAGE_EDITOR     3
#define DRED_INFO_BAR_TYPE_ICON_EDITOR      4
#define DRED_INFO_BAR_TYPE_PARTICLE_EDITOR  5

typedef struct
{
    int type;

    char lineStr[32];
    char colStr[32];
} dred_info_bar_data;

void dred_info_bar__on_paint__none(dred_info_bar* pInfoBar, dred_info_bar_data* data, void* pPaintData)
{
    (void)data;
    drgui_draw_rect(pInfoBar, drgui_get_local_rect(pInfoBar), drgui_rgb(64, 64, 64), pPaintData);
}

void dred_info_bar__on_paint__text_editor(dred_info_bar* pInfoBar, dred_info_bar_data* data, void* pPaintData)
{
    (void)data;

    // PROTOTYPING.
    drgui_draw_rect(pInfoBar, drgui_get_local_rect(pInfoBar), drgui_rgb(128, 64, 64), pPaintData);
}


void dred_info_bar__on_paint(dred_info_bar* pInfoBar, drgui_rect rect, void* pPaintData)
{
    (void)rect;

    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    assert(data != NULL);

    
    //drgui_draw_rect_with_outline(pInfoBar, drgui_get_local_rect(pInfoBar), drgui_rgb(255, 128, 128), 2, drgui_rgb(0, 0, 0), pPaintData);

    
    if (data->type == DRED_INFO_BAR_TYPE_NONE) {
        dred_info_bar__on_paint__none(pInfoBar, data, pPaintData);
    } else if (data->type == DRED_INFO_BAR_TYPE_TEXT_EDITOR) {
        dred_info_bar__on_paint__text_editor(pInfoBar, data, pPaintData);
    }
}

dred_info_bar* dred_info_bar_create(dred_context* pDred, dred_control* pParent)
{
    dred_info_bar* pInfoBar = dred_control_create(pDred, pParent, DRED_CONTROL_TYPE_INFO_BAR, sizeof(dred_info_bar_data));
    if (pInfoBar == NULL) {
        return NULL;
    }

    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    assert(data != NULL);

    data->type = DRED_INFO_BAR_TYPE_NONE;
    data->lineStr[0] = '\0';
    data->colStr[0] = '\0';

    // Events.
    pInfoBar->onPaint = dred_info_bar__on_paint;

    return pInfoBar;
}

void dred_info_bar_delete(dred_info_bar* pInfoBar)
{
    dred_control_delete(pInfoBar);
}

void dred_info_bar_update(dred_info_bar* pInfoBar, dred_control* pControl)
{
    dred_info_bar_data* data = (dred_info_bar_data*)dred_control_get_extra_data(pInfoBar);
    if (data == NULL) {
        return;
    }

    data->type = DRED_INFO_BAR_TYPE_NONE;

    if (pControl != NULL) {
        if (dred_control_is_of_type(pControl, DRED_CONTROL_TYPE_TEXT_EDITOR)) {
            data->type = DRED_INFO_BAR_TYPE_TEXT_EDITOR;

            _itoa_s(dred_text_editor_get_cursor_line(pControl), data->lineStr, sizeof(data->lineStr), 10);
            _itoa_s(dred_text_editor_get_cursor_line(pControl), data->colStr,  sizeof(data->colStr),  10);
        }
    }


    // The bar needs to be redrawn.
    drgui_dirty(pInfoBar, drgui_get_local_rect(pInfoBar));
}