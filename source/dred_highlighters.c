
///////////////////////////////////////////////////////////////////////////////
//
// Generic Highlighter
//
///////////////////////////////////////////////////////////////////////////////

// The generic highlighter is controlled based on basic properties. It does not do any language specific features, however it should have
// enough properties that it can be used for many languages.

bool dred_highlighter_generic__get_block_comment_of_character(dred_highlighter_generic* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    // TODO: Implement me.
    (void)pHighlighter;
    (void)iChar;
    (void)pCharBegOut;
    (void)pCharEndOut;

    return false;
}

bool dred_highlighter_generic__get_line_comment_of_character(dred_highlighter_generic* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    drcpp_region region;
    if (!drcpp_parser_get_line_comment_of_character(pHighlighter->text, iChar, &region)) {
        return false;
    }

    *pCharBegOut = region.iCharBeg;
    *pCharEndOut = region.iCharEnd;
    return true;
}

bool dred_highlighter_generic__get_comment_of_character(dred_highlighter_generic* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    size_t iBegBlock = (size_t)-1;
    size_t iEndBlock = (size_t)-1;
    bool isInBlockComment = dred_highlighter_generic__get_block_comment_of_character(pHighlighter, iChar, &iBegBlock, &iEndBlock);
    
    size_t iBegLine = (size_t)-1;
    size_t iEndLine = (size_t)-1;
    bool isInLineComment = dred_highlighter_generic__get_line_comment_of_character(pHighlighter, iChar, &iBegLine, &iEndLine);

    if (isInBlockComment) {
        if (isInLineComment) {
            if (iBegBlock < iBegLine) {
                *pCharBegOut = iBegBlock;
                *pCharEndOut = iEndBlock;
            } else {
                *pCharBegOut = iBegLine;
                *pCharEndOut = iEndLine;
            }
        } else {
            *pCharBegOut = iBegBlock;
            *pCharEndOut = iEndBlock;
        }
    } else {
        if (isInLineComment) {
            *pCharBegOut = iBegLine;
            *pCharEndOut = iEndLine;
        } else {
            return false;   // Not any comment block.
        }
    }

    return true;
}


bool dred_highlighter_generic_init(dred_highlighter_generic* pHighlighter, const char* text, dred_highlight_keyword* keywords, size_t keywordCount)
{
    if (pHighlighter == NULL) {
        return false;
    }

    if (keywords == NULL) {
        keywordCount = 0;
    }

    pHighlighter->text = text;
    pHighlighter->keywords = keywords;
    pHighlighter->keywordCount = keywordCount;

    return true;
}

bool dred_highlighter_generic_get_next_highlight(dred_highlighter_generic* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut, dred_highlight_category* pCategoryOut)
{
    if (pHighlighter == NULL || pCharBegOut == NULL || pCharEndOut == NULL || pCategoryOut == NULL) {
        return false;
    }

    size_t iCharBegComment = (size_t)-1;
    size_t iCharEndComment = (size_t)-1;
    bool isInComment = dred_highlighter_generic__get_comment_of_character(pHighlighter, iChar, &iCharBegComment, &iCharEndComment);

    // TODO: String
    //size_t iCharBegString = (size_t)-1;
    //size_t iCharEndString = (size_t)-1;
    bool isInString = false;

    // TODO: Keyword
    //size_t iCharBegKeyword = (size_t)-1;
    //size_t iCharEndKeyword = (size_t)-1;
    bool isInKeyword = false;

    if (isInComment || isInString || isInKeyword) {
        *pCharBegOut = iCharBegComment;
        *pCharEndOut = iCharEndComment;
        return true;
    }



    // If we get here it means the cursor is not sitting on a highlight. Now we need to find the _next_ highlight on the line.



    return false;
}