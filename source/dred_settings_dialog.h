
typedef struct
{
    dred_window* pWindow;
} dred_settings_dialog;

dred_settings_dialog* dred_settings_dialog_create(dred_context* pDred);
void dred_settings_dialog_delete(dred_settings_dialog* pDialog);

void dred_settings_dialog_show(dred_settings_dialog* pDialog);
void dred_settings_dialog_hide(dred_settings_dialog* pDialog);