#ifndef CONSTANT_H
#define CONSTANT_H
#include <exception>
/*
   rdesktop: A Remote Desktop Protocol client.
   Miscellaneous protocol constants
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

/* TCP port for Remote Desktop Protocol */
#define VERSION "1.2.0"

#define NUM_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#define STRNCPY(dst, src, n) { strncpy(dst,src,n-1); dst[n-1] = 0; }

typedef int BOOL;

#include <QImage>

typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;

typedef QImage *HRDPBITMAP;
typedef QImage *HGLYPH;
typedef void *HCOLOURMAP;
typedef void *HRDPCURSOR;

typedef struct _COLOURENTRY {
    uint8 red;
    uint8 green;
    uint8 blue;

}
        COLOURENTRY;

typedef struct _COLOURMAP {
    uint16 ncolours;
    COLOURENTRY *colours;

}
        COLOURMAP;

typedef struct _BOUNDS {
    uint16 left;
    uint16 top;
    uint16 right;
    uint16 bottom;

}
        BOUNDS;

typedef struct _PEN {
    uint8 style;
    uint8 width;
    uint8 colour;

}
        PEN;

typedef struct _BRUSH {
    uint8 xorigin;
    uint8 yorigin;
    uint8 style;
    uint8 pattern[8];

}
        BRUSH;

typedef struct _FONTGLYPH {
    sint16 offset;
    sint16 baseline;
    uint16 width;
    uint16 height;
    QImage *pixmap;

}
        FONTGLYPH;

typedef struct _DATABLOB {
    uint8 *data;
    int size;

}
        DATABLOB;

typedef struct _key_translation {
    uint8 scancode;
    uint16 modifiers;
}
        key_translation;

#define TCP_PORT_RDP 3389

/* ISO PDU codes */
enum ISO_PDU_CODE {
    ISO_PDU_CR = 0xE0,    /* Connection Request */
    ISO_PDU_CC = 0xD0,    /* Connection Confirm */
    ISO_PDU_DR = 0x80,    /* Disconnect Request */
    ISO_PDU_DT = 0xF0,    /* Data */
    ISO_PDU_ER = 0x70    /* Error */
};

/* MCS PDU codes */
enum MCS_PDU_TYPE {
    MCS_EDRQ = 1,        /* Erect Domain Request */
    MCS_DPUM = 8,        /* Disconnect Provider Ultimatum */
    MCS_AURQ = 10,        /* Attach User Request */
    MCS_AUCF = 11,        /* Attach User Confirm */
    MCS_CJRQ = 14,        /* Channel Join Request */
    MCS_CJCF = 15,        /* Channel Join Confirm */
    MCS_SDRQ = 25,        /* Send Data Request */
    MCS_SDIN = 26        /* Send Data Indication */
};

#define MCS_CONNECT_INITIAL    0x7f65
#define MCS_CONNECT_RESPONSE    0x7f66

#define BER_TAG_BOOLEAN        1
#define BER_TAG_INTEGER        2
#define BER_TAG_OCTET_STRING    4
#define BER_TAG_RESULT        10
#define MCS_TAG_DOMAIN_PARAMS    0x30

#define MCS_GLOBAL_CHANNEL    1003

/* RDP secure transport constants */
#define SEC_RANDOM_SIZE        32
#define SEC_MODULUS_SIZE    64
#define SEC_PADDING_SIZE    8
#define SEC_EXPONENT_SIZE    4

#define SEC_CLIENT_RANDOM    0x0001
#define SEC_ENCRYPT        0x0008
#define SEC_LOGON_INFO        0x0040
#define SEC_LICENCE_NEG        0x0080

#define SEC_TAG_SRV_INFO    0x0c01
#define SEC_TAG_SRV_CRYPT    0x0c02
#define SEC_TAG_SRV_3        0x0c03

#define SEC_TAG_CLI_INFO    0xc001
#define SEC_TAG_CLI_CRYPT    0xc002

#define SEC_TAG_PUBKEY        0x0006
#define SEC_TAG_KEYSIG        0x0008

#define SEC_RSA_MAGIC        0x31415352    /* RSA1 */

/* RDP licensing constants */
#define LICENCE_TOKEN_SIZE    10
#define LICENCE_HWID_SIZE    20
#define LICENCE_SIGNATURE_SIZE    16

#define LICENCE_TAG_DEMAND    0x0201
#define LICENCE_TAG_AUTHREQ    0x0202
#define LICENCE_TAG_ISSUE    0x0203
#define LICENCE_TAG_REISSUE    0x0204
#define LICENCE_TAG_PRESENT    0x0212
#define LICENCE_TAG_REQUEST    0x0213
#define LICENCE_TAG_AUTHRESP    0x0215
#define LICENCE_TAG_RESULT    0x02ff

