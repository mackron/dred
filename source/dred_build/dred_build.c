// Copyright (C) 2017 David Reid. See included LICENSE file.

#define _CRT_SECURE_NO_WARNINGS

#define DR_IMPLEMENTATION
#include "../../../../dr_libs/dr.h"

#include "../external/stretchy_buffer.h"

#define DR_WEBGEN_IMPLEMENTATION
#include "../../../dr_webgen/dr_webgen.h"

#include "../external/json.c"

#include "../dred/dred_build_config.h"

#include "../dred/dtk/dtk.c"
#include "../dred/dred_string_pool.h"
#include "../dred/dred_string_pool.c"

typedef struct
{
    char name[256];
    char proc[256];
    char flags[256];
} command_var;
command_var* g_CommandVars = NULL;  // <-- stretchy_buffer

typedef struct
{
    char name[256];
    char varname[256];
    unsigned int type;
    char typeSrc[256];
    char setCallback[256];
    char defaultValue[1024];
    char defaultValueSrc[1024];
    char* documentation;
} config_var;
config_var* g_ConfigVars = NULL;    // <-- stretchy_buffer

typedef struct
{
    char filename[256];
    char id[256];
    unsigned int baseWidth;
    unsigned int baseHeight;
} stock_image;

static void fwrite_string(FILE* pFile, const char* str)
{
    fwrite(str, 1, strlen(str), pFile);
}

const char* dred_build__json_error_to_string(enum json_parse_error_e error)
{
    switch (error)
    {
        case json_parse_error_none: return "json_parse_error_none";
        case json_parse_error_expected_comma: return "json_parse_error_expected_comma";
        case json_parse_error_expected_colon: return "json_parse_error_expected_colon";
        case json_parse_error_expected_opening_quote: return "json_parse_error_expected_opening_quote";
        case json_parse_error_invalid_string_escape_sequence: return "json_parse_error_invalid_string_escape_sequence";
        case json_parse_error_invalid_number_format: return "json_parse_error_invalid_number_format";
        case json_parse_error_invalid_value: return "json_parse_error_invalid_value";
        case json_parse_error_premature_end_of_buffer: return "json_parse_error_premature_end_of_buffer";
        case json_parse_error_invalid_string: return "json_parse_error_invalid_string";
        case json_parse_error_allocator_failed: return "json_parse_error_allocator_failed";
        case json_parse_error_unexpected_trailing_characters: return "json_parse_error_unexpected_trailing_characters";
        case json_parse_error_unknown: return "json_parse_error_unknown";
        default: break;
    }

    return "Unknown error";
}

