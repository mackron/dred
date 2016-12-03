// Copyright (C) 2016 David Reid. See included LICENSE file.

// The flags below are posted on on key down/up and mouse button events.
#define DTK_MOUSE_BUTTON_LEFT_DOWN    (1 << 0)
#define DTK_MOUSE_BUTTON_RIGHT_DOWN   (1 << 1)
#define DTK_MOUSE_BUTTON_MIDDLE_DOWN  (1 << 2)
#define DTK_MOUSE_BUTTON_4_DOWN       (1 << 3)
#define DTK_MOUSE_BUTTON_5_DOWN       (1 << 4)
#define DTK_KEY_SHIFT_DOWN            (1 << 5)        // Whether or not a shift key is down at the time the input event is handled.
#define DTK_KEY_CTRL_DOWN             (1 << 6)        // Whether or not a ctrl key is down at the time the input event is handled.
#define DTK_KEY_ALT_DOWN              (1 << 7)        // Whether or not an alt key is down at the time the input event is handled.
#define DTK_KEY_AUTO_REPEATED         (1 << 31)       // Whether or not the key press is generated due to auto-repeating. Only used with key down events.


///////////////////////////////////////////////////////////////////////////////
//
// Keyboard
//
///////////////////////////////////////////////////////////////////////////////
typedef dtk_uint32 dtk_key;
#define DTK_KEY_BACKSPACE              0xff08
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
#define DTK_MOUSE_BUTTON_LEFT   0
#define DTK_MOUSE_BUTTON_RIGHT  1
#define DTK_MOUSE_BUTTON_MIDDLE 2
#define DTK_MOUSE_BUTTON_4      3
#define DTK_MOUSE_BUTTON_5      4