#define LICENCE_TAG_USER    0x000f
#define LICENCE_TAG_HOST    0x0010

/* RDP PDU codes */
enum RDP_PDU_TYPE {
    RDP_PDU_DEMAND_ACTIVE = 1,
    RDP_PDU_CONFIRM_ACTIVE = 3,
    RDP_PDU_DEACTIVATE = 6,
    RDP_PDU_DATA = 7
};

enum RDP_DATA_PDU_TYPE {
    RDP_DATA_PDU_UPDATE = 2,
    RDP_DATA_PDU_CONTROL = 20,
    RDP_DATA_PDU_POINTER = 27,
    RDP_DATA_PDU_INPUT = 28,
    RDP_DATA_PDU_SYNCHRONISE = 31,
    RDP_DATA_PDU_BELL = 34,
    RDP_DATA_PDU_LOGON = 38,
    RDP_DATA_PDU_FONT2 = 39
};

enum RDP_CONTROL_PDU_TYPE {
    RDP_CTL_REQUEST_CONTROL = 1,
    RDP_CTL_GRANT_CONTROL = 2,
    RDP_CTL_DETACH = 3,
    RDP_CTL_COOPERATE = 4
};

enum RDP_UPDATE_PDU_TYPE {
    RDP_UPDATE_ORDERS = 0,
    RDP_UPDATE_BITMAP = 1,
    RDP_UPDATE_PALETTE = 2,
    RDP_UPDATE_SYNCHRONIZE = 3
};

enum RDP_POINTER_PDU_TYPE {
    RDP_POINTER_MOVE = 3,
    RDP_POINTER_COLOR = 6,
    RDP_POINTER_CACHED = 7
};

enum RDP_INPUT_DEVICE {
    RDP_INPUT_SYNCHRONIZE = 0,
    RDP_INPUT_CODEPOINT = 1,
    RDP_INPUT_VIRTKEY = 2,
    RDP_INPUT_SCANCODE = 4,
    RDP_INPUT_MOUSE = 0x8001
};

/* Device flags */
#define KBD_FLAG_RIGHT          0x0001
#define KBD_FLAG_EXT            0x0100
#define KBD_FLAG_QUIET          0x1000
#define KBD_FLAG_DOWN           0x4000
#define KBD_FLAG_UP             0x8000

/* These are for synchronization; not for keystrokes */
#define KBD_FLAG_SCROLL   0x0001
#define KBD_FLAG_NUMLOCK  0x0002
#define KBD_FLAG_CAPITAL  0x0004

/* See T.128 */
#define RDP_KEYPRESS 0
#define RDP_KEYRELEASE (KBD_FLAG_DOWN | KBD_FLAG_UP)

#define MOUSE_FLAG_MOVE         0x0800
#define MOUSE_FLAG_BUTTON1      0x1000
#define MOUSE_FLAG_BUTTON2      0x2000
#define MOUSE_FLAG_BUTTON3      0x4000
#define MOUSE_FLAG_BUTTON4      0x0280
#define MOUSE_FLAG_BUTTON5      0x0380
#define MOUSE_FLAG_DOWN         0x8000

/* Raster operation masks */
#define ROP2_S(rop3) (rop3 & 0xf)
#define ROP2_P(rop3) ((rop3 & 0x3) | ((rop3 & 0x30) >> 2))

#define ROP2_COPY    0xc
#define ROP2_XOR    0x6
#define ROP2_AND    0x8
#define ROP2_NXOR    0x9
#define ROP2_OR        0xe

#define MIX_TRANSPARENT    0
#define MIX_OPAQUE    1

#define TEXT2_VERTICAL        0x04
#define TEXT2_IMPLICIT_X    0x20

/* RDP capabilities */
#define RDP_CAPSET_GENERAL    1
#define RDP_CAPLEN_GENERAL    0x18
#define OS_MAJOR_TYPE_UNIX    4
#define OS_MINOR_TYPE_XSERVER    7

#define RDP_CAPSET_BITMAP    2
#define RDP_CAPLEN_BITMAP    0x1C

#define RDP_CAPSET_ORDER    3
#define RDP_CAPLEN_ORDER    0x58
#define ORDER_CAP_NEGOTIATE    2
#define ORDER_CAP_NOSUPPORT    4

#define RDP_CAPSET_BMPCACHE    4
#define RDP_CAPLEN_BMPCACHE    0x28

#define RDP_CAPSET_CONTROL    5
#define RDP_CAPLEN_CONTROL    0x0C

#define RDP_CAPSET_ACTIVATE    7
#define RDP_CAPLEN_ACTIVATE    0x0C

