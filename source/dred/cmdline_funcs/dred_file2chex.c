// Copyright (C) 2017 David Reid. See included LICENSE file.

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

#ifdef _MSC_VER
    FILE* pFile;
    if (fopen_s(&pFile, argv[1], "rb") != 0) {
        return -2;  // Could not find file.
    }
#else
    FILE* pFile = fopen(argv[1], "rb");
    if (pFile == NULL) {
        return -2;  // Could not find file.
    }
#endif

    fseek(pFile, 0, SEEK_END);
    long fileSize = ftell(pFile);
    if (fileSize == -1) {
        fclose(pFile);
        return -3;  // Failed to retrieve the size of the file.
    }
    fseek(pFile, 0, SEEK_SET);

    unsigned char* pFileData = (unsigned char*)malloc(fileSize);
    size_t bytesRead = fread(pFileData, 1, fileSize, pFile);
	if (bytesRead != (size_t)fileSize) {
		free(pFileData);
		fclose(pFile);
		return -4;
	}
    fclose(pFile);

    char* pHexData = dred_codegen_buffer_to_c_array(pFileData, fileSize, variableName);
    free(pFileData);

    if (pHexData == NULL) {
        return -5;
    }

    printf("%s", pHexData);
    free(pHexData);
    return 0;
}
