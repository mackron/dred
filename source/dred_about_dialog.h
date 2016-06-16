
typedef struct
{
    dred_window* pWindow;
    drgui_image* pLogo;
} dred_about_dialog;

dred_about_dialog* dred_about_dialog_create(dred_context* pDred);
void dred_about_dialog_delete(dred_about_dialog* pDialog);

void dred_about_dialog_show(dred_about_dialog* pDialog);
void dred_about_dialog_hide(dred_about_dialog* pDialog);