#define RDP_CAPSET_POINTER    8
#define RDP_CAPLEN_POINTER    0x08

#define RDP_CAPSET_SHARE    9
#define RDP_CAPLEN_SHARE    0x08

#define RDP_CAPSET_COLCACHE    10
#define RDP_CAPLEN_COLCACHE    0x08

#define RDP_CAPSET_UNKNOWN    13
#define RDP_CAPLEN_UNKNOWN    0x9C

#define RDP_SOURCE        "MSTSC"

/* Logon flags */
#define RDP_LOGON_NORMAL    0x33
#define RDP_LOGON_AUTO        0x8

/* Keymap flags */
#define MapRightShiftMask   (1<<0)
#define MapLeftShiftMask    (1<<1)
#define MapShiftMask (MapRightShiftMask | MapLeftShiftMask)

#define MapRightAltMask     (1<<2)
#define MapLeftAltMask      (1<<3)
#define MapAltGrMask MapRightAltMask

#define MapRightCtrlMask    (1<<4)
#define MapLeftCtrlMask     (1<<5)
#define MapCtrlMask (MapRightCtrlMask | MapLeftCtrlMask)

#define MapRightWinMask     (1<<6)
#define MapLeftWinMask      (1<<7)
#define MapWinMask (MapRightWinMask | MapLeftWinMask)

#define MapNumLockMask      (1<<8)
#define MapCapsLockMask     (1<<9)

#define MapLocalStateMask   (1<<10)

#define MapInhibitMask      (1<<11)

#define MASK_ADD_BITS(var, mask) (var |= mask)
#define MASK_REMOVE_BITS(var, mask) (var &= ~mask)
#define MASK_HAS_BITS(var, mask) ((var & mask)>0)
#define MASK_CHANGE_BIT(var, mask, active) (var = ((var & ~mask) | (active ? mask : 0)))

/* scancodes.h */

#define SCANCODE_EXTENDED 0x80

#define SCANCODE_KEY_1 0x29
#define SCANCODE_CHAR_GRAVE 0x29

#define SCANCODE_KEY_2 0x2
#define SCANCODE_CHAR_1 SCANCODE_KEY_2

#define SCANCODE_KEY_3 0x3
#define SCANCODE_CHAR_2 SCANCODE_KEY_3

#define SCANCODE_KEY_4 0x4
#define SCANCODE_CHAR_3 SCANCODE_KEY_4

#define SCANCODE_KEY_5 0x5
#define SCANCODE_CHAR_4 SCANCODE_KEY_5

#define SCANCODE_KEY_6 0x6
#define SCANCODE_CHAR_5 SCANCODE_KEY_6

#define SCANCODE_KEY_7 0x7
#define SCANCODE_CHAR_6 SCANCODE_KEY_7

#define SCANCODE_KEY_8 0x8
#define SCANCODE_CHAR_7 SCANCODE_KEY_8

#define SCANCODE_KEY_9 0x9
#define SCANCODE_CHAR_8 SCANCODE_KEY_9

#define SCANCODE_KEY_10 0xa
#define SCANCODE_CHAR_9 SCANCODE_KEY_10

#define SCANCODE_KEY_11 0xb
#define SCANCODE_CHAR_0 SCANCODE_KEY_11

#define SCANCODE_KEY_12 0xc
#define SCANCODE_CHAR_MINUS SCANCODE_KEY_12

#define SCANCODE_KEY_13 0xd
#define SCANCODE_CHAR_EQUAL SCANCODE_KEY_13

/* Key 14 does not exist */

#define SCANCODE_KEY_15 0xe
#define SCANCODE_CHAR_BACKSPACE SCANCODE_KEY_15

#define SCANCODE_KEY_16 0xf
#define SCANCODE_CHAR_TAB SCANCODE_KEY_16

#define SCANCODE_KEY_17 0x10
#define SCANCODE_CHAR_Q SCANCODE_KEY_17

#define SCANCODE_KEY_18 0x11
#define SCANCODE_CHAR_W SCANCODE_KEY_18

#define SCANCODE_KEY_19 0x12
#define SCANCODE_CHAR_E SCANCODE_KEY_19

#define SCANCODE_KEY_20 0x13
#define SCANCODE_CHAR_R SCANCODE_KEY_20

#define SCANCODE_KEY_21 0x14
#define SCANCODE_CHAR_T SCANCODE_KEY_21

#define SCANCODE_KEY_22 0x15
#define SCANCODE_CHAR_Y SCANCODE_KEY_22

#define SCANCODE_KEY_23 0x16
#define SCANCODE_CHAR_U SCANCODE_KEY_23

#define SCANCODE_KEY_24 0x17
#define SCANCODE_CHAR_I SCANCODE_KEY_24

