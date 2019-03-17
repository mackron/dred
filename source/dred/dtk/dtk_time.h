// Copyright (C) 2019 David Reid. See included LICENSE file.

// Retrieves a time_t as of the time the function was called.
time_t dtk_now();

// Formats a data/time string.
size_t dtk_datetime_short(time_t t, char* strOut, size_t strOutSize);

// Returns a date string in YYYYMMDD format.
size_t dtk_date_YYYYMMDD(time_t t, char* strOut, size_t strOutSize);

// Retrieves the day of month of the given time_t structure.
int dtk_day(time_t t);

// Retrieves the 1-based month of the given time_t structure.
int dtk_month(time_t t);

// Retrieves the year of the given time_t structure.
int dtk_year(time_t t);