dr_bool32 dred_build_find_config_var(const char* name, size_t* pIndex)
{
    if (pIndex) *pIndex = 0;

    for (int i = 0; i < stb_sb_count(g_ConfigVars); ++i) {
        if (strcmp(g_ConfigVars[i].name, name) == 0) {
            if (pIndex) *pIndex = i;
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

#include "dred_build_shortcuts.c"
#include "dred_build_menus.c"
#include "dred_build_website.c"

#include <stdio.h>
#include <assert.h>

double g_StockImageScales[] = {1.0, 1.5, 2.0};
#define STOCK_IMAGE_SCALE_COUNT (sizeof(g_StockImageScales) / sizeof(g_StockImageScales[0]))

#define BYTES_PER_ROW   16

#define CONFIG_VAR_TYPE_INTEGER     1
#define CONFIG_VAR_TYPE_FLOAT       2
#define CONFIG_VAR_TYPE_BOOL        3
#define CONFIG_VAR_TYPE_STRING      4
#define CONFIG_VAR_TYPE_FONT        5
#define CONFIG_VAR_TYPE_IMAGE       6
#define CONFIG_VAR_TYPE_COLOR       7

const char* config_var_type_to_string(unsigned int type)
{
    if (type == CONFIG_VAR_TYPE_INTEGER) {
        return "int";
    }
    if (type == CONFIG_VAR_TYPE_FLOAT) {
        return "float";
    }
    if (type == CONFIG_VAR_TYPE_BOOL) {
        return "dr_bool32";
    }
    if (type == CONFIG_VAR_TYPE_STRING) {
        return "char*";
    }
    if (type == CONFIG_VAR_TYPE_FONT) {
        return "dred_font*";
    }
    if (type == CONFIG_VAR_TYPE_IMAGE) {
        return "dred_image*";
    }
    if (type == CONFIG_VAR_TYPE_COLOR) {
        return "dtk_color";
    }

    return "UNKOWN TYPE";
}

unsigned int parse_config_var_type(const char* type)
{
    if (strcmp(type, "int") == 0) {
        return CONFIG_VAR_TYPE_INTEGER;
    } else if (strcmp(type, "float") == 0) {
        return CONFIG_VAR_TYPE_FLOAT;
    } else if (strcmp(type, "bool") == 0 || strcmp(type, "dr_bool32") == 0) {
        return CONFIG_VAR_TYPE_BOOL;
    } else if (strcmp(type, "string") == 0) {
        return CONFIG_VAR_TYPE_STRING;
    } else if (strcmp(type, "font") == 0) {
        return CONFIG_VAR_TYPE_FONT;
    } else if (strcmp(type, "image") == 0) {
        return CONFIG_VAR_TYPE_IMAGE;
    } else if (strcmp(type, "color") == 0) {
        return CONFIG_VAR_TYPE_COLOR;
    }

    printf("WARNING: Unknown config variable type: %s\n", type);
    return 0;   // Unknown type.
}

void get_config_var_default_value(unsigned int type, char* valueOut, size_t valueOutSize)
{
    if (type == CONFIG_VAR_TYPE_INTEGER) {
        strcpy_s(valueOut, valueOutSize, "0");
        return;
    }
    if (type == CONFIG_VAR_TYPE_FLOAT) {
        strcpy_s(valueOut, valueOutSize, "0.0f");
        return;
    }
    if (type == CONFIG_VAR_TYPE_BOOL) {
        strcpy_s(valueOut, valueOutSize, "DR_FALSE");
        return;
    }
    if (type == CONFIG_VAR_TYPE_STRING) {
        strcpy_s(valueOut, valueOutSize, "dtk_make_string(\"\")");
        return;
    }
    if (type == CONFIG_VAR_TYPE_FONT) {
        strcpy_s(valueOut, valueOutSize, "dred_parse_and_load_font(pConfig->pDred, \"system-font-ui\")");
        return;
    }
    if (type == CONFIG_VAR_TYPE_IMAGE) {
        strcpy_s(valueOut, valueOutSize, "NULL");
        return;
    }
    if (type == CONFIG_VAR_TYPE_COLOR) {
        strcpy_s(valueOut, valueOutSize, "dred_rgb(0, 0, 0)");
        return;
    }
}

void parse_config_var_value(unsigned int type, const char* valueIn, char* valueOut, size_t valueOutSize)
{
    valueIn = dr_first_non_whitespace(valueIn);

    char* str = dr_string_replace(valueIn, "\"", "\\\"");

    if (type == CONFIG_VAR_TYPE_INTEGER) {
        strcpy_s(valueOut, valueOutSize, valueIn);
        return;
    }
    if (type == CONFIG_VAR_TYPE_FLOAT) {
        strcpy_s(valueOut, valueOutSize, valueIn);
        return;
    }
    if (type == CONFIG_VAR_TYPE_BOOL) {
        strcpy_s(valueOut, valueOutSize, valueIn);
        return;
    }
    if (type == CONFIG_VAR_TYPE_STRING) {
        snprintf(valueOut, valueOutSize, "dtk_make_string(\"%s\")", str);
        return;
    }
    if (type == CONFIG_VAR_TYPE_FONT) {
        snprintf(valueOut, valueOutSize, "dred_parse_and_load_font(pConfig->pDred, \"%s\")", str);
        return;
    }
    if (type == CONFIG_VAR_TYPE_IMAGE) {
        strcpy_s(valueOut, valueOutSize, "NULL");
        return;
    }
    if (type == CONFIG_VAR_TYPE_COLOR) {
        // Format: <r> <g> <b>
        char r[4];
        char g[4];
        char b[4];
        char a[4];

        valueIn = dr_next_token(valueIn, r, sizeof(r));
        valueIn = dr_next_token(valueIn, g, sizeof(g));
        valueIn = dr_next_token(valueIn, b, sizeof(b));
        if (dr_next_token(valueIn, a, sizeof(a)) == NULL) {
            a[0] = '2'; a[1] = '5'; a[2] = '5'; a[3] = '\0';
        }

        snprintf(valueOut, valueOutSize, "dred_rgba(%s, %s, %s, %s)", r, g, b, a);
        return;
    }

    free(str);
}


char* write_image_data_rgba8(char* output, unsigned int* pCurrentByteColumn, const uint8_t* pImageData, unsigned int width, unsigned int height, unsigned int stride)
{
    unsigned int currentByteColumn = *pCurrentByteColumn;

    char byteStr[5] = {'0', 'x', '0', '0', '\0'};

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            for (unsigned int c = 0; c < 4; ++c) {
                if (currentByteColumn == 0) {
                    output = dtk_append_string(output, "\n    ");
                }

                uint8_t b = pImageData[y*stride + x*4 + c];
                byteStr[2] = ((b >>  4) + '0'); if (byteStr[2] > '9') byteStr[2] += 7;
                byteStr[3] = ((b & 0xF) + '0'); if (byteStr[3] > '9') byteStr[3] += 7;
                output = dtk_append_string(output, byteStr);

                currentByteColumn = (currentByteColumn + 1) % BYTES_PER_ROW;

                if (y+1 != height || x+1 != width || c+1 != 4) {
                    output = dtk_append_string(output, ",");
                }
            }
        }
    }

    *pCurrentByteColumn = currentByteColumn;
    return output;
}

char* stringify_string_pool_data(dred_string_pool* pStringPool)
{
    unsigned int currentByteColumn = 0;
    char byteStr[5] = {'0', 'x', '0', '0', '\0'};

    char* output = dtk_make_string("const unsigned char g_InitialStringPoolData[] = {");
    for (size_t i = 0; i < pStringPool->byteCount; ++i) {
        if (currentByteColumn == 0) {
            output = dtk_append_string(output, "\n    ");
        }

        uint8_t b = (uint8_t)pStringPool->pData[i];
        byteStr[2] = ((b >>  4) + '0'); if (byteStr[2] > '9') byteStr[2] += 7;
        byteStr[3] = ((b & 0xF) + '0'); if (byteStr[3] > '9') byteStr[3] += 7;
        output = dtk_append_string(output, byteStr);

        currentByteColumn = (currentByteColumn + 1) % BYTES_PER_ROW;

        if (i < pStringPool->byteCount-1) {
            output = dtk_append_string(output, ",");
        }
    }

    output = dtk_append_string(output, "\n};\n");
    return output;
}



void generate_commands_list(FILE* pFileOut)
{
    assert(pFileOut != NULL);

    size_t fileDataSize;
    char* fileData = dr_open_and_read_text_file("../../../source/dred/dred_commands.h", &fileDataSize);
    if (fileData == NULL) {
        return;
    }

    // Look for the line beginning with "// BEGIN COMMAND LIST"
    char line[1024];
    const char* nextLine = fileData;
    while (nextLine != NULL) {
        if (dr_copy_line(nextLine, line, sizeof(line)) == (size_t)-1) {
            return;
        }
        if (strstr(line, "// BEGIN COMMAND LIST") != NULL) {
            nextLine = dr_next_line(nextLine);
            break;
        }
        nextLine = dr_next_line(nextLine);
    }



    char* CommandNamePool = dtk_make_string("const char g_CommandNamePool[] = ");
    char* CommandNames    = dtk_make_string("const char* g_CommandNames[] = {\n");
    char* Commands        = dtk_make_string("dred_command g_Commands[] = {\n");

    size_t runningNameLength = 0;
    int commandCount = 0;

    while (nextLine != NULL) {
        // The next line should be in the format of <name> <proc> <flags>
        size_t lineLength = dr_copy_line(nextLine, line, sizeof(line));
        if (lineLength <= 2) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        if (strstr(line, "// END COMMAND LIST") != NULL) {
            break;
        }


        command_var command;
        const char* next = dr_next_token(line + 2, command.name, sizeof(command.name));     // Skip past "//"
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        next = dr_next_token(next, command.proc, sizeof(command.proc));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        next = dr_next_token(next, command.flags, sizeof(command.flags));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }


        snprintf(line, sizeof(line), "\n    \"%s\\0\"", command.name);
        CommandNamePool = dtk_append_string(CommandNamePool, line);

        snprintf(line, sizeof(line), "    g_CommandNamePool + %d,\n", (int)runningNameLength);
        CommandNames = dtk_append_string(CommandNames, line);

        snprintf(line, sizeof(line), "    {%s, %s},\n", command.proc, command.flags);
        Commands = dtk_append_string(Commands, line);



        runningNameLength += strlen(command.name)+1;
        commandCount += 1;

        stb_sb_push(g_CommandVars, command);

        nextLine = dr_next_line(nextLine + lineLength);
    }

    fwrite_string(pFileOut, "\n// Commands\n");
    snprintf(line, sizeof(line), "#define DRED_COMMAND_COUNT %d\n\n", commandCount);
    fwrite_string(pFileOut, line);

    CommandNamePool = dtk_append_string(CommandNamePool, ";\n\n");
    CommandNames    = dtk_append_string(CommandNames,    "};\n\n");
    Commands        = dtk_append_string(Commands,        "};\n\n");

    fwrite_string(pFileOut, CommandNamePool);
    fwrite_string(pFileOut, CommandNames);
    fwrite_string(pFileOut, Commands);
}