#define SCANCODE_KEY_25 0x18
#define SCANCODE_CHAR_O SCANCODE_KEY_25

#define SCANCODE_KEY_26 0x19
#define SCANCODE_CHAR_P SCANCODE_KEY_26

#define SCANCODE_KEY_27 0x1a
#define SCANCODE_CHAR_BRACKETLEFT SCANCODE_KEY_27

#define SCANCODE_KEY_28 0x1b
#define SCANCODE_CHAR_BRACKETRIGHT SCANCODE_KEY_28

/* Only on US keyboard */
#define SCANCODE_KEY_29 0x2b
#define SCANCODE_CHAR_BACKSLASH SCANCODE_KEY_29

#define SCANCODE_KEY_30 0x3a
#define SCANCODE_CHAR_CAPSLOCK SCANCODE_KEY_30

#define SCANCODE_KEY_31 0x1e
#define SCANCODE_CHAR_A SCANCODE_KEY_31

#define SCANCODE_KEY_32 0x1f
#define SCANCODE_CHAR_S SCANCODE_KEY_32

#define SCANCODE_KEY_33 0x20
#define SCANCODE_CHAR_D SCANCODE_KEY_33

#define SCANCODE_KEY_34 0x21
#define SCANCODE_CHAR_F SCANCODE_KEY_34

#define SCANCODE_KEY_35 0x22
#define SCANCODE_CHAR_G SCANCODE_KEY_35

#define SCANCODE_KEY_36 0x23
#define SCANCODE_CHAR_H SCANCODE_KEY_36

#define SCANCODE_KEY_37 0x24
#define SCANCODE_CHAR_J SCANCODE_KEY_37

#define SCANCODE_KEY_38 0x25
#define SCANCODE_CHAR_K SCANCODE_KEY_38

#define SCANCODE_KEY_39 0x26
#define SCANCODE_CHAR_L SCANCODE_KEY_39

#define SCANCODE_KEY_40 0x27
#define SCANCODE_CHAR_SEMICOLON SCANCODE_KEY_40

#define SCANCODE_KEY_41 0x28
#define SCANCODE_CHAR_APOSTROPHE SCANCODE_KEY_41

/* Only on international keyboard */
#define SCANCODE_KEY_42 0x2b

#define SCANCODE_KEY_43 0x1c
#define SCANCODE_CHAR_ENTER SCANCODE_KEY_43

#define SCANCODE_KEY_44 0x2a
#define SCANCODE_CHAR_LSHIFT SCANCODE_KEY_44

/* Only on international keyboard */
#define SCANCODE_KEY_45 0x56

#define SCANCODE_KEY_46 0x2c
#define SCANCODE_CHAR_Z SCANCODE_KEY_46

#define SCANCODE_KEY_47 0x2d
#define SCANCODE_CHAR_X SCANCODE_KEY_47

#define SCANCODE_KEY_48 0x2e
#define SCANCODE_CHAR_C SCANCODE_KEY_48

#define SCANCODE_KEY_49 0x2f
#define SCANCODE_CHAR_V SCANCODE_KEY_49

#define SCANCODE_KEY_50 0x30
#define SCANCODE_CHAR_B SCANCODE_KEY_50

#define SCANCODE_KEY_51 0x31
#define SCANCODE_CHAR_N SCANCODE_KEY_51

#define SCANCODE_KEY_52 0x32
#define SCANCODE_CHAR_M SCANCODE_KEY_52

#define SCANCODE_KEY_53 0x33
#define SCANCODE_CHAR_COMMA SCANCODE_KEY_53

#define SCANCODE_KEY_54 0x34
#define SCANCODE_CHAR_DOT SCANCODE_KEY_54

#define SCANCODE_KEY_55 0x35
#define SCANCODE_CHAR_SLASH SCANCODE_KEY_55

/* Only on Brazilian and some Far East keyboards */
#define SCANCODE_KEY_56 0x73

#define SCANCODE_KEY_57 0x36
#define SCANCODE_CHAR_RSHIFT SCANCODE_KEY_57

#define SCANCODE_KEY_58 0x1d
#define SCANCODE_CHAR_LCTRL SCANCODE_KEY_58

/* Key 59 does not exist */

#define SCANCODE_KEY_60 0x38
#define SCANCODE_CHAR_LALT SCANCODE_KEY_60

#define SCANCODE_KEY_61 0x39
#define SCANCODE_CHAR_SPACE SCANCODE_KEY_61

#define SCANCODE_KEY_62 (SCANCODE_EXTENDED | 0x38)
#define SCANCODE_CHAR_RALT SCANCODE_KEY_62

/* Key 63 does not exist */

#define SCANCODE_KEY_64 (SCANCODE_EXTENDED | 0x1d)
#define SCANCODE_CHAR_RCTRL SCANCODE_KEY_64

