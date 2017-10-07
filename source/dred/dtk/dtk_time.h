// Copyright (C) 2017 David Reid. See included LICENSE file.

// Retrieves a time_t as of the time the function was called.
time_t dtk_now();

// Formats a data/time string.
size_t dtk_datetime_short(time_t t, char* strOut, size_t strOutSize);

// Returns a date string in YYYYMMDD format.
size_t dtk_date_YYYYMMDD(time_t t, char* strOut, size_t strOutSize);