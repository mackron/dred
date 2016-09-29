// Copyright (C) 2016 David Reid. See included LICENSE file.

///////////////////////////////////////////////////////////////////////////////
//
// Generic Highlighter
//
///////////////////////////////////////////////////////////////////////////////

// The generic highlighter is controlled based on basic properties. It does not do any language specific features, however it should have
// enough properties that it can be used for many languages.

drBool32 dred_highlighter__get_block_comment_of_character(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;
    return DR_FALSE;
}

drBool32 dred_highlighter__get_line_comment_of_character(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;
    return DR_FALSE;
}

drBool32 dred_highlighter__get_comment_of_character(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;
    return DR_FALSE;
}



drBool32 dred_highlighter__get_next_block_comment_on_line(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;
    return DR_FALSE;
}

drBool32 dred_highlighter__get_next_line_comment_on_line(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;
    return DR_FALSE;
}

drBool32 dred_highlighter__get_next_comment_on_line(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;
    return DR_FALSE;
}


drBool32 dred_highlighter__on_get_next_highlight(drte_engine* pEngine, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut, drte_style_token* pStyleTokenOut, void* pUserData)
{
    if (pEngine == NULL || pCharBegOut == NULL || pCharEndOut == NULL || pStyleTokenOut == NULL || pUserData == NULL) {
        return DR_FALSE;
    }

    (void)iChar;
    return DR_FALSE;
}


drBool32 dred_highlighter_init(dred_highlighter* pHighlighter, dred_context* pDred, drte_engine* pEngine, dred_highlight_keyword* keywords, size_t keywordCount)
{
    if (pHighlighter == NULL) {
        return DR_FALSE;
    }

    (void)pDred;
    (void)pEngine;
    (void)keywords;
    (void)keywordCount;
    return DR_FALSE;
}
