
#define DR_IMPLEMENTATION
#include "../../../dr_libs/dr.h"

#define DR_PATH_IMPLEMENTATION
#include "../../../dr_libs/dr_path.h"

#define GB_STRING_IMPLEMENTATION
#include "../../source/gb_string.h"

#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <stdio.h>
#include <assert.h>

double g_StockImageScales[] = {1.0, 1.5, 2.0};
#define STOCK_IMAGE_SCALE_COUNT (sizeof(g_StockImageScales) / sizeof(g_StockImageScales[0]))

#define BYTES_PER_ROW   16

typedef struct
{
    char filename[256];
    char id[256];
    unsigned int baseWidth;
    unsigned int baseHeight;
} stock_image;

char* write_image_data_rgba8(char* output, unsigned int* pCurrentByteColumn, const uint8_t* pImageData, unsigned int width, unsigned int height, unsigned int stride)
{
    unsigned int currentByteColumn = *pCurrentByteColumn;

    char byteStr[5] = {'0', 'x', '0', '0', '\0'};

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            for (unsigned int c = 0; c < 4; ++c) {
                if (currentByteColumn == 0) {
                    output = gb_append_cstring(output, "\n    ");
                }

                uint8_t b = pImageData[y*stride + x*4 + c];
                byteStr[2] = ((b >>  4) + '0'); if (byteStr[2] > '9') byteStr[2] += 7;
                byteStr[3] = ((b & 0xF) + '0'); if (byteStr[3] > '9') byteStr[3] += 7;
                output = gb_append_cstring(output, byteStr);

                currentByteColumn = (currentByteColumn + 1) % BYTES_PER_ROW;

                if (y+1 != height || x+1 != width || c+1 != 4) {
                    output = gb_append_cstring(output, ",");
                }
            }
        }
    }

    *pCurrentByteColumn = currentByteColumn;
    return output;
}


void fwrite_string(FILE* pFile, const char* str)
{
    fwrite(str, 1, strlen(str), pFile);
}

void generate_commands_list(FILE* pFileOut)
{
    assert(pFileOut != NULL);
    
    size_t fileDataSize;
    char* fileData = dr_open_and_read_text_file("../source/dred_commands.h", &fileDataSize);
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


    char* CommandNamePool = gb_make_string("const char g_CommandNamePool[] = ");
    char* CommandNames    = gb_make_string("const char* g_CommandNames[] = {\n");
    char* Commands        = gb_make_string("dred_command g_Commands[] = {\n");

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

        
        char name[256];
        const char* next = dr_next_token(line + 2, name, sizeof(name));     // Skip past "//"
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        char proc[256];
        next = dr_next_token(next, proc, sizeof(proc));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }

        char flags[256];
        next = dr_next_token(next, flags, sizeof(flags));
        if (next == NULL) {
            nextLine = dr_next_line(nextLine);
            continue;
        }


        snprintf(line, sizeof(line), "\n    \"%s\\0\"", name);
        CommandNamePool = gb_append_cstring(CommandNamePool, line);

        snprintf(line, sizeof(line), "    g_CommandNamePool + %d,\n", (int)runningNameLength);
        CommandNames = gb_append_cstring(CommandNames, line);

        snprintf(line, sizeof(line), "    {%s, %s},\n", proc, flags);
        Commands = gb_append_cstring(Commands, line);

        runningNameLength += strlen(name)+1;
        commandCount += 1;

        nextLine = dr_next_line(nextLine + lineLength);
    }

    fwrite_string(pFileOut, "\n// Commands\n");
    snprintf(line, sizeof(line), "#define DRED_COMMAND_COUNT %d\n\n", commandCount);
    fwrite_string(pFileOut, line);

    CommandNamePool = gb_append_cstring(CommandNamePool, ";\n\n");
    CommandNames    = gb_append_cstring(CommandNames,    "};\n\n");
    Commands        = gb_append_cstring(Commands,        "};\n\n");

    fwrite_string(pFileOut, CommandNamePool);
    fwrite_string(pFileOut, CommandNames);
    fwrite_string(pFileOut, Commands);
}