void generate_stock_images(FILE* pFileOut, FILE* pFileOutH)
{
    assert(pFileOut != NULL);
    assert(pFileOutH != NULL);

    size_t sourceFileDataSize;
    char* sourceFileData = dr_open_and_read_text_file("../../../source/dred/dred_image_library.h", &sourceFileDataSize);
    if (sourceFileData == NULL) {
        return;
    }

    // Look for the line beginning with "// BEGIN COMMAND LIST"
    char line[1024];
    const char* nextLine = sourceFileData;
    while (nextLine != NULL) {
        if (dr_copy_line(nextLine, line, sizeof(line)) == (size_t)-1) {
            return;
        }
        if (strstr(line, "// BEGIN STOCK IMAGE LIST") != NULL) {
            nextLine = dr_next_line(nextLine);
            break;
        }
        nextLine = dr_next_line(nextLine);
    }

    unsigned int imageCount = 0;
    stock_image* pStockImages = NULL;

    while (nextLine != NULL) {
        // The next line should be in the format of <name> <proc> <flags>
        size_t lineLength = dr_copy_line(nextLine, line, sizeof(line));
        if (lineLength <= 2) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        if (strstr(line, "// END STOCK IMAGE LIST") != NULL) {
            break;
        }

        stock_image stockImage;
        
        const char* next = dr_next_token(line + 2, stockImage.filename, sizeof(stockImage.filename));     // Skip past "//"
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        next = dr_next_token(next, stockImage.id, sizeof(stockImage.id));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        char widthStr[256];
        next = dr_next_token(next, widthStr, sizeof(widthStr));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        char heightStr[256];
        next = dr_next_token(next, heightStr, sizeof(heightStr));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        stockImage.baseWidth = (unsigned int)atoi(widthStr);
        stockImage.baseHeight = (unsigned int)atoi(heightStr);

        pStockImages = realloc(pStockImages, (imageCount+1) * sizeof(stock_image));
        pStockImages[imageCount] = stockImage;
        imageCount += 1;

        nextLine = dr_next_line(nextLine + lineLength);
    }


    // At this point we have the images so now we need to generate the code.
    fwrite_string(pFileOut, "\n// Stock Images\n");
    snprintf(line, sizeof(line), "#define DRED_STOCK_IMAGE_COUNT %d\n", imageCount);
    fwrite_string(pFileOut, line);

    snprintf(line, sizeof(line), "#define DRED_STOCK_IMAGE_SCALE_COUNT %d\n\n", (int)STOCK_IMAGE_SCALE_COUNT);
    fwrite_string(pFileOut, line);

    char* StockImageData = dtk_make_string("const uint8_t g_StockImageData[] = {");
    char* StockImages    = dtk_make_string("const dred_image_desc g_StockImages[DRED_STOCK_IMAGE_COUNT][DRED_STOCK_IMAGE_SCALE_COUNT] = {");

    unsigned int currentByteColumn = 0;
    unsigned int runningDataOffset = 0;

    for (unsigned int iStockImage = 0; iStockImage < imageCount; ++iStockImage)
    {
        stock_image* pStockImage = &pStockImages[iStockImage];

        snprintf(line, sizeof(line), "#define %s %d\n", pStockImage->id, iStockImage);
        fwrite_string(pFileOutH, line);

        char* imageStr = dtk_make_string("");
        if (iStockImage > 0) {
            imageStr = dtk_append_string(imageStr, ",");
        }

        imageStr = dtk_append_string(imageStr, "\n    {");


        // Load the image and rasterize it.
        char filename[256];
        drpath_copy_and_append(filename, sizeof(filename), "../../../resources/images", pStockImage->filename);

        char* svg = dr_open_and_read_text_file(filename, NULL);
        NSVGimage* pSVGImage = nsvgParse(svg, "px", 96);
        if (pSVGImage == NULL) {
            printf("Warning: Failed to parse SVG file: %s\n", filename);
        }
            

        int svgWidth  = (int)pSVGImage->width;
        int svgHeight = (int)pSVGImage->height;

        // At this point we have loaded the image and now we need to rasterize it.
        void* pImageData = malloc(svgWidth * svgHeight * 4);
            
        NSVGrasterizer* pSVGRast = nsvgCreateRasterizer();
        if (pSVGRast == NULL) {
            printf("Warning: Failed to create rasterizer for SVG file: %s\n", filename);
        }


        for (unsigned int iScale = 0; iScale < STOCK_IMAGE_SCALE_COUNT; ++iScale)
        {
            double scale = g_StockImageScales[iScale];
            unsigned int scaledWidth = (unsigned int)(pStockImage->baseWidth * scale);
            unsigned int scaledHeight = (unsigned int)(pStockImage->baseHeight * scale);

            if (iScale > 0) {
                imageStr = dtk_append_string(imageStr, ",\n     ");
            }

            snprintf(line, sizeof(line), "{%.1ff, %d, %d, g_StockImageData + %d}", scale, scaledWidth, scaledHeight, runningDataOffset);
            imageStr = dtk_append_string(imageStr, line);

            
            nsvgRasterize(pSVGRast, pSVGImage, 0, 0, scaledWidth / pSVGImage->width, pImageData, (int)svgWidth, (int)svgHeight, (int)svgWidth*4);
            
            
            StockImageData = write_image_data_rgba8(StockImageData, &currentByteColumn, pImageData, scaledWidth, scaledHeight, svgWidth*4);
            if (iStockImage+1 != imageCount || iScale+1 != STOCK_IMAGE_SCALE_COUNT) {
                StockImageData = dtk_append_string(StockImageData, ",");
            }

            runningDataOffset += (scaledWidth * scaledHeight * 4);
        }

        nsvgDeleteRasterizer(pSVGRast);
        nsvgDelete(pSVGImage);
        dr_free_file_data(svg);
        free(pImageData);


        imageStr = dtk_append_string(imageStr, "}");

        StockImages = dtk_append_string(StockImages, imageStr);
        dtk_free_string(imageStr);
    }

    StockImageData = dtk_append_string(StockImageData, "\n};\n\n");
    fwrite_string(pFileOut, StockImageData);

    StockImages = dtk_append_string(StockImages, "\n};\n\n");
    fwrite_string(pFileOut, StockImages);
}

