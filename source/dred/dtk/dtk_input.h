// Copyright (C) 2017 David Reid. See included LICENSE file.

// Modifier flags. These will be passed to the "state" flags for mouse and key events.
#define DTK_MODIFIER_MOUSE_BUTTON_LEFT      (1 << 0)
#define DTK_MODIFIER_MOUSE_BUTTON_RIGHT     (1 << 1)
#define DTK_MODIFIER_MOUSE_BUTTON_MIDDLE    (1 << 2)
#define DTK_MODIFIER_MOUSE_BUTTON_4         (1 << 3)
#define DTK_MODIFIER_MOUSE_BUTTON_5         (1 << 4)
#define DTK_MODIFIER_SHIFT                  (1 << 5)
#define DTK_MODIFIER_CTRL                   (1 << 6)
#define DTK_MODIFIER_ALT                    (1 << 7)

// Key state flags. These will be passed to the "state" flags for key events.
#define DTK_KEY_STATE_AUTO_REPEATED         (1 << 31)



///////////////////////////////////////////////////////////////////////////////
//
// Keyboard
//
///////////////////////////////////////////////////////////////////////////////
typedef dtk_uint32 dtk_key;
#define DTK_KEY_BACKSPACE              0xff08
#define DTK_KEY_TAB                    0xff09
#define DTK_KEY_RETURN                 0xff0d
#define DTK_KEY_SHIFT                  0xff10
#define DTK_KEY_ESCAPE                 0xff1b
#define DTK_KEY_PAGE_UP                0xff55
#define DTK_KEY_PAGE_DOWN              0xff56
#define DTK_KEY_END                    0xff57
#define DTK_KEY_HOME                   0xff50
#define DTK_KEY_ARROW_LEFT             0x8fb
#define DTK_KEY_ARROW_UP               0x8fc
#define DTK_KEY_ARROW_RIGHT            0x8fd
#define DTK_KEY_ARROW_DOWN             0x8fe
#define DTK_KEY_DELETE                 0xffff
#define DTK_KEY_F1                     0xffbe
#define DTK_KEY_F2                     0xffbf
#define DTK_KEY_F3                     0xffc0
#define DTK_KEY_F4                     0xffc1
#define DTK_KEY_F5                     0xffc2
#define DTK_KEY_F6                     0xffc3
#define DTK_KEY_F7                     0xffc4
#define DTK_KEY_F8                     0xffc5
#define DTK_KEY_F9                     0xffc6
#define DTK_KEY_F10                    0xffc7
#define DTK_KEY_F11                    0xffc8
#define DTK_KEY_F12                    0xffc9
#define DTK_KEY_SPACE                  0x020

// Converts a key to a string, returning the length of the string.
size_t dtk_key_to_string(dtk_key key, char* strOut, size_t strOutSize);

// Converts a string to a key code.
dtk_key dtk_key_parse(const char* str);


///////////////////////////////////////////////////////////////////////////////
//
// Mouse
//
///////////////////////////////////////////////////////////////////////////////
typedef dtk_uint32 dtk_mouse_button;
#define DTK_MOUSE_BUTTON_LEFT   1
#define DTK_MOUSE_BUTTON_RIGHT  2
#define DTK_MOUSE_BUTTON_MIDDLE 3
#define DTK_MOUSE_BUTTON_4      4
#define DTK_MOUSE_BUTTON_5      5

// Retrieves the modifier flag for the given mouse button. The return value will be a value with one of
// the DTK_MODIFIER_MOUSE_BUTTON_* flags set.
dtk_uint32 dtk_get_mouse_button_modifier_flag(dtk_mouse_button button);