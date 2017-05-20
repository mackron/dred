// Copyright (C) 2017 David Reid. See included LICENSE file.

typedef struct
{
    char printerName[1024];
    unsigned int firstPage;
    unsigned int lastPage;
    unsigned int copies;
} dred_print_info;