void generate_config_vars(FILE* pFileOut, FILE* pFileOutH)
{
    assert(pFileOut != NULL);
    assert(pFileOutH != NULL);

    size_t sourceFileDataSize;
    char* sourceFileData = dr_open_and_read_text_file("../../../source/dred/dred_config.h", &sourceFileDataSize);
    if (sourceFileData == NULL) {
        return;
    }

    // Look for the line beginning with "// BEGIN CONFIG VARS"
    char line[4096];
    const char* nextLine = sourceFileData;
    while (nextLine != NULL) {
        if (dr_copy_line(nextLine, line, sizeof(line)) == (size_t)-1) {
            return;
        }
        if (strstr(line, "// BEGIN CONFIG VARS") != NULL) {
            nextLine = dr_next_line(nextLine);
            break;
        }
        nextLine = dr_next_line(nextLine);
    }


    config_var var;
    memset(&var, 0, sizeof(var));

    while (nextLine != NULL) {
        size_t lineLength = dr_copy_line(nextLine, line, sizeof(line));
        if (lineLength <= 2) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        if (strstr(line, "// END CONFIG VARS") != NULL) {
            break;
        }

        
        // First, just skip past the "//" to keep the rest simple.
        char* lineBeg = line + 2;

        // How we treat this line depends on whether or not we are starting a new declaration or adding to the documentation of the 
        // previous variable. Documentation is designated with 3 spaces.
        if (lineBeg[0] == ' ' && lineBeg[1] == ' ' && lineBeg[2] == ' ') {
            if (var.documentation == NULL) {
                var.documentation = dtk_make_string(lineBeg + 3);
            } else {
                var.documentation = dtk_append_string(var.documentation, " ");
                var.documentation = dtk_append_string(var.documentation, lineBeg + 3);
            }
        } else if (lineBeg[0] == '\n') {
            if (var.documentation) {
                var.documentation = dtk_append_string(var.documentation, "\n");
            }
        } else if (lineBeg[0] >= 32 && lineBeg[0] < 126) {
            // It's a new variable. If we were parsing a variable earlier the old one will need to be added.
            if (var.name[0] != '\0') {
                stb_sb_push(g_ConfigVars, var);
            }

            memset(&var, 0, sizeof(var));   // <-- Just make sure the variable is reset to make things easier.

            // The format of this line should be <config name> <C variable name> <type>
            const char* next = dr_next_token(line + 2, var.name, sizeof(var.name));     // Skip past "//"
            if (next == NULL) {
                nextLine = dr_next_line(nextLine);
                continue;
            }

            next = dr_next_token(next, var.varname, sizeof(var.varname));
            if (next == NULL) {
                nextLine = dr_next_line(nextLine);
                continue;
            }

            next = dr_next_token(next, var.typeSrc, sizeof(var.typeSrc));
            if (next == NULL) {
                nextLine = dr_next_line(nextLine);
                continue;
            }
            var.type = parse_config_var_type(var.typeSrc);

            next = dr_next_token(next, var.setCallback, sizeof(var.setCallback));
            if (next == NULL) {
                nextLine = dr_next_line(nextLine);
                continue;
            }


            // At this point "next" will be pointing to the default value. Just in case it is not present we fill out a default value first.
            get_config_var_default_value(var.type, var.defaultValue, sizeof(var.defaultValue));

            const char* defaultValue = next;
            strcpy_s(var.defaultValueSrc, sizeof(var.defaultValueSrc), defaultValue);

            char unused[1024];
            if (dr_next_token(next, unused, sizeof(unused))) {
                parse_config_var_value(var.type, defaultValue, var.defaultValue, sizeof(var.defaultValue));
            }
        }

        nextLine = dr_next_line(nextLine + lineLength);
    }

    // There may be a leftover config variable to add to the main list.
    if (var.name[0] != '\0') {
        stb_sb_push(g_ConfigVars, var);
    }
    

    char* declarationsOutput = dtk_make_string("\n\n#define DRED_CONFIG_VARIABLE_DECLARATIONS");
    for (int iVar = 0; iVar < stb_sb_count(g_ConfigVars); ++iVar) {
        config_var* pVar = &g_ConfigVars[iVar];
        declarationsOutput = dtk_append_string(declarationsOutput, " \\\n");
        declarationsOutput = dtk_append_string(declarationsOutput, config_var_type_to_string(pVar->type));
        declarationsOutput = dtk_append_string(declarationsOutput, " ");
        declarationsOutput = dtk_append_string(declarationsOutput, pVar->varname);
        declarationsOutput = dtk_append_string(declarationsOutput, ";");
    }
    declarationsOutput = dtk_append_string(declarationsOutput, "\n");   // <-- Need this new-line character to prevent a warning in GCC.
    fwrite_string(pFileOutH, declarationsOutput);


    char* funcOutput = dtk_make_string("\n\nvoid dred_config_init_variables__autogenerated(dred_config* pConfig)\n{\n");
    for (int iVar = 0; iVar < stb_sb_count(g_ConfigVars); ++iVar) {
        config_var* pVar = &g_ConfigVars[iVar];
        funcOutput = dtk_append_string(funcOutput, "    pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, " = ");
        funcOutput = dtk_append_string(funcOutput, pVar->defaultValue);
        funcOutput = dtk_append_string(funcOutput, ";\n");
    }
    funcOutput = dtk_append_string(funcOutput, "}");
    fwrite_string(pFileOut, funcOutput);
    dtk_free_string(funcOutput);


    funcOutput = dtk_make_string("\n\nvoid dred_config_uninit_variables__autogenerated(dred_config* pConfig)\n{\n");
    for (int iVar = 0; iVar < stb_sb_count(g_ConfigVars); ++iVar) {
        config_var* pVar = &g_ConfigVars[iVar];
        switch (pVar->type)
        {
            case CONFIG_VAR_TYPE_STRING:
            {
                funcOutput = dtk_append_string(funcOutput, "    dtk_free_string(pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
                funcOutput = dtk_append_string(funcOutput, "    pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, " = NULL;\n");
            } break;

            case CONFIG_VAR_TYPE_FONT:
            {
                funcOutput = dtk_append_string(funcOutput, "    dred_font_library_delete_font(&pConfig->pDred->fontLibrary, pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
                funcOutput = dtk_append_string(funcOutput, "    pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, " = NULL;\n");
            } break;

            case CONFIG_VAR_TYPE_IMAGE:
            {
                funcOutput = dtk_append_string(funcOutput, "    dred_image_library_delete_image(&pConfig->pDred->imageLibrary, pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
                funcOutput = dtk_append_string(funcOutput, "    pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, " = NULL;\n");
            } break;

            default: break;
        }
    }
    funcOutput = dtk_append_string(funcOutput, "}");
    fwrite_string(pFileOut, funcOutput);
    dtk_free_string(funcOutput);



    funcOutput = dtk_make_string("\n\nvoid dred_config_write_to_file__autogenerated(dred_config* pConfig, dred_file file)\n{\n");
    funcOutput = dtk_append_string(funcOutput, "    char tempbuf[4096];\n\n");
    funcOutput = dtk_append_string(funcOutput, "    char tempbuf2[4096];\n\n");
    for (int iVar = 0; iVar < stb_sb_count(g_ConfigVars); ++iVar) {
        config_var* pVar = &g_ConfigVars[iVar];

        switch (pVar->type)
        {
            case CONFIG_VAR_TYPE_INTEGER:
            {
                funcOutput = dtk_append_string(funcOutput, "    snprintf(tempbuf, sizeof(tempbuf), \""); funcOutput = dtk_append_string(funcOutput, pVar->name); funcOutput = dtk_append_string(funcOutput, " %d\\n\", ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_FLOAT:
            {
                funcOutput = dtk_append_string(funcOutput, "    snprintf(tempbuf, sizeof(tempbuf), \""); funcOutput = dtk_append_string(funcOutput, pVar->name); funcOutput = dtk_append_string(funcOutput, " %f\\n\", ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_BOOL:
            {
                funcOutput = dtk_append_string(funcOutput, "    snprintf(tempbuf, sizeof(tempbuf), \""); funcOutput = dtk_append_string(funcOutput, pVar->name); funcOutput = dtk_append_string(funcOutput, " %s\\n\", ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, " ? \"true\" : \"false\");\n");
            } break;

            case CONFIG_VAR_TYPE_STRING:
            {
                funcOutput = dtk_append_string(funcOutput, "    snprintf(tempbuf, sizeof(tempbuf), \""); funcOutput = dtk_append_string(funcOutput, pVar->name); funcOutput = dtk_append_string(funcOutput, " \\\"%s\\\"\\n\", ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_FONT:
            {
                funcOutput = dtk_append_string(funcOutput, "    dred_font_to_string(pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ", tempbuf2, sizeof(tempbuf2));\n");
                funcOutput = dtk_append_string(funcOutput, "    snprintf(tempbuf, sizeof(tempbuf), \""); funcOutput = dtk_append_string(funcOutput, pVar->name); funcOutput = dtk_append_string(funcOutput, " %s\\n\", tempbuf2);\n");
            } break;

            case CONFIG_VAR_TYPE_IMAGE:
            {
            } break;

            case CONFIG_VAR_TYPE_COLOR:
            {
                funcOutput = dtk_append_string(funcOutput, "    snprintf(tempbuf, sizeof(tempbuf), \""); funcOutput = dtk_append_string(funcOutput, pVar->name); funcOutput = dtk_append_string(funcOutput, " %d %d %d %d\\n\", ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ".r, ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ".g, ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ".b, ");
                funcOutput = dtk_append_string(funcOutput, "pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ".a");
                funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            default: break;
        }

        funcOutput = dtk_append_string(funcOutput, "    dred_file_write_string(file, tempbuf);\n\n");
    }
    funcOutput = dtk_append_string(funcOutput, "}\n\n");
    fwrite_string(pFileOut, funcOutput);



    funcOutput = dtk_make_string("\n\nvoid dred_config_set__autogenerated(dred_config* pConfig, const char* key, const char* value)\n{\n");
    for (int iVar = 0; iVar < stb_sb_count(g_ConfigVars); ++iVar) {
        config_var* pVar = &g_ConfigVars[iVar];

        funcOutput = dtk_append_string(funcOutput, "    if (strcmp(key, \"");
        funcOutput = dtk_append_string(funcOutput, pVar->name);
        funcOutput = dtk_append_string(funcOutput, "\") == 0) {\n");

        switch (pVar->type)
        {
            case CONFIG_VAR_TYPE_INTEGER:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = atoi(value);\n");
            } break;

            case CONFIG_VAR_TYPE_FLOAT:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = (float)atof(value);\n");
            } break;

            case CONFIG_VAR_TYPE_BOOL:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dred_parse_bool(value);\n");
            } break;

            case CONFIG_VAR_TYPE_STRING:
            {
                funcOutput = dtk_append_string(funcOutput, "        dtk_free_string(pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, ");\n");
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dtk_make_string(value);\n");
            } break;

            case CONFIG_VAR_TYPE_FONT:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dred_parse_and_load_font(pConfig->pDred, value);\n");
            } break;

            case CONFIG_VAR_TYPE_IMAGE:
            {
                // TODO: Implement this properly once a proper imaging system is ready.
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = NULL;\n");
            } break;

            case CONFIG_VAR_TYPE_COLOR:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dred_parse_color(value);\n");
            } break;

            default: break;
        }

        if (pVar->setCallback[0] != '\0' && strcmp(pVar->setCallback, "none") != 0) {
            funcOutput = dtk_append_string(funcOutput, "        if (pConfig->pDred->isInitialized) ");
            funcOutput = dtk_append_string(funcOutput, pVar->setCallback);
            funcOutput = dtk_append_string(funcOutput, "(pConfig->pDred);\n");
        }

        funcOutput = dtk_append_string(funcOutput, "        return;\n    }\n");
    }
    funcOutput = dtk_append_string(funcOutput, "\n    dred_warningf(pConfig->pDred, \"Unknown config variable: %s\\n\", key);\n");
    funcOutput = dtk_append_string(funcOutput, "}\n\n");
    fwrite_string(pFileOut, funcOutput);


    funcOutput = dtk_make_string("\n\nvoid dred_config_set_default__autogenerated(dred_config* pConfig, const char* key)\n{\n");
    for (int iVar = 0; iVar < stb_sb_count(g_ConfigVars); ++iVar) {
        config_var* pVar = &g_ConfigVars[iVar];

        funcOutput = dtk_append_string(funcOutput, "    if (strcmp(key, \"");
        funcOutput = dtk_append_string(funcOutput, pVar->name);
        funcOutput = dtk_append_string(funcOutput, "\") == 0) {\n");

        // For strings, the previous string needs to be free'd first.
        if (pVar->type == CONFIG_VAR_TYPE_STRING) {
            funcOutput = dtk_append_string(funcOutput, "        dtk_free_string(pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
            funcOutput = dtk_append_string(funcOutput, ");\n");
        }

        funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, " = ");
        funcOutput = dtk_append_string(funcOutput, pVar->defaultValue);
        funcOutput = dtk_append_string(funcOutput, ";\n");

        if (pVar->setCallback[0] != '\0' && strcmp(pVar->setCallback, "none") != 0) {
            funcOutput = dtk_append_string(funcOutput, "        if (pConfig->pDred->isInitialized) ");
            funcOutput = dtk_append_string(funcOutput, pVar->setCallback);
            funcOutput = dtk_append_string(funcOutput, "(pConfig->pDred);\n");
        }

        funcOutput = dtk_append_string(funcOutput, "        return;\n    }\n");
    }
    funcOutput = dtk_append_string(funcOutput, "}\n\n");
    fwrite_string(pFileOut, funcOutput);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    char exedir[256];
    dr_get_executable_directory_path(exedir, sizeof(exedir));
    dr_set_current_directory(exedir);

    FILE* pFileOut = dr_fopen("../../../source/dred/dred_autogenerated.c", "w+b");
    if (pFileOut == NULL) {
        printf("Failed to create output file.");
        return -1;
    }
    fwrite_string(pFileOut, "// Copyright (C) 2017 David Reid. See included LICENSE file.\n\n");
    fwrite_string(pFileOut, "// This file was automatically generated by the pre-build tool. Do not modify.\n\n");

    FILE* pFileOutH = dr_fopen("../../../source/dred/dred_autogenerated.h", "w+b");
    if (pFileOutH == NULL) {
        printf("Failed to create output file.");
        return -1;
    }
    fwrite_string(pFileOutH, "// Copyright (C) 2017 David Reid. See included LICENSE file.\n\n");
    fwrite_string(pFileOutH, "// This file was automatically generated by the pre-build tool. Do not modify.\n\n");

    // The string pool for building the initial data for the main string pool in dred.
    dred_string_pool stringPool;
    dred_string_pool_init(&stringPool, NULL, 0);

    // External libraries.
    dr_copy_file("../../../../../dr_libs/dr.h", "../../../source/external/dr.h", DR_FALSE);

    // Commands.
    generate_commands_list(pFileOut);

    // Stock images.
    generate_stock_images(pFileOut, pFileOutH);
    
    // Config vars.
    generate_config_vars(pFileOut, pFileOutH);

    // Shortcuts.
    dred_build__generate_shortcuts(pFileOut, pFileOutH, &stringPool);

    // Menus.
    dred_build__generate_menus(pFileOut, pFileOutH, &stringPool);


    // The main string pool.
    char* stringPoolStr = stringify_string_pool_data(&stringPool);
    fprintf(pFileOut, "%s", stringPoolStr);
    dtk_free_string(stringPoolStr);


    fclose(pFileOut);
    fclose(pFileOutH);


    // Website. (Temporarily disabled until the performance issue is resolved.)
    dred_build__generate_website(g_CommandVars, g_ConfigVars);


    return 0;
}