void generate_stock_images(FILE* pFileOut, FILE* pFileOutH)
{
    assert(pFileOut != NULL);
    assert(pFileOutH != NULL);

    size_t sourceFileDataSize;
    char* sourceFileData = dr_open_and_read_text_file("../source/dred_image_library.h", &sourceFileDataSize);
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

    snprintf(line, sizeof(line), "#define DRED_STOCK_IMAGE_SCALE_COUNT %d\n\n", STOCK_IMAGE_SCALE_COUNT);
    fwrite_string(pFileOut, line);

    char* StockImageData = gb_make_string("const uint8_t g_StockImageData[] = {");
    char* StockImages    = gb_make_string("const dred_image_desc g_StockImages[DRED_STOCK_IMAGE_COUNT][DRED_STOCK_IMAGE_SCALE_COUNT] = {");

    unsigned int currentByteColumn = 0;
    unsigned int runningDataOffset = 0;

    for (unsigned int iStockImage = 0; iStockImage < imageCount; ++iStockImage)
    {
        stock_image* pStockImage = &pStockImages[iStockImage];

        snprintf(line, sizeof(line), "#define %s %d\n", pStockImage->id, iStockImage);
        fwrite_string(pFileOutH, line);

        char* imageStr = gb_make_string("");
        if (iStockImage > 0) {
            imageStr = gb_append_cstring(imageStr, ",");
        }

        imageStr = gb_append_cstring(imageStr, "\n    {");


        // Load the image and rasterize it.
        char filename[256];
        drpath_copy_and_append(filename, sizeof(filename), "../resources/images", pStockImage->filename);

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
                imageStr = gb_append_cstring(imageStr, ",\n     ");
            }

            snprintf(line, sizeof(line), "{%.1ff, %d, %d, %s, g_StockImageData + %d}", scale, scaledWidth, scaledHeight, "drgui_image_format_rgba8", runningDataOffset);
            imageStr = gb_append_cstring(imageStr, line);

            
            nsvgRasterize(pSVGRast, pSVGImage, 0, 0, scaledWidth / pSVGImage->width, pImageData, (int)svgWidth, (int)svgHeight, (int)svgWidth*4);
            
            
            StockImageData = write_image_data_rgba8(StockImageData, &currentByteColumn, pImageData, scaledWidth, scaledHeight, svgWidth*4);
            if (iStockImage+1 != imageCount || iScale+1 != STOCK_IMAGE_SCALE_COUNT) {
                StockImageData = gb_append_cstring(StockImageData, ",");
            }

            runningDataOffset += (scaledWidth * scaledHeight * 4);
        }

        nsvgDeleteRasterizer(pSVGRast);
        nsvgDelete(pSVGImage);
        dr_free_file_data(svg);
        free(pImageData);


        imageStr = gb_append_cstring(imageStr, "}");

        StockImages = gb_append_cstring(StockImages, imageStr);
        gb_free_string(imageStr);
    }

    StockImageData = gb_append_cstring(StockImageData, "\n};\n\n");
    fwrite_string(pFileOut, StockImageData);

    StockImages = gb_append_cstring(StockImages, "\n};\n\n");
    fwrite_string(pFileOut, StockImages);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    char exedir[256];
    dr_get_executable_directory_path(exedir, sizeof(exedir));
    dr_set_current_directory(exedir);

    FILE* pFileOut = dr_fopen("../source/dred_autogenerated.c", "w+b");
    if (pFileOut == NULL) {
        printf("Failed to create output file.");
        return -1;
    }
    fwrite_string(pFileOut, "// This file was automatically generated by the pre-build tool. Do not modify.\n\n");

    FILE* pFileOutH = dr_fopen("../source/dred_autogenerated.h", "w+b");
    if (pFileOutH == NULL) {
        printf("Failed to create output file.");
        return -1;
    }
    fwrite_string(pFileOutH, "// This file was automatically generated by the pre-build tool. Do not modify.\n\n");


    // Commands.
    generate_commands_list(pFileOut);

    // Stock images.
    generate_stock_images(pFileOut, pFileOutH);


    fclose(pFileOut);
    fclose(pFileOutH);

    return 0;
}