/*
   rdesktop: A Remote Desktop Protocol client.
   User interface services - X Window System
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

// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
#include <time.h>
#include <errno.h>
#include "rdesktop.h"
#include <exception.h>

extern int width;
extern int height;
extern BOOL sendmotion;
extern BOOL fullscreen;
extern BOOL grab_keyboard;
extern BOOL hide_decorations;
extern char title[];
BOOL enable_compose = False;
BOOL focused;
BOOL mouse_in_wnd;

// Display *display;
static int x_socket;
// static Screen *screen;
// static Window wnd;
// static GC gc;
// static Visual *visual;
static int depth;
static int bpp;
// static XIM IM;
// static XIC IC;
// static XModifierKeymap *mod_map;
// static Cursor current_cursor;
// static Atom protocol_atom, kill_atom;

/* endianness */
static BOOL host_be;
static BOOL xserver_be;

/* software backing store */
static BOOL ownbackstore;
// static Pixmap backstore;

/* MWM decorations */
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define PROP_MOTIF_WM_HINTS_ELEMENTS    5
typedef struct
{
	uint32 flags;
	uint32 functions;
	uint32 decorations;
	sint32 inputMode;
	uint32 status;
}
PropMotifWmHints;


#define FILL_RECTANGLE(x,y,cx,cy)\
{ \
	XFillRectangle(display, wnd, gc, x, y, cx, cy); \
	if (ownbackstore) \
		XFillRectangle(display, backstore, gc, x, y, cx, cy); \
}

#define FILL_RECTANGLE_BACKSTORE(x,y,cx,cy)\
{ \
	XFillRectangle(display, ownbackstore ? backstore : wnd, gc, x, y, cx, cy); \
}

/* colour maps */
BOOL owncolmap = False;
// static Colormap xcolmap;
static uint32 *colmap;

#define TRANSLATE(col)		( owncolmap ? col : translate_colour(colmap[col]) )
#define SET_FOREGROUND(col)	XSetForeground(display, gc, TRANSLATE(col));
#define SET_BACKGROUND(col)	XSetBackground(display, gc, TRANSLATE(col));


#define SET_FUNCTION(rop2)	{ if (rop2 != ROP2_COPY) XSetFunction(display, gc, rop2_map[rop2]); }
#define RESET_FUNCTION(rop2)	{ if (rop2 != ROP2_COPY) XSetFunction(display, gc, GXcopy); }

void
mwm_hide_decorations(void)
{
	throw not_implemented_error();
}

static void
translate8(uint8 * data, uint8 * out, uint8 * end)
{
	while (out < end)
		*(out++) = (uint8) colmap[*(data++)];
}

static void
translate16(uint8 * data, uint16 * out, uint16 * end)
{
	while (out < end)
		*(out++) = (uint16) colmap[*(data++)];
}

/* little endian - conversion happens when colourmap is built */
static void
translate24(uint8 * data, uint8 * out, uint8 * end)
{
	uint32 value;

	while (out < end)
	{
		value = colmap[*(data++)];
		*(out++) = value;
		*(out++) = value >> 8;
		*(out++) = value >> 16;
	}
}

static void
translate32(uint8 * data, uint32 * out, uint32 * end)
{
	while (out < end)
		*(out++) = colmap[*(data++)];
}

static uint8 *
translate_image(int width, int height, uint8 * data)
{
	throw not_implemented_error();
}

#define BSWAP16(x) { x = (((x & 0xff) << 8) | (x >> 8)); }
#define BSWAP24(x) { x = (((x & 0xff) << 16) | (x >> 16) | ((x >> 8) & 0xff00)); }
#define BSWAP32(x) { x = (((x & 0xff00ff) << 8) | ((x >> 8) & 0xff00ff)); \
			x = (x << 16) | (x >> 16); }

static uint32
translate_colour(uint32 colour)
{
	switch (bpp)
	{
		case 16:
			if (host_be != xserver_be)
				BSWAP16(colour);
			break;

		case 24:
			if (xserver_be)
				BSWAP24(colour);
			break;

		case 32:
			if (host_be != xserver_be)
				BSWAP32(colour);
			break;
	}

	return colour;
}

BOOL
get_key_state(unsigned int state, uint32 keysym)
{
	throw not_implemented_error();
}

BOOL
ui_init(void)
{
	throw not_implemented_error();
}

void
ui_deinit(void)
{
	throw not_implemented_error();
}

BOOL
ui_create_window(void)
{
	throw not_implemented_error();
}

void
ui_destroy_window(void)
{
	throw not_implemented_error();
}

void
xwin_toggle_fullscreen(void)
{
	throw not_implemented_error();
}

/* Process all events in Xlib queue 
   Returns 0 after user quit, 1 otherwise */
static int
xwin_process_events(void)
{
	throw not_implemented_error();
}

/* Returns 0 after user quit, 1 otherwise */
int
ui_select(int rdp_socket)
{
	throw not_implemented_error();
}

void
ui_move_pointer(int x, int y)
{
	throw not_implemented_error();
}

HBITMAP
ui_create_bitmap(int width, int height, uint8 * data)
{
	throw not_implemented_error();
}

void
ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 * data)
{
	throw not_implemented_error();
}

void
ui_destroy_bitmap(HBITMAP bmp)
{
	throw not_implemented_error();
}

HGLYPH
ui_create_glyph(int width, int height, uint8 * data)
{
	throw not_implemented_error();
}

void
ui_destroy_glyph(HGLYPH glyph)
{
	throw not_implemented_error();
}