/* Key 65 - 74 does not exist */

#define SCANCODE_KEY_75 (SCANCODE_EXTENDED | 0x52)
#define SCANCODE_CHAR_INSERT SCANCODE_KEY_75

#define SCANCODE_KEY_76 (SCANCODE_EXTENDED | 0x53)
#define SCANCODE_CHAR_DELETE SCANCODE_KEY_76

/* Key 77 - 78 does not exist */

#define SCANCODE_KEY_79 (SCANCODE_EXTENDED | 0x4b)
#define SCANCODE_CHAR_LARROW SCANCODE_KEY_79

#define SCANCODE_KEY_80 (SCANCODE_EXTENDED | 0x47)
#define SCANCODE_CHAR_HOME SCANCODE_KEY_80

#define SCANCODE_KEY_81 (SCANCODE_EXTENDED | 0x4f)
#define SCANCODE_CHAR_END SCANCODE_KEY_81

/* Key 82 does not exist */

#define SCANCODE_KEY_83 (SCANCODE_EXTENDED | 0x48)
#define SCANCODE_CHAR_UPARROW SCANCODE_KEY_82

#define SCANCODE_KEY_84 (SCANCODE_EXTENDED | 0x50)
#define SCANCODE_CHAR_DNARROW SCANCODE_KEY_84

#define SCANCODE_KEY_85 (SCANCODE_EXTENDED | 0x49)
#define SCANCODE_CHAR_PAGEUP SCANCODE_KEY_85

#define SCANCODE_KEY_86 (SCANCODE_EXTENDED | 0x51)
#define SCANCODE_CHAR_PAGEDOWN SCANCODE_KEY_86

/* Key 87 - 88 does not exist */

#define SCANCODE_KEY_89 (SCANCODE_EXTENDED | 0x4d)
#define SCANCODE_CHAR_RARROW SCANCODE_KEY_89

#define SCANCODE_KEY_90 0x45
#define SCANCODE_CHAR_NUMLOCK SCANCODE_KEY_90

#define SCANCODE_KEY_91 0x47
#define SCANCODE_CHAR_NUMERIC7 SCANCODE_KEY_91

#define SCANCODE_KEY_92 0x4b
#define SCANCODE_CHAR_NUMERIC4 SCANCODE_KEY_92

#define SCANCODE_KEY_93 0x4f
#define SCANCODE_CHAR_NUMERIC1 SCANCODE_KEY_93

/* Key 94 does not exist */

#define SCANCODE_KEY_95 (SCANCODE_EXTENDED | 0x35)
#define SCANCODE_CHAR_NUMERICSLASH SCANCODE_KEY_95

#define SCANCODE_KEY_96 0x48
#define SCANCODE_CHAR_NUMERIC8 SCANCODE_KEY_96

#define SCANCODE_KEY_97 0x4c
#define SCANCODE_CHAR_NUMERIC5 SCANCODE_KEY_97

#define SCANCODE_KEY_98 0x50
#define SCANCODE_CHAR_NUMERIC2 SCANCODE_KEY_98

#define SCANCODE_KEY_99 0x52
#define SCANCODE_CHAR_NUMERIC0 SCANCODE_KEY_99

#define SCANCODE_KEY_100 0x37
#define SCANCODE_CHAR_NUMERICSTAR SCANCODE_KEY_100

#define SCANCODE_KEY_101 0x49
#define SCANCODE_CHAR_NUMERIC9 SCANCODE_KEY_101

#define SCANCODE_KEY_102 0x4d
#define SCANCODE_CHAR_NUMERIC6 SCANCODE_KEY_102

#define SCANCODE_KEY_103 0x51
#define SCANCODE_CHAR_NUMERIC3 SCANCODE_KEY_103

#define SCANCODE_KEY_104 0x53
#define SCANCODE_CHAR_NUMERICDOT SCANCODE_KEY_104

#define SCANCODE_KEY_105 0x4a
#define SCANCODE_CHAR_NUMERICMINUS SCANCODE_KEY_105

#define SCANCODE_KEY_106 0x4e
#define SCANCODE_CHAR_NUMERICPLUS SCANCODE_KEY_106

/* Only on Brazilian and some Far East keyboards */
#define SCANCODE_KEY_107 0x

#define SCANCODE_KEY_108 (SCANCODE_EXTENDED | 0x1c)
#define SCANCODE_CHAR_NUMERICENTER SCANCODE_KEY_108

/* Key 109 does not exist */

#define SCANCODE_KEY_110 0x1
#define SCANCODE_CHAR_ESC SCANCODE_KEY_110

/* Key 111 does not exist */

