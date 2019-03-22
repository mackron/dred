// Copyright (C) 2019 David Reid. See included LICENSE file.

// Command: dred -f file2chex [Input File Name] [Options]
//    -n OutputVariableName : Specifies the name the name of the output variable. Defaults to "FileData"
//
// Implementation: dred_file2chex

// dred -f file2chex
int dred_file2chex(int argc, char** argv)
{
    if (argc <= 1) {
        return -1;  // No file specified.
    }

    const char* variableName = "FileData";
    if (argc > 3 && strcmp(argv[2], "-n") == 0) {
        variableName = argv[3];
    }

    FILE* pFile;
    dtk_result result = dtk_fopen(&pFile, argv[1], dtk_fopenmode(DTK_OPEN_MODE_READ));
    if (result != DTK_SUCCESS) {
        return -2;  // Could not open file.
    }

    fseek(pFile, 0, SEEK_END);
    long fileSize = ftell(pFile);
    if (fileSize == -1) {
        dtk_fclose(pFile);
        return -3;  // Failed to retrieve the size of the file.
    }
    fseek(pFile, 0, SEEK_SET);

    unsigned char* pFileData = (unsigned char*)malloc(fileSize);
    size_t bytesRead = fread(pFileData, 1, fileSize, pFile);
	if (bytesRead != (size_t)fileSize) {
		free(pFileData);
		dtk_fclose(pFile);
		return -4;
	}
    dtk_fclose(pFile);

    char* pHexData = dred_codegen_buffer_to_c_array(pFileData, fileSize, variableName);
    free(pFileData);

    if (pHexData == NULL) {
        return -5;
    }

    printf("%s", pHexData);
    free(pHexData);
    return 0;
}
