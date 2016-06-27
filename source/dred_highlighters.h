// This file contains all of the highlighters for every supported language.

typedef enum
{
    dred_highlight_category_default,
    dred_highlight_category_comment,
    dred_highlight_category_string,
    dred_highlight_category_token
} dred_highlight_category;

typedef struct
{
    const char* keyword;
} dred_highlight_keyword;

#if 0
static dred_highlight_keyword g_KeywordsC[] = {
    "char",
    "short",
    "int",
    "float",
    "unsigned"
};
#endif

typedef struct
{
    const char* text;   // <-- This is a reference, not a copy.
    dred_highlight_keyword* keywords;
    size_t keywordCount;
} dred_highlighter_generic;

//
bool dred_highlighter_generic_init(dred_highlighter_generic* pHighlighter, const char* text, dred_highlight_keyword* keywords, size_t keywordCount);

// Gets the next highlight based on the given character.
bool dred_highlighter_generic_get_next_highlight(dred_highlighter_generic* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut, dred_highlight_category* pCategoryOut);