#define SCANCODE_KEY_112 0x3b
#define SCANCODE_CHAR_F1 SCANCODE_KEY_112

#define SCANCODE_KEY_113 0x3c
#define SCANCODE_CHAR_F2 SCANCODE_KEY_113

#define SCANCODE_KEY_114 0x3d
#define SCANCODE_CHAR_F3 SCANCODE_KEY_114

#define SCANCODE_KEY_115 0x3e
#define SCANCODE_CHAR_F4 SCANCODE_KEY_115

#define SCANCODE_KEY_116 0x3f
#define SCANCODE_CHAR_F5 SCANCODE_KEY_116

#define SCANCODE_KEY_117 0x40
#define SCANCODE_CHAR_F6 SCANCODE_KEY_117

#define SCANCODE_KEY_118 0x41
#define SCANCODE_CHAR_F7 SCANCODE_KEY_118

#define SCANCODE_KEY_119 0x42
#define SCANCODE_CHAR_F8 SCANCODE_KEY_119

#define SCANCODE_KEY_120 0x43
#define SCANCODE_CHAR_F9 SCANCODE_KEY_120

#define SCANCODE_KEY_121 0x44
#define SCANCODE_CHAR_F10 SCANCODE_KEY_121

#define SCANCODE_KEY_122 0x57
#define SCANCODE_CHAR_F11 SCANCODE_KEY_122

#define SCANCODE_KEY_123 0x58
#define SCANCODE_CHAR_F12 SCANCODE_KEY_123

/* Key 124: The Print Screen sequence is complicated, and
   hardcoded in xkeymap.c */

#define SCANCODE_KEY_125 0x46
#define SCANCODE_CHAR_SCROLLLOCK SCANCODE_KEY_125

/* Key 126: The Pause and Break sequences is complicated, and
   hardcoded in xkeymap.c */

/* 
   The keys below does not have any key location number
*/

#define SCANCODE_CHAR_LWIN (SCANCODE_EXTENDED | 0x5b)

#define SCANCODE_CHAR_RWIN (SCANCODE_EXTENDED | 0x5c)

#define SCANCODE_CHAR_APPLICATION (SCANCODE_EXTENDED | 0x5d)

#define SCANCODE_CHAR_ACPIPOWER (SCANCODE_EXTENDED | 0x5e)

#define SCANCODE_CHAR_ACPISLEEP (SCANCODE_EXTENDED | 0x5f)

#define SCANCODE_CHAR_ACPIWAKE (SCANCODE_EXTENDED | 0x63)


/* bitmap.c */

#define CVAL(p)   (*(p++))

#define UNROLL8(exp) { exp exp exp exp exp exp exp exp }

#define REPEAT(statement) \
{ \
    while((count & ~0x7) && ((x+8) < width)) \
        UNROLL8( statement; count--; x++; ); \
    \
    while((count > 0) && (x < width)) { statement; count--; x++; } \
}

#define MASK_UPDATE() \
{ \
    mixmask <<= 1; \
    if (mixmask == 0) \
    { \
        mask = fom_mask ? fom_mask : CVAL(input); \
        mixmask = 1; \
    } \
}

/* exception.h */
struct not_implemented_error: public std::exception{
    /* Nothing */
} ;

/* orders.h */
#define RDP_ORDER_STANDARD   0x01
#define RDP_ORDER_SECONDARY  0x02
#define RDP_ORDER_BOUNDS     0x04
#define RDP_ORDER_CHANGE     0x08
#define RDP_ORDER_DELTA      0x10
#define RDP_ORDER_LASTBOUNDS 0x20
#define RDP_ORDER_SMALL      0x40
#define RDP_ORDER_TINY       0x80

enum RDP_ORDER_TYPE {
    RDP_ORDER_DESTBLT = 0,
    RDP_ORDER_PATBLT = 1,
    RDP_ORDER_SCREENBLT = 2,
    RDP_ORDER_LINE = 9,
    RDP_ORDER_RECT = 10,
    RDP_ORDER_DESKSAVE = 11,
    RDP_ORDER_MEMBLT = 13,
    RDP_ORDER_TRIBLT = 14,
    RDP_ORDER_POLYLINE = 22,
    RDP_ORDER_TEXT2 = 27
};

enum RDP_SECONDARY_ORDER_TYPE {
    RDP_ORDER_RAW_BMPCACHE = 0,
    RDP_ORDER_COLCACHE = 1,
    RDP_ORDER_BMPCACHE = 2,
    RDP_ORDER_FONTCACHE = 3
};

typedef struct _DESTBLT_ORDER {
    uint16 x;
    uint16 y;
    uint16 cx;
    uint16 cy;
    uint8 opcode;

}
        DESTBLT_ORDER;

