// Copyright (C) 2016 David Reid. See included LICENSE file.

// This file contains all of the highlighters for every supported language.

typedef enum
{
    dred_highlight_category_default,
    dred_highlight_category_comment,
    dred_highlight_category_string,
    dred_highlight_category_datatype,
    dred_highlight_category_instruction
} dred_highlight_category;

typedef struct
{
    const char* keyword;
    dred_highlight_category category;
} dred_highlight_keyword;

static dred_highlight_keyword g_KeywordsC[] = {
    { "char",     dred_highlight_category_datatype },
    { "short",    dred_highlight_category_datatype },
    { "int",      dred_highlight_category_datatype },
    { "float",    dred_highlight_category_datatype },
    { "unsigned", dred_highlight_category_datatype }
};

typedef struct
{
    dred_context* pDred;
    drte_engine* pEngine;
    drte_engine_on_get_next_highlight_proc onNextHighlight;

    union
    {
        struct
        {
            dred_text_style comment;
            dred_text_style string;
            dred_text_style keyword;
        } common;

        struct
        {
            int unused;
        } cpp;
    } styles;

    union
    {
        struct
        {
            dred_highlight_keyword* keywords;
            size_t keywordCount;
        } builtin;
    } data;
    
} dred_highlighter;

//
dr_bool32 dred_highlighter_init(dred_highlighter* pHighlighter, dred_context* pDred, drte_engine* pEngine, dred_highlight_keyword* keywords, size_t keywordCount);