HCURSOR
ui_create_cursor(unsigned int x, unsigned int y, int width, int height,
		 uint8 * andmask, uint8 * xormask)
{
	throw not_implemented_error();
}

void
ui_set_cursor(HCURSOR cursor)
{
	throw not_implemented_error();
}

void
ui_destroy_cursor(HCURSOR cursor)
{
	throw not_implemented_error();
}

#define MAKE_XCOLOR(xc,c) \
		(xc)->red   = ((c)->red   << 8) | (c)->red; \
		(xc)->green = ((c)->green << 8) | (c)->green; \
		(xc)->blue  = ((c)->blue  << 8) | (c)->blue; \
		(xc)->flags = DoRed | DoGreen | DoBlue;


HCOLOURMAP
ui_create_colourmap(COLOURMAP * colours)
{
	throw not_implemented_error();
}

void
ui_destroy_colourmap(HCOLOURMAP map)
{
	throw not_implemented_error();
}

void
ui_set_colourmap(HCOLOURMAP map)
{
	throw not_implemented_error();
}

void
ui_set_clip(int x, int y, int cx, int cy)
{
	throw not_implemented_error();
}

void
ui_reset_clip(void)
{
	throw not_implemented_error();
}

void
ui_bell(void)
{
	throw not_implemented_error();
}

void
ui_destblt(uint8 opcode,
	   /* dest */ int x, int y, int cx, int cy)
{
	throw not_implemented_error();
}

void
ui_patblt(uint8 opcode,
	  /* dest */ int x, int y, int cx, int cy,
	  /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
	throw not_implemented_error();
}

void
ui_screenblt(uint8 opcode,
	     /* dest */ int x, int y, int cx, int cy,
	     /* src */ int srcx, int srcy)
{
	throw not_implemented_error();
}

void
ui_memblt(uint8 opcode,
	  /* dest */ int x, int y, int cx, int cy,
	  /* src */ HBITMAP src, int srcx, int srcy)
{
	throw not_implemented_error();
}

void
ui_triblt(uint8 opcode,
	  /* dest */ int x, int y, int cx, int cy,
	  /* src */ HBITMAP src, int srcx, int srcy,
	  /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
	/* This is potentially difficult to do in general. Until someone
	   comes up with a more efficient way of doing it I am using cases. */

	switch (opcode)
	{
		case 0x69:	/* PDSxxn */
			ui_memblt(ROP2_XOR, x, y, cx, cy, src, srcx, srcy);
			ui_patblt(ROP2_NXOR, x, y, cx, cy, brush, bgcolour, fgcolour);
			break;

		case 0xb8:	/* PSDPxax */
			ui_patblt(ROP2_XOR, x, y, cx, cy, brush, bgcolour, fgcolour);
			ui_memblt(ROP2_AND, x, y, cx, cy, src, srcx, srcy);
			ui_patblt(ROP2_XOR, x, y, cx, cy, brush, bgcolour, fgcolour);
			break;

		case 0xc0:	/* PSa */
			ui_memblt(ROP2_COPY, x, y, cx, cy, src, srcx, srcy);
			ui_patblt(ROP2_AND, x, y, cx, cy, brush, bgcolour, fgcolour);
			break;

		default:
			unimpl("triblt 0x%x\n", opcode);
			ui_memblt(ROP2_COPY, x, y, cx, cy, src, srcx, srcy);
	}
}

void
ui_line(uint8 opcode,
	/* dest */ int startx, int starty, int endx, int endy,
	/* pen */ PEN * pen)
{
	throw not_implemented_error();
}

void
ui_rect(
	       /* dest */ int x, int y, int cx, int cy,
	       /* brush */ int colour)
{
	throw not_implemented_error();
}

/* warning, this function only draws on wnd or backstore, not both */
void
ui_draw_glyph(int mixmode,
	      /* dest */ int x, int y, int cx, int cy,
	      /* src */ HGLYPH glyph, int srcx, int srcy,
	      int bgcolour, int fgcolour)
{
	throw not_implemented_error();
}

#define DO_GLYPH(ttext,idx) \
{\
  glyph = cache_get_font (font, ttext[idx]);\
  if (!(flags & TEXT2_IMPLICIT_X))\
    {\
      xyoffset = ttext[++idx];\
      if ((xyoffset & 0x80))\
	{\
	  if (flags & TEXT2_VERTICAL) \
	    y += ttext[idx+1] | (ttext[idx+2] << 8);\
	  else\
	    x += ttext[idx+1] | (ttext[idx+2] << 8);\
	  idx += 2;\
	}\
      else\
	{\
	  if (flags & TEXT2_VERTICAL) \
	    y += xyoffset;\
	  else\
	    x += xyoffset;\
	}\
    }\
  if (glyph != NULL)\
    {\
      ui_draw_glyph (mixmode, x + glyph->offset,\
		     y + glyph->baseline,\
		     glyph->width, glyph->height,\
		     glyph->pixmap, 0, 0, bgcolour, fgcolour);\
      if (flags & TEXT2_IMPLICIT_X)\
	x += glyph->width;\
    }\
}

void
ui_draw_text(uint8 font, uint8 flags, int mixmode, int x, int y,
	     int clipx, int clipy, int clipcx, int clipcy,
	     int boxx, int boxy, int boxcx, int boxcy, int bgcolour,
	     int fgcolour, uint8 * text, uint8 length)
{
	throw not_implemented_error();
}

void
ui_desktop_save(uint32 offset, int x, int y, int cx, int cy)
{
	throw not_implemented_error();
}

void
ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy)
{
	throw not_implemented_error();
}
