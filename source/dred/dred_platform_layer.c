// Copyright (C) 2018 David Reid. See included LICENSE file.

//////////////////////////////////////////////////////////////////
//
// Win32
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_WIN32
//// Drag and Drop ////
dtk_bool32 dred_begin_drag_and_drop__win32(dred_data_type dataType, const void* pData, size_t dataSize)
{
#if 0
    IDataObjectVtbl dataObject;
    dataObject.QueryInterface = NULL;
	dataObject.AddRef = NULL;
	dataObject.Release = NULL;
	dataObject.GetData = NULL;
	dataObject.GetDataHere = NULL;
	dataObject.QueryGetData = NULL;
	dataObject.GetCanonicalFormatEtc = NULL;
	dataObject.SetData = NULL;
	dataObject.EnumFormatEtc = NULL;
	dataObject.DAdvise = NULL;
	dataObject.DUnadvise = NULL;
	dataObject.EnumDAdvise = NULL;
    DoDragDrop();
#endif

    (void)dataType;
    (void)pData;
    (void)dataSize;
    return DTK_FALSE;
}
#endif




//////////////////////////////////////////////////////////////////
//
// GTK+ 3
//
//////////////////////////////////////////////////////////////////

#ifdef DRED_GTK
//// Drag and Drop ////
dtk_bool32 dred_begin_drag_and_drop__gtk(dred_data_type dataType, const void* pData, size_t dataSize)
{
    (void)dataType;
    (void)pData;
    (void)dataSize;
    return DTK_FALSE;
}
#endif



//// Drag and Drop ////

dtk_bool32 dred_begin_drag_and_drop(dred_data_type dataType, const void* pData, size_t dataSize)
{
    if (pData == NULL) return DTK_FALSE;

#ifdef DRED_WIN32
    return dred_begin_drag_and_drop__win32(dataType, pData, dataSize);
#endif
#ifdef DRED_GTK
    return dred_begin_drag_and_drop__gtk(dataType, pData, dataSize);
#endif
}
