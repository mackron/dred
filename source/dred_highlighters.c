
///////////////////////////////////////////////////////////////////////////////
//
// Generic Highlighter
//
///////////////////////////////////////////////////////////////////////////////

// The generic highlighter is controlled based on basic properties. It does not do any language specific features, however it should have
// enough properties that it can be used for many languages.

bool dred_highlighter__get_block_comment_of_character(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    drcpp_region region;
    if (!drcpp_parser_get_block_comment_of_character(pHighlighter->pEngine->text, iChar, &region)) {
        return false;
    }

    *pCharBegOut = region.iCharBeg;
    *pCharEndOut = region.iCharEnd;
    return true;
}

bool dred_highlighter__get_line_comment_of_character(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    drcpp_region region;
    if (!drcpp_parser_get_line_comment_of_character(pHighlighter->pEngine->text, iChar, &region)) {
        return false;
    }

    *pCharBegOut = region.iCharBeg;
    *pCharEndOut = region.iCharEnd;
    return true;
}

bool dred_highlighter__get_comment_of_character(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    size_t iBegBlock = (size_t)-1;
    size_t iEndBlock = (size_t)-1;
    bool isInBlockComment = dred_highlighter__get_block_comment_of_character(pHighlighter, iChar, &iBegBlock, &iEndBlock);
    
    size_t iBegLine = (size_t)-1;
    size_t iEndLine = (size_t)-1;
    bool isInLineComment = dred_highlighter__get_line_comment_of_character(pHighlighter, iChar, &iBegLine, &iEndLine);

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



bool dred_highlighter__get_next_block_comment_on_line(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    drcpp_region region;
    if (!drcpp_parser_get_next_block_comment_on_line(pHighlighter->pEngine->text, iChar, &region)) {    
        return false;
    }

    *pCharBegOut = region.iCharBeg;
    *pCharEndOut = region.iCharEnd;
    return true;
}

bool dred_highlighter__get_next_line_comment_on_line(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    drcpp_region region;
    if (!drcpp_parser_get_next_line_comment_on_line(pHighlighter->pEngine->text, iChar, &region)) {    
        return false;
    }

    *pCharBegOut = region.iCharBeg;
    *pCharEndOut = region.iCharEnd;
    return true;
}

bool dred_highlighter__get_next_comment_on_line(dred_highlighter* pHighlighter, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut)
{
    assert(pHighlighter != NULL);
    assert(pCharBegOut != NULL);
    assert(pCharEndOut != NULL);

    size_t iBegBlock = (size_t)-1;
    size_t iEndBlock = (size_t)-1;
    bool foundBlockComment = dred_highlighter__get_next_block_comment_on_line(pHighlighter, iChar, &iBegBlock, &iEndBlock);
    
    size_t iBegLine = (size_t)-1;
    size_t iEndLine = (size_t)-1;
    bool foundLineComment = dred_highlighter__get_next_line_comment_on_line(pHighlighter, iChar, &iBegLine, &iEndLine);

    if (foundBlockComment) {
        if (foundLineComment) {
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
        if (foundLineComment) {
            *pCharBegOut = iBegLine;
            *pCharEndOut = iEndLine;
        } else {
            return false;   // Not any comment block.
        }
    }

    return true;
}


bool dred_highlighter__on_get_next_highlight(drte_engine* pEngine, size_t iChar, size_t* pCharBegOut, size_t* pCharEndOut, drte_style_token* pStyleTokenOut, void* pUserData)
{
    if (pEngine == NULL || pCharBegOut == NULL || pCharEndOut == NULL || pStyleTokenOut == NULL || pUserData == NULL) {
        return false;
    }

    dred_highlighter* pHighlighter = (dred_highlighter*)pUserData;


    size_t iCharBegComment = (size_t)-1;
    size_t iCharEndComment = (size_t)-1;
    bool isInComment = dred_highlighter__get_comment_of_character(pHighlighter, iChar, &iCharBegComment, &iCharEndComment);

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
        *pStyleTokenOut = (drte_style_token)&pHighlighter->styles.common.comment;    // TODO: Implement me properly.
        return true;
    }


    // If we get here it means the cursor is not sitting on a highlight. Now we need to find the _next_ highlight on the line.


    bool isCommentOnLine = dred_highlighter__get_next_comment_on_line(pHighlighter, iChar, &iCharBegComment, &iCharEndComment);

    // TODO: String
    bool isStringOnLine = false;

    // TODO: Keyword
    bool isKeywordOnLine = false;

    if (isCommentOnLine || isStringOnLine || isKeywordOnLine) {
        *pCharBegOut = iCharBegComment;
        *pCharEndOut = iCharEndComment;
        *pStyleTokenOut = (drte_style_token)&pHighlighter->styles.common.comment;    // TODO: Implement me properly.
        return true;
    }

    return false;
}


bool dred_highlighter_init(dred_highlighter* pHighlighter, dred_context* pDred, drte_engine* pEngine, dred_highlight_keyword* keywords, size_t keywordCount)
{
    if (pHighlighter == NULL) {
        return false;
    }

    if (keywords == NULL) {
        keywordCount = 0;
    }

    pHighlighter->pDred = pDred;
    pHighlighter->pEngine = pEngine;
    pHighlighter->onNextHighlight = dred_highlighter__on_get_next_highlight;

    pHighlighter->data.builtin.keywords = keywords;
    pHighlighter->data.builtin.keywordCount = keywordCount;

    return true;
}
