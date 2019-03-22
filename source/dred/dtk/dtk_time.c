// Copyright (C) 2019 David Reid. See included LICENSE file.

time_t dtk_now()
{
    return time(NULL);
}

size_t dtk_datetime_short(time_t t, char* strOut, size_t strOutSize)
{
#if defined(_MSC_VER)
	struct tm local;
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
	struct tm local;
	localtime_s(&local, &t);
    return strftime(strOut, strOutSize, "%Y%m%d", &local);
#else
	struct tm *local = localtime(&t);
	return strftime(strOut, strOutSize, "%Y%m%d", local);
#endif
}

dtk_result dtk_localtime(struct tm* lt, const time_t* t)
{
#ifdef _MSC_VER
    return dtk_result_from_errno(localtime_s(lt, t));
#else
    if (lt == NULL) {
        return DTK_INVALID_ARGS;
    }

    struct tm* _lt = localtime(t);
    if (_lt != NULL) {
        *lt = *_lt;
    } else {
        return DTK_ERROR;
    }
#endif
}

int dtk_day(time_t t)
{
    struct tm lt;
    dtk_localtime(&lt, &t);
    return lt.tm_mday;
}

int dtk_month(time_t t)
{
    struct tm lt;
    dtk_localtime(&lt, &t);
    return lt.tm_mon + 1;
}

int dtk_year(time_t t)
{
    struct tm lt;
    dtk_localtime(&lt, &t);
    return lt.tm_year + 1900;
}