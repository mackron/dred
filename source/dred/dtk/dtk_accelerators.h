// Copyright (C) 2019 David Reid. See included LICENSE file.

typedef struct
{
    dtk_key key;
    dtk_uint32 modifiers;
    dtk_uint32 id;
} dtk_accelerator;

DTK_INLINE dtk_accelerator dtk_accelerator_init(dtk_key key, dtk_uint32 modifiers, dtk_uint32 id)
{
    dtk_accelerator a;
    a.key = key;
    a.modifiers = modifiers;
    a.id = id;
    return a;
}

DTK_INLINE dtk_accelerator dtk_accelerator_none()
{
    return dtk_accelerator_init(0, 0, 0);
}

DTK_INLINE dtk_bool32 dtk_accelerator_equal(dtk_accelerator a, dtk_accelerator b)
{
    // Normalize tabs.
    if (a.key == '\t') a.key = DTK_KEY_TAB;
    if (b.key == '\t') b.key = DTK_KEY_TAB;
    return a.key == b.key && a.modifiers == b.modifiers;
}

// Converts a string to an accelerator object.
dtk_result dtk_accelerator_parse(const char* accelStr, dtk_accelerator* pAccelerator);

// Converts a string to an accelerator chord. On input, pCount is the number of dtk_accelerator objects
// available in pAccelerators. On output it is set to the number of accelerators that were output, or
// would have been had there been enough room.
dtk_result dtk_accelerator_parse_chord(const char* accelStr, dtk_accelerator* pAccelerators, dtk_uint32* pCount);

// Converts an accelerator to it's string form. Useful for displaying the accelerator on menu items.
size_t dtk_accelerator_to_string(dtk_accelerator accelerator, char* strOut, size_t strOutSize);