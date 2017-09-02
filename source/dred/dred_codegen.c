// Copyright (C) 2017 David Reid. See included LICENSE file.

char* dred_codegen_buffer_to_c_array(const unsigned char* buffer, size_t size, const char* variableName)
{
    const unsigned int bytesPerLine = 16;
    const char* header = "static const unsigned char ";
    const char* declarationTail = "[] = {\n";

    size_t headerLen          = strlen(header);
    size_t variableNameLen    = strlen(variableName);
    size_t declarationTailLen = strlen(declarationTail);

    size_t totalLen = headerLen + variableNameLen + declarationTailLen;
    totalLen += size * 6;                                                // x6 because we store 6 character's per byte.
    totalLen += (size / bytesPerLine + 1) * 4;                           // Indentation.
    totalLen += 2;                                                       // +2 for the "};" at the end.

    char* output = (char*)malloc(totalLen);                              // No need for +1 for the null terminator because the last byte will not have a trailing "," which leaves room.

    char* runningOutput = output;
    memcpy(runningOutput, header, headerLen);
    runningOutput += headerLen;

    memcpy(runningOutput, variableName, variableNameLen);
    runningOutput += variableNameLen;

    memcpy(runningOutput, declarationTail, declarationTailLen);
    runningOutput += declarationTailLen;

    for (unsigned int i = 0; i < size; ++i)
    {
        const unsigned char byte = buffer[i];

        if ((i % bytesPerLine) == 0) {
            runningOutput[0] = ' ';
            runningOutput[1] = ' ';
            runningOutput[2] = ' ';
            runningOutput[3] = ' ';
            runningOutput += 4;
        }

        runningOutput[0] = '0';
        runningOutput[1] = 'x';
        runningOutput[2] = ((byte >>  4) + '0'); if (runningOutput[2] > '9') runningOutput[2] += 7;
        runningOutput[3] = ((byte & 0xF) + '0'); if (runningOutput[3] > '9') runningOutput[3] += 7;
        runningOutput += 4;

        if (i + 1 < size) {
            *runningOutput++ = ',';
        }
        
        if ((i % bytesPerLine)+1 == bytesPerLine || i + 1 == size) {
            *runningOutput++ = '\n';
        } else {
            *runningOutput++ = ' ';
        }
    }

    runningOutput[0] = '}';
    runningOutput[1] = ';';
    runningOutput[2] = '\0';
    return output;
}


char* dred_codegen_buffer_to_c_string(const unsigned char* buffer, size_t size, const char* variableName)
{
    if (buffer == NULL) return NULL;

    const char* input = (const char*)buffer;
    const char* indent = "";

    char* output = dtk_make_string("");

    // Don't include the variable declaration if no variable name was specified.
    if (variableName != NULL) {
        indent = "    ";
        output = dtk_make_string("static const char* ");
        output = dtk_append_string(output, variableName);
        output = dtk_append_string(output, " = {\n");
    }

    output = dtk_append_string(dtk_append_string(output, indent), "\"");   // <-- Begin the first line with a double-quote.
    {
        // At the momement all we're doing is wrapping each line with " ... \n", but later on we'll want to do
        // proper tab formatting and UTF-8 conversion.
        for (unsigned int ichar = 0; ichar < size; ++ichar) {
            switch (input[ichar]) {
                case '\n': output = dtk_append_string(output, "\\n"); output = dtk_append_string(output, "\"\n"); output = dtk_append_string(dtk_append_string(output, indent), "\""); break;  // <-- Terminate the line with a double-quote and place the double-quote for the following line.
                case '\r': output = dtk_append_string(output, "\\r"); break;
                case '\t': output = dtk_append_string(output, "\\t"); break;
                case '\"': output = dtk_append_string(output, "\\\""); break;
                case '\\': output = dtk_append_string(output, "\\\\"); break;
                default:
                {
                    // TODO: Check for non-ASCII characters and add support for UTF-8 hex characters.
                    output = dtk_append_string_length(output, &input[ichar], 1);  
                } break;
            }
        }
    }
    output = dtk_append_string(output, "\"");   // <-- End the last line with a double-quote.
    if (variableName != NULL) {
        output = dtk_append_string(output, "\n};");
    }

    // The documentation for this function says to release the returned pointer with free(), however since we
    // used gb_string to construct it. Therefore we need to make a copy of the string before returning.
    size_t outputLength = dtk_string_length(output);
    char* actualOutput = (char*)malloc(outputLength + 1);
    memcpy(actualOutput, output, outputLength + 1); // +1 for null terminator.

    dtk_free_string(output);
    return actualOutput;
}