typedef struct _PATBLT_ORDER {
    uint16 x;
    uint16 y;
    uint16 cx;
    uint16 cy;
    uint8 opcode;
    uint8 bgcolour;
    uint8 fgcolour;
    BRUSH brush;

}
        PATBLT_ORDER;

typedef struct _SCREENBLT_ORDER {
    uint16 x;
    uint16 y;
    uint16 cx;
    uint16 cy;
    uint8 opcode;
    uint16 srcx;
    uint16 srcy;

}
        SCREENBLT_ORDER;

typedef struct _LINE_ORDER {
    uint16 mixmode;
    uint16 startx;
    uint16 starty;
    uint16 endx;
    uint16 endy;
    uint8 bgcolour;
    uint8 opcode;
    PEN pen;

}
        LINE_ORDER;

typedef struct _RECT_ORDER {
    uint16 x;
    uint16 y;
    uint16 cx;
    uint16 cy;
    uint8 colour;

}
        RECT_ORDER;

typedef struct _DESKSAVE_ORDER {
    uint32 offset;
    uint16 left;
    uint16 top;
    uint16 right;
    uint16 bottom;
    uint8 action;

}
        DESKSAVE_ORDER;

typedef struct _TRIBLT_ORDER {
    uint8 colour_table;
    uint8 cache_id;
    uint16 x;
    uint16 y;
    uint16 cx;
    uint16 cy;
    uint8 opcode;
    uint16 srcx;
    uint16 srcy;
    uint8 bgcolour;
    uint8 fgcolour;
    BRUSH brush;
    uint16 cache_idx;
    uint16 unknown;

}
        TRIBLT_ORDER;

typedef struct _MEMBLT_ORDER {
    uint8 colour_table;
    uint8 cache_id;
    uint16 x;
    uint16 y;
    uint16 cx;
    uint16 cy;
    uint8 opcode;
    uint16 srcx;
    uint16 srcy;
    uint16 cache_idx;

}
        MEMBLT_ORDER;

#define MAX_DATA 256

typedef struct _POLYLINE_ORDER {
    uint16 x;
    uint16 y;
    uint8 opcode;
    uint8 fgcolour;
    uint8 lines;
    uint8 datasize;
    uint8 data[MAX_DATA];

}
        POLYLINE_ORDER;

#define MAX_TEXT 256

typedef struct _TEXT2_ORDER {
    uint8 font;
    uint8 flags;
    uint8 mixmode;
    uint8 unknown;
    uint8 fgcolour;
    uint8 bgcolour;
    uint16 clipleft;
    uint16 cliptop;
    uint16 clipright;
    uint16 clipbottom;
    uint16 boxleft;
    uint16 boxtop;
    uint16 boxright;
    uint16 boxbottom;
    uint16 x;
    uint16 y;
    uint8 length;
    uint8 text[MAX_TEXT];

}
        TEXT2_ORDER;

typedef struct _RDP_ORDER_STATE {
    uint8 order_type;
    BOUNDS bounds;

    DESTBLT_ORDER destblt;
    PATBLT_ORDER patblt;
    SCREENBLT_ORDER screenblt;
    LINE_ORDER line;
    RECT_ORDER rect;
    DESKSAVE_ORDER desksave;
    MEMBLT_ORDER memblt;
    TRIBLT_ORDER triblt;
    POLYLINE_ORDER polyline;
    TEXT2_ORDER text2;

}
        RDP_ORDER_STATE;

typedef struct _RDP_RAW_BMPCACHE_ORDER {
    uint8 cache_id;
    uint8 pad1;
    uint8 width;
    uint8 height;
    uint8 bpp;
    uint16 bufsize;
    uint16 cache_idx;
    uint8 *data;

}
        RDP_RAW_BMPCACHE_ORDER;

typedef struct _RDP_BMPCACHE_ORDER {
    uint8 cache_id;
    uint8 pad1;
    uint8 width;
    uint8 height;
    uint8 bpp;
    uint16 bufsize;
    uint16 cache_idx;
    uint16 pad2;
    uint16 size;
    uint16 row_size;
    uint16 final_size;
    uint8 *data;

}
        RDP_BMPCACHE_ORDER;

#define MAX_GLYPH 32

typedef struct _RDP_FONT_GLYPH {
    uint16 character;
    uint16 unknown;
    uint16 baseline;
    uint16 width;
    uint16 height;
    uint8 data[MAX_GLYPH];

}
        RDP_FONT_GLYPH;

#define MAX_GLYPHS 256

typedef struct _RDP_FONTCACHE_ORDER {
    uint8 font;
    uint8 nglyphs;
    RDP_FONT_GLYPH glyphs[MAX_GLYPHS];

}
        RDP_FONTCACHE_ORDER;

