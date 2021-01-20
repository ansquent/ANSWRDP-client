/*
   rdesktop: A Remote Desktop Protocol client.
   User interface services - X keyboard mapping
   Copyright (C) Matthew Chapman 1999-2002
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define XK_MISCELLANY
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include "rdesktop.h"
#include "scancodes.h"
#include <exception.h>
#include <QMouseEvent>

#define KEYMAP_SIZE 0xffff+1
#define KEYMAP_MASK 0xffff
#define KEYMAP_MAX_LINE_LENGTH 80

extern char keymapname[16];
extern int keylayout;
extern BOOL enable_compose;

static BOOL keymap_loaded;
static key_translation keymap[KEYMAP_SIZE];
static int min_keycode;
static uint16 remote_modifier_state = 0;

static void update_modifier_state(uint8 scancode, BOOL pressed);

static void
add_to_keymap(char *keyname, uint8 scancode, uint16 modifiers, char *mapname)
{
	throw not_implemented_error();
}


static BOOL
xkeymap_read(char *mapname)
{
	throw not_implemented_error();
}


/* Before connecting and creating UI */
void
xkeymap_init(void)
{
	throw not_implemented_error();
}

/* Handles, for example, multi-scancode keypresses (which is not
   possible via keymap-files) */
BOOL
handle_special_keys(int key, uint32 ev_time, BOOL pressed)
{
    //throw not_implemented_error();
    return False;
}


key_translation
xkeymap_translate_key(uint32 keysym, unsigned int keycode, unsigned int state)
{
	throw not_implemented_error();
}

uint16
xkeymap_translate_button(Qt::MouseButton button)
{
    switch (button)
    {
        case Qt::MouseButton::LeftButton:	/* left */
            return MOUSE_FLAG_BUTTON1;
        case Qt::MouseButton::MidButton:	/* middle */
            return MOUSE_FLAG_BUTTON3;
        case Qt::MouseButton::RightButton:	/* right */
            return MOUSE_FLAG_BUTTON2;
    }

    return 0;
}

char *
get_ksname(uint32 keysym)
{
	throw not_implemented_error();
}


void
ensure_remote_modifiers(uint32 ev_time, key_translation tr)
{
	throw not_implemented_error();
}


void
reset_modifier_keys(unsigned int state)
{
	throw not_implemented_error();
}


static void
update_modifier_state(uint8 scancode, BOOL pressed)
{
#ifdef WITH_DEBUG_KBD
	uint16 old_modifier_state;

	old_modifier_state = remote_modifier_state;
#endif

	switch (scancode)
	{
		case SCANCODE_CHAR_LSHIFT:
			MASK_CHANGE_BIT(remote_modifier_state, MapLeftShiftMask, pressed);
			break;
		case SCANCODE_CHAR_RSHIFT:
			MASK_CHANGE_BIT(remote_modifier_state, MapRightShiftMask, pressed);
			break;
		case SCANCODE_CHAR_LCTRL:
			MASK_CHANGE_BIT(remote_modifier_state, MapLeftCtrlMask, pressed);
			break;
		case SCANCODE_CHAR_RCTRL:
			MASK_CHANGE_BIT(remote_modifier_state, MapRightCtrlMask, pressed);
			break;
		case SCANCODE_CHAR_LALT:
			MASK_CHANGE_BIT(remote_modifier_state, MapLeftAltMask, pressed);
			break;
		case SCANCODE_CHAR_RALT:
			MASK_CHANGE_BIT(remote_modifier_state, MapRightAltMask, pressed);
			break;
		case SCANCODE_CHAR_LWIN:
			MASK_CHANGE_BIT(remote_modifier_state, MapLeftWinMask, pressed);
			break;
		case SCANCODE_CHAR_RWIN:
			MASK_CHANGE_BIT(remote_modifier_state, MapRightWinMask, pressed);
			break;
		case SCANCODE_CHAR_NUMLOCK:
			/* KeyReleases for NumLocks are sent immediately. Toggle the
			   modifier state only on Keypress */
			if (pressed)
			{
				BOOL newNumLockState;
				newNumLockState =
					(MASK_HAS_BITS
					 (remote_modifier_state, MapNumLockMask) == False);
				MASK_CHANGE_BIT(remote_modifier_state,
						MapNumLockMask, newNumLockState);
			}
			break;
	}

#ifdef WITH_DEBUG_KBD
	if (old_modifier_state != remote_modifier_state)
	{
		DEBUG_KBD(("Before updating modifier_state:0x%x, pressed=0x%x\n",
			   old_modifier_state, pressed));
		DEBUG_KBD(("After updating modifier_state:0x%x\n", remote_modifier_state));
	}
#endif

}

/* Send keyboard input */
void
rdp_send_scancode(uint32 time, uint16 flags, uint8 scancode)
{
	update_modifier_state(scancode, !(flags & RDP_KEYRELEASE));

	if (scancode & SCANCODE_EXTENDED)
	{
		DEBUG_KBD(("Sending extended scancode=0x%x, flags=0x%x\n",
			   scancode & ~SCANCODE_EXTENDED, flags));
		rdp_send_input(time, RDP_INPUT_SCANCODE, flags | KBD_FLAG_EXT,
			       scancode & ~SCANCODE_EXTENDED, 0);
	}
	else
	{
		DEBUG_KBD(("Sending scancode=0x%x, flags=0x%x\n", scancode, flags));
		rdp_send_input(time, RDP_INPUT_SCANCODE, flags, scancode, 0);
	}
}
