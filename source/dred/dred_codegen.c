// Copyright (C) 2016 David Reid. See included LICENSE file.

char* dred_codegen_buffer_to_c_array(const unsigned char* buffer, unsigned int size, const char* variableName)
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

    char* output = malloc(totalLen);                                     // No need for +1 for the null terminator because the last byte will not have a trailing "," which leaves room.

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