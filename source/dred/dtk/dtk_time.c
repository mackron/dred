// Copyright (C) 2018 David Reid. See included LICENSE file.

time_t dtk_now()
{
    return time(NULL);
}

size_t dtk_datetime_short(time_t t, char* strOut, size_t strOutSize)
{
#if defined(_MSC_VER)
	time_t local;
	localtime_s(&local, &t);
    return strftime(strOut, strOutSize, "%x %H:%M:%S", &local);
#else
	struct tm *local = localtime(&t);
	return strftime(strOut, strOutSize, "%x %H:%M:%S", local);
#endif
}

size_t dtk_date_YYYYMMDD(time_t t, char* strOut, size_t strOutSize)
{
#if defined(_MSC_VER)
	time_t local;
	localtime_s(&local, &t);
    return strftime(strOut, strOutSize, "%Y%m%d", &local);
#else
	struct tm *local = localtime(&t);
	return strftime(strOut, strOutSize, "%Y%m%d", local);
#endif
}
