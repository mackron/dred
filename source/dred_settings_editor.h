
#define DRED_CONTROL_TYPE_SETTINGS_EDITOR  "dred.editor.settings"

typedef dred_editor dred_settings_editor;

dred_settings_editor* dred_settings_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute);
void dred_settings_editor_delete(dred_settings_editor* pSettingsEditor);