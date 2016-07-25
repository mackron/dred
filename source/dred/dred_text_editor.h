// Copyright (C) 2016 David Reid. See included LICENSE file.

#define DRED_CONTROL_TYPE_TEXT_EDITOR  "dred.editor.text"

typedef struct dred_text_editor dred_text_editor;
#define DRED_TEXT_EDITOR(a) ((dred_text_editor*)(a))

struct dred_text_editor
{
    // The base editor.
    dred_editor editor;

    dred_textbox* pTextBox;
    unsigned int iBaseUndoPoint;    // Used to determine whether or no the file has been modified.
    float textScale;

    dred_highlighter highlighter;
};


// dred_text_editor_create()
dred_text_editor* dred_text_editor_create(dred_context* pDred, dred_control* pParent, float sizeX, float sizeY, const char* filePathAbsolute);

// dred_text_editor_delete()
void dred_text_editor_delete(dred_text_editor* pTextEditor);


// Retrieves a copy of the text.
size_t dred_text_editor_get_text(dred_text_editor* pTextEditor, char* pTextOut, size_t textOutSize);


// Enables word wrap.
void dred_text_editor_enable_word_wrap(dred_text_editor* pTextEditor);

// Disables word wrap.
void dred_text_editor_disable_word_wrap(dred_text_editor* pTextEditor);

// Determines whether or not word wrap is enabled.
bool dred_text_editor_is_word_wrap_enabled(dred_text_editor* pTextEditor);


// Inserts text at every cursor.
//
// Returns whether or not the text was changed.
bool dred_text_editor_insert_text_at_cursors(dred_text_editor* pTextEditor, const char* text);


// Refreshes the styling of the given text editor.
void dred_text_editor_refresh_styling(dred_text_editor* pTextEditor);

// Sets the syntax highlighting for the given text editor based on a language string.
void dred_text_editor_set_highlighter(dred_text_editor* pTextEditor, const char* lang);


// Sets the base font to use with this text editor.
void dred_text_editor_set_font(dred_text_editor* pTextEditor, dred_font* pFont);


// Shows the line numbers.
void dred_text_editor_show_line_numbers(dred_text_editor* pTextEditor);

// Hides the line numbers.
void dred_text_editor_hide_line_numbers(dred_text_editor* pTextEditor);


// Retrieves the index of the line the caret is currently sitting on.
size_t dred_text_editor_get_cursor_line(dred_text_editor* pTextEditor);

// Retrieves the index of the column the caret is currently sitting on.
size_t dred_text_editor_get_cursor_column(dred_text_editor* pTextEditor);


// dred_text_editor_goto_ratio()
void dred_text_editor_goto_ratio(dred_text_editor* pTextEditor, size_t ratio);

// dred_text_editor_goto_line()
void dred_text_editor_goto_line(dred_text_editor* pTextEditor, size_t lineNumber);


// Finds and selects the next occurance of the given string, starting from the cursor and looping back to the start.
bool dred_text_editor_find_and_select_next(dred_text_editor* pTextEditor, const char* text);

// Finds the next occurance of the given string and replaces it with another.
bool dred_text_editor_find_and_replace_next(dred_text_editor* pTextEditor, const char* text, const char* replacement);

// Finds every occurance of the given string and replaces it with another.
bool dred_text_editor_find_and_replace_all(dred_text_editor* pTextEditor, const char* text, const char* replacement);


// Sets the scale of the internal text.
void dred_text_editor_set_text_scale(dred_text_editor* pTextEditor, float textScale);

// Unindents selected blocks.
void dred_text_editor_unindent_selected_blocks(dred_text_editor* pTextEditor);