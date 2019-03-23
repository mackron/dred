// Copyright (C) 2019 David Reid. See included LICENSE file.

#define _CRT_SECURE_NO_WARNINGS

#include "../external/stretchy_buffer.h"
#include "../external/json.c"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#include "../dred/dred_build_config.h"

#include "../dred/dtk/dtk.c"

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
} stock_image;

dtk_bool32 stock_image_is_svg(const stock_image* pStockImage)
{
    return dtk_path_extension_equal(pStockImage->filename, "svg");
}

dtk_bool32 stock_image_is_raster(const stock_image* pStockImage)
{
    return !stock_image_is_svg(pStockImage);
}


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

dtk_bool32 dred_build_find_config_var(const char* name, size_t* pIndex)
{
    if (pIndex) *pIndex = 0;

    for (int i = 0; i < stb_sb_count(g_ConfigVars); ++i) {
        if (strcmp(g_ConfigVars[i].name, name) == 0) {
            if (pIndex) *pIndex = i;
            return DTK_TRUE;
        }
    }

    return DTK_FALSE;
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
        return "dtk_bool32";
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
    } else if (strcmp(type, "bool") == 0 || strcmp(type, "dtk_bool32") == 0) {
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
        strcpy_s(valueOut, valueOutSize, "DTK_FALSE");
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
        strcpy_s(valueOut, valueOutSize, "dtk_rgb(0, 0, 0)");
        return;
    }
}

void parse_config_var_value(unsigned int type, const char* valueIn, char* valueOut, size_t valueOutSize)
{
    valueIn = dtk_first_non_whitespace(valueIn);

    dtk_string str = dtk_string_replace(valueIn, "\"", "\\\"");

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

        valueIn = dtk_next_token(valueIn, r, sizeof(r));
        valueIn = dtk_next_token(valueIn, g, sizeof(g));
        valueIn = dtk_next_token(valueIn, b, sizeof(b));
        if (dtk_next_token(valueIn, a, sizeof(a)) == NULL) {
            a[0] = '2'; a[1] = '5'; a[2] = '5'; a[3] = '\0';
        }

        snprintf(valueOut, valueOutSize, "dtk_rgba(%s, %s, %s, %s)", r, g, b, a);
        return;
    }

    dtk_free_string(str);
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

char* write_image_data_string(char* output, unsigned int* pCurrentByteColumn, const char* str)
{
    unsigned int currentByteColumn = *pCurrentByteColumn;
    char byteStr[5] = {'0', 'x', '0', '0', '\0'};

    size_t len = strlen(str);
    for (size_t i = 0; i < len+1; ++i) {    // +1 to include null terminator.
        if (currentByteColumn == 0) {
            output = dtk_append_string(output, "\n    ");
        }

        uint8_t b = (uint8_t)str[i];
        byteStr[2] = ((b >>  4) + '0'); if (byteStr[2] > '9') byteStr[2] += 7;
        byteStr[3] = ((b & 0xF) + '0'); if (byteStr[3] > '9') byteStr[3] += 7;
        output = dtk_append_string(output, byteStr);

        currentByteColumn = (currentByteColumn + 1) % BYTES_PER_ROW;

        if (i < len) {
            output = dtk_append_string(output, ",");
        }
    }

    *pCurrentByteColumn = currentByteColumn;
    return output;
}