typedef struct _RDP_COLCACHE_ORDER {
    uint8 cache_id;
    COLOURMAP map;

}
        RDP_COLCACHE_ORDER;

#define XK_MISCELLANY
#define KEYMAP_SIZE 0xffff+1
#define KEYMAP_MASK 0xffff
#define KEYMAP_MAX_LINE_LENGTH 80

/* Parser state */
typedef struct stream
{
    unsigned char *p;
    unsigned char *end;
    unsigned char *data;
    unsigned int size;

    /* Offsets of various headers */
    unsigned char *iso_hdr;
    unsigned char *mcs_hdr;
    unsigned char *sec_hdr;
    unsigned char *rdp_hdr;

}
        *STREAM;

#define s_push_layer(s,h,n)	{ (s)->h = (s)->p; (s)->p += n; }
#define s_pop_layer(s,h)	(s)->p = (s)->h;
#define s_mark_end(s)		(s)->end = (s)->p;
#define s_check(s)		((s)->p <= (s)->end)
#define s_check_rem(s,n)	((s)->p + n <= (s)->end)
#define s_check_end(s)		((s)->p == (s)->end)

#if defined(L_ENDIAN) && !defined(NEED_ALIGN)
#define in_uint16_le(s,v)	{ v = *(uint16 *)((s)->p); (s)->p += 2; }
#define in_uint32_le(s,v)	{ v = *(uint32 *)((s)->p); (s)->p += 4; }
#define out_uint16_le(s,v)	{ *(uint16 *)((s)->p) = v; (s)->p += 2; }
#define out_uint32_le(s,v)	{ *(uint32 *)((s)->p) = v; (s)->p += 4; }

#else
#define in_uint16_le(s,v)	{ v = *((s)->p++); v += *((s)->p++) << 8; }
#define in_uint32_le(s,v)	{ in_uint16_le(s,v) \
				v += *((s)->p++) << 16; v += *((s)->p++) << 24; }
#define out_uint16_le(s,v)	{ *((s)->p++) = (v) & 0xff; *((s)->p++) = ((v) >> 8) & 0xff; }
#define out_uint32_le(s,v)	{ out_uint16_le(s, (v) & 0xffff); out_uint16_le(s, ((v) >> 16) & 0xffff); }
#endif

#if defined(B_ENDIAN) && !defined(NEED_ALIGN)
#define in_uint16_be(s,v)	{ v = *(uint16 *)((s)->p); (s)->p += 2; }
#define in_uint32_be(s,v)	{ v = *(uint32 *)((s)->p); (s)->p += 4; }
#define out_uint16_be(s,v)	{ *(uint16 *)((s)->p) = v; (s)->p += 2; }
#define out_uint32_be(s,v)	{ *(uint32 *)((s)->p) = v; (s)->p += 4; }

#define B_ENDIAN_PREFERRED
#define in_uint16(s,v)		in_uint16_be(s,v)
#define in_uint32(s,v)		in_uint32_be(s,v)
#define out_uint16(s,v)		out_uint16_be(s,v)
#define out_uint32(s,v)		out_uint32_be(s,v)

#else
#define next_be(s,v)		v = ((v) << 8) + *((s)->p++);
#define in_uint16_be(s,v)	{ v = *((s)->p++); next_be(s,v); }
#define in_uint32_be(s,v)	{ in_uint16_be(s,v); next_be(s,v); next_be(s,v); }
#define out_uint16_be(s,v)	{ *((s)->p++) = ((v) >> 8) & 0xff; *((s)->p++) = (v) & 0xff; }
#define out_uint32_be(s,v)	{ out_uint16_be(s, ((v) >> 16) & 0xffff); out_uint16_be(s, (v) & 0xffff); }
#endif

#ifndef B_ENDIAN_PREFERRED
#define in_uint16(s,v)		in_uint16_le(s,v)
#define in_uint32(s,v)		in_uint32_le(s,v)
#define out_uint16(s,v)		out_uint16_le(s,v)
#define out_uint32(s,v)		out_uint32_le(s,v)
#endif

#define in_uint8(s,v)		v = *((s)->p++);
#define in_uint8p(s,v,n)	{ v = (s)->p; (s)->p += n; }
#define in_uint8a(s,v,n)	{ memcpy(v,(s)->p,n); (s)->p += n; }
#define in_uint8s(s,n)		(s)->p += n;
#define out_uint8(s,v)		*((s)->p++) = v;
#define out_uint8p(s,v,n)	{ memcpy((s)->p,v,n); (s)->p += n; }
#define out_uint8a(s,v,n)	out_uint8p(s,v,n);
#define out_uint8s(s,n)		{ memset((s)->p,0,n); (s)->p += n; }

#endif