char* stringify_string_pool_data(dtk_string_pool* pStringPool)
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
    char* fileData;
    dtk_result result = dtk_open_and_read_text_file("../../../source/dred/dred_commands.h", &fileDataSize, &fileData);
    if (result != DTK_SUCCESS) {
        return;
    }

    // Look for the line beginning with "// BEGIN COMMAND LIST"
    char line[1024];
    const char* nextLine = fileData;
    while (nextLine != NULL) {
        if (dtk_copy_line(nextLine, line, sizeof(line)) == (size_t)-1) {
            return;
        }
        if (strstr(line, "// BEGIN COMMAND LIST") != NULL) {
            nextLine = dtk_next_line(nextLine);
            break;
        }
        nextLine = dtk_next_line(nextLine);
    }



    char* CommandNamePool = dtk_make_string("const char g_CommandNamePool[] = ");
    char* CommandNames    = dtk_make_string("const char* g_CommandNames[] = {\n");
    char* Commands        = dtk_make_string("dred_command g_Commands[] = {\n");

    size_t runningNameLength = 0;
    int commandCount = 0;

    while (nextLine != NULL) {
        // The next line should be in the format of <name> <proc> <flags>
        size_t lineLength = dtk_copy_line(nextLine, line, sizeof(line));
        if (lineLength <= 2) {
            nextLine = dtk_next_line(nextLine);
            continue;
        }

        if (strstr(line, "// END COMMAND LIST") != NULL) {
            break;
        }


        command_var command;
        const char* next = dtk_next_token(line + 2, command.name, sizeof(command.name));     // Skip past "//"
        if (next == NULL) {
            nextLine = dtk_next_line(nextLine);
            continue;
        }

        next = dtk_next_token(next, command.proc, sizeof(command.proc));
        if (next == NULL) {
            nextLine = dtk_next_line(nextLine);
            continue;
        }

        next = dtk_next_token(next, command.flags, sizeof(command.flags));
        if (next == NULL) {
            nextLine = dtk_next_line(nextLine);
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

        nextLine = dtk_next_line(nextLine + lineLength);
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
    char* sourceFileData;
    dtk_result result = dtk_open_and_read_text_file("../../../source/dred/dred_image_library.h", &sourceFileDataSize, &sourceFileData);
    if (result != DTK_SUCCESS) {
        return;
    }

    // Look for the line beginning with "// BEGIN COMMAND LIST"
    char line[1024];
    const char* nextLine = sourceFileData;
    while (nextLine != NULL) {
        if (dtk_copy_line(nextLine, line, sizeof(line)) == (size_t)-1) {
            return;
        }
        if (strstr(line, "// BEGIN STOCK IMAGE LIST") != NULL) {
            nextLine = dtk_next_line(nextLine);
            break;
        }
        nextLine = dtk_next_line(nextLine);
    }

    unsigned int imageCount = 0;
    unsigned int imageCountSVG = 0;
    unsigned int imageCountRaster = 0;
    stock_image* pStockImages = NULL;

    while (nextLine != NULL) {
        // The next line should be in the format of <path> <id>
        size_t lineLength = dtk_copy_line(nextLine, line, sizeof(line));
        if (lineLength <= 2) {
            nextLine = dtk_next_line(nextLine);
            continue;
        }

        if (strstr(line, "// END STOCK IMAGE LIST") != NULL) {
            break;
        }

        stock_image stockImage;

        const char* next = dtk_next_token(line + 2, stockImage.filename, sizeof(stockImage.filename));     // Skip past "//"
        if (next == NULL) {
            nextLine = dtk_next_line(nextLine);
            continue;
        }

        next = dtk_next_token(next, stockImage.id, sizeof(stockImage.id));
        if (next == NULL) {
            nextLine = dtk_next_line(nextLine);
            continue;
        }

        pStockImages = realloc(pStockImages, (imageCount+1) * sizeof(stock_image));
        pStockImages[imageCount] = stockImage;
        imageCount += 1;

        if (stock_image_is_svg(&stockImage)) {
            imageCountSVG += 1;
        } else {
            imageCountRaster += 1;
        }

        nextLine = dtk_next_line(nextLine + lineLength);
    }


    // At this point we have the images so now we need to generate the code.
    fwrite_string(pFileOut, "\n// Stock Images\n");
    fprintf(pFileOutH, "#define DRED_STOCK_IMAGE_COUNT %d\n", imageCount);
    fprintf(pFileOut,  "#define DRED_STOCK_IMAGE_COUNT_SVG    %d\n", imageCountSVG);
    fprintf(pFileOut,  "#define DRED_STOCK_IMAGE_COUNT_RASTER %d\n", imageCountRaster);

    char* StockImageDataSVG;
    char* StockImagesSVG;
    if (imageCountSVG > 0) {
        StockImageDataSVG = dtk_make_string("const dtk_uint8 g_StockImageDataSVG[] = {");
        StockImagesSVG    = dtk_make_string("const dred_image_desc_svg g_StockImagesSVG[DRED_STOCK_IMAGE_COUNT_SVG] = {\n");
    } else {
        StockImageDataSVG = dtk_make_string("const dtk_uint8 g_StockImageDataSVG[1] = {0}; // No SVG images.\n");
        StockImagesSVG    = dtk_make_string("const dred_image_desc_svg g_StockImagesSVG[1] = {0};\n\n");
    }

    char* StockImageDataRaster;
    char* StockImagesRaster;
    if (imageCountRaster > 0) {
        StockImageDataRaster = dtk_make_string("const dtk_uint8 g_StockImageDataRaster[] = {");
        StockImagesRaster    = dtk_make_string("const dred_image_desc_raster g_StockImagesRaster[DRED_STOCK_IMAGE_COUNT_RASTER] = {\n");
    } else {
        StockImageDataRaster = dtk_make_string("const dtk_uint8 g_StockImageDataRaster[1] = {0}; // No raster images.\n");
        StockImagesRaster    = dtk_make_string("const dred_image_desc_raster g_StockImagesRaster[1] = {0};\n\n");
    }


    unsigned int currentByteColumn;
    unsigned int runningDataOffset;
    unsigned int runningIndex = 0;

    // SVG images
    currentByteColumn = 0;
    runningDataOffset = 0;
    unsigned int iImageSVG = 0;
    for (unsigned int iStockImage = 0; iStockImage < imageCount; ++iStockImage) {
        const stock_image* pStockImage = &pStockImages[iStockImage];
        if (stock_image_is_svg(pStockImage)) {
            fprintf(pFileOutH, "#define %s %d\n", pStockImage->id, runningIndex + DTK_STOCK_IMAGE_COUNT);

            // For SVG's, the image data is just the content of the file.
            char filename[256];
            dtk_path_append(filename, sizeof(filename), "../../../resources/images", pStockImage->filename);

            size_t svgSizeInBytes;
            char* svg;
            if (dtk_open_and_read_text_file(filename, &svgSizeInBytes, &svg) != DTK_SUCCESS) {
                printf("ERROR: Failed to open SVG image file: %s\n", filename);
                break;
            }

            if (iImageSVG > 0) {
                StockImagesSVG = dtk_append_string(StockImagesSVG, ",\n");
            }

            snprintf(line, sizeof(line), "    {(char*)(g_StockImageDataSVG + %d)}", runningDataOffset);
            StockImagesSVG = dtk_append_string(StockImagesSVG, line);

            StockImageDataSVG = write_image_data_string(StockImageDataSVG, &currentByteColumn, svg);
            runningDataOffset += (unsigned int)svgSizeInBytes;

            if (iImageSVG < imageCountSVG-1) {
                StockImageDataSVG = dtk_append_string(StockImageDataSVG, ",");
            }

            iImageSVG += 1;
            runningIndex += 1;
        }
    }

    if (imageCountSVG > 0) {
        StockImageDataSVG = dtk_append_string(StockImageDataSVG, "\n};\n\n");
        StockImagesSVG    = dtk_append_string(StockImagesSVG, "\n};\n\n");
    }

    fwrite_string(pFileOut, StockImageDataSVG);
    fwrite_string(pFileOut, StockImagesSVG);


    // Raster images.
    currentByteColumn = 0;
    runningDataOffset = 0;
    unsigned int iImageRaster = 0;
    for (unsigned int iStockImage = 0; iStockImage < imageCount; ++iStockImage) {
        const stock_image* pStockImage = &pStockImages[iStockImage];
        if (stock_image_is_raster(pStockImage)) {
            fprintf(pFileOutH, "#define %s %d\n", pStockImage->id, runningIndex + DTK_STOCK_IMAGE_COUNT);

            char filename[256];
            dtk_path_append(filename, sizeof(filename), "../../../resources/images", pStockImage->filename);

            int sizeX;
            int sizeY;
            unsigned char* pImageData = stbi_load(filename, &sizeX, &sizeY, NULL, 4);
            if (pImageData == NULL) {
                printf("ERROR: Failed to open raster image file: %s\n", filename);
                break;
            }


            if (iImageRaster > 0) {
                StockImagesRaster = dtk_append_string(StockImagesRaster, ",\n");
            }

            snprintf(line, sizeof(line), "    {%d, %d, g_StockImageDataRaster + %d}", sizeX, sizeY, runningDataOffset);
            StockImagesRaster = dtk_append_string(StockImagesRaster, line);

            StockImageDataRaster = write_image_data_rgba8(StockImageDataRaster, &currentByteColumn, pImageData, sizeX, sizeY, sizeX*4);
            runningDataOffset += (unsigned int)(sizeX*sizeY*4);

            if (iImageRaster < imageCountRaster-1) {
                StockImageDataRaster = dtk_append_string(StockImageDataSVG, ",");
            }

            iImageRaster += 1;
            runningIndex += 1;
        }
    }

    if (imageCountRaster > 0) {
        StockImageDataRaster = dtk_append_string(StockImageDataRaster, "\n};\n\n");
        StockImagesRaster = dtk_append_string(StockImagesRaster, "\n};\n\n");
    }

    fwrite_string(pFileOut, StockImageDataRaster);
    fwrite_string(pFileOut, StockImagesRaster);
}

void generate_config_vars(FILE* pFileOut, FILE* pFileOutH)
{
    assert(pFileOut != NULL);
    assert(pFileOutH != NULL);

    size_t sourceFileDataSize;
    char* sourceFileData;
    dtk_result result = dtk_open_and_read_text_file("../../../source/dred/dred_config.h", &sourceFileDataSize, &sourceFileData);
    if (result != DTK_SUCCESS) {
        return;
    }

    // Look for the line beginning with "// BEGIN CONFIG VARS"
    char line[4096];
    const char* nextLine = sourceFileData;
    while (nextLine != NULL) {
        if (dtk_copy_line(nextLine, line, sizeof(line)) == (size_t)-1) {
            return;
        }
        if (strstr(line, "// BEGIN CONFIG VARS") != NULL) {
            nextLine = dtk_next_line(nextLine);
            break;
        }
        nextLine = dtk_next_line(nextLine);
    }


    config_var var;
    memset(&var, 0, sizeof(var));

    while (nextLine != NULL) {
        size_t lineLength = dtk_copy_line(nextLine, line, sizeof(line));
        if (lineLength <= 2) {
            nextLine = dtk_next_line(nextLine);
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
            const char* next = dtk_next_token(line + 2, var.name, sizeof(var.name));     // Skip past "//"
            if (next == NULL) {
                nextLine = dtk_next_line(nextLine);
                continue;
            }

            next = dtk_next_token(next, var.varname, sizeof(var.varname));
            if (next == NULL) {
                nextLine = dtk_next_line(nextLine);
                continue;
            }

            next = dtk_next_token(next, var.typeSrc, sizeof(var.typeSrc));
            if (next == NULL) {
                nextLine = dtk_next_line(nextLine);
                continue;
            }
            var.type = parse_config_var_type(var.typeSrc);

            next = dtk_next_token(next, var.setCallback, sizeof(var.setCallback));
            if (next == NULL) {
                nextLine = dtk_next_line(nextLine);
                continue;
            }


            // At this point "next" will be pointing to the default value. Just in case it is not present we fill out a default value first.
            get_config_var_default_value(var.type, var.defaultValue, sizeof(var.defaultValue));

            const char* defaultValue = next;
            strcpy_s(var.defaultValueSrc, sizeof(var.defaultValueSrc), defaultValue);

            char unused[1024];
            if (dtk_next_token(next, unused, sizeof(unused))) {
                parse_config_var_value(var.type, defaultValue, var.defaultValue, sizeof(var.defaultValue));
            }
        }

        nextLine = dtk_next_line(nextLine + lineLength);
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



    funcOutput = dtk_make_string("\n\nvoid dred_config_write_to_file__autogenerated(dred_config* pConfig, FILE* file)\n{\n");
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

        funcOutput = dtk_append_string(funcOutput, "    dtk_fwrite_string(file, tempbuf);\n\n");
    }
    funcOutput = dtk_append_string(funcOutput, "}\n\n");
    fwrite_string(pFileOut, funcOutput);



    funcOutput = dtk_make_string("\n\nvoid dred_config_set__autogenerated(dred_config* pConfig, const char* key, const char* value, dtk_uint32 flags)\n{\n");
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

                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_int(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_FLOAT:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = (float)atof(value);\n");

                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_double(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_BOOL:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dred_parse_bool(value);\n");
                
                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_bool(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_STRING:
            {
                funcOutput = dtk_append_string(funcOutput, "        dtk_free_string(pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, ");\n");
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dtk_make_string(value);\n");

                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_string(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_FONT:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dred_parse_and_load_font(pConfig->pDred, value);\n");

                // TODO: Update bindings.
            } break;

            case CONFIG_VAR_TYPE_IMAGE:
            {
                // TODO: Implement this properly once a proper imaging system is ready.
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = NULL;\n");

                // TODO: Update bindings.
            } break;

            case CONFIG_VAR_TYPE_COLOR:
            {
                funcOutput = dtk_append_string(funcOutput, "        pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname);
                funcOutput = dtk_append_string(funcOutput, " = dtk_parse_color(value);\n");

                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_color(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
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


    funcOutput = dtk_make_string("\n\nvoid dred_config_set_default__autogenerated(dred_config* pConfig, const char* key, dtk_uint32 flags)\n{\n");
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

        switch (pVar->type)
        {
            case CONFIG_VAR_TYPE_INTEGER:
            {
                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_int(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_FLOAT:
            {
                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_double(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_BOOL:
            {
                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_bool(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_STRING:
            {
                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_string(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
            } break;

            case CONFIG_VAR_TYPE_FONT:
            {
                // TODO: Update bindings.
            } break;

            case CONFIG_VAR_TYPE_IMAGE:
            {
                // TODO: Update bindings.
            } break;

            case CONFIG_VAR_TYPE_COLOR:
            {
                funcOutput = dtk_append_string(funcOutput, "        if ((flags & DRED_CONFIG_NO_UPDATE_BINDINGS) == 0) dtk_update_bindings_color(&pConfig->pDred->tk, NULL, \"config.");
                funcOutput = dtk_append_string(funcOutput, pVar->name);
                funcOutput = dtk_append_string(funcOutput, "\", pConfig->"); funcOutput = dtk_append_string(funcOutput, pVar->varname); funcOutput = dtk_append_string(funcOutput, ");\n");
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
    funcOutput = dtk_append_string(funcOutput, "}\n\n");
    fwrite_string(pFileOut, funcOutput);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    char exedir[256];
    dtk_get_executable_directory_path(exedir, sizeof(exedir));
    dtk_set_current_directory(exedir);

    FILE* pFileOut;
    dtk_result result = dtk_fopen(&pFileOut, "../../../source/dred/dred_autogenerated.c", "w+b");
    if (result != DTK_SUCCESS) {
        printf("Failed to create output file.");
        return -1;
    }
    fwrite_string(pFileOut, "// Copyright (C) 2019 David Reid. See included LICENSE file.\n\n");
    fwrite_string(pFileOut, "// This file was automatically generated by the pre-build tool. Do not modify.\n\n");

    FILE* pFileOutH;
    result = dtk_fopen(&pFileOutH, "../../../source/dred/dred_autogenerated.h", "w+b");
    if (result != DTK_SUCCESS) {
        printf("Failed to create output file.");
        return -1;
    }
    fwrite_string(pFileOutH, "// Copyright (C) 2019 David Reid. See included LICENSE file.\n\n");
    fwrite_string(pFileOutH, "// This file was automatically generated by the pre-build tool. Do not modify.\n\n");

    // The string pool for building the initial data for the main string pool in dred.
    dtk_string_pool stringPool;
    dtk_string_pool_init(&stringPool, NULL, 0);


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