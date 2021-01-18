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
#include <mainwindow.h>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QObject>
#include <QApplication>
#include <QVector>
#include <QRgb>

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

static MainWindow * window = nullptr;
static QImage::Format bpp = QImage::Format_RGB888;
static QPixmap * pixmap = nullptr;

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

BOOL
get_key_state(unsigned int state, uint32 keysym)
{
    throw not_implemented_error();
}

BOOL
ui_init(MainWindow * w)
{
    //	throw not_implemented_error();
    window = w;
    window->setFixedSize(width, height);
    window->getPanel()->setGeometry(0, 0, width, height);
    pixmap = new QPixmap(width, height);
    window->show();
    return True;
}

void
ui_deinit(void)
{
    throw not_implemented_error();
}

BOOL
ui_create_window(void)
{
    return True;
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

HRDPBITMAP
ui_create_bitmap(int width, int height, uint8 * data)
{
    //throw not_implemented_error();
    QImage * result = new QImage(data, width, height, QImage::Format_Grayscale8);
    *result = result->convertToFormat(bpp);
    return result;
}

void
ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 * data)
{
    //throw not_implemented_error();
    QImage image(data, width, height, bpp);
    window->getPanel()->setPixmap(QPixmap::fromImage(image));
    window->repaint();
}

void
ui_destroy_bitmap(HRDPBITMAP bmp)
{
    //throw not_implemented_error();
    delete bmp;
}

HGLYPH
ui_create_glyph(int width, int height, uint8 * data)
{
    return (HGLYPH)1;
    throw not_implemented_error();
}

void
ui_destroy_glyph(HGLYPH glyph)
{
    return;
    throw not_implemented_error();
}

HRDPCURSOR
ui_create_cursor(unsigned int x, unsigned int y, int width, int height,
         uint8 * andmask, uint8 * xormask)
{
    return (HRDPCURSOR)1;
    throw not_implemented_error();
}

void
ui_set_cursor(HRDPCURSOR cursor)
{
    //throw not_implemented_error();
}

void
ui_destroy_cursor(HRDPCURSOR cursor)
{
    //throw not_implemented_error();
}

HCOLOURMAP
ui_create_colourmap(COLOURMAP * colours)
{
    return NULL;
    //throw not_implemented_error();
}

void
ui_destroy_colourmap(HCOLOURMAP map)
{
    //throw not_implemented_error();
}

void
ui_set_colourmap(HCOLOURMAP map)
{
    //throw not_implemented_error();
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
    //throw not_implemented_error();
    QApplication::beep();
}

void
ui_destblt(uint8 opcode,
       /* dest */ int x, int y, int cx, int cy)
{
    //throw not_implemented_error();
}

void
ui_patblt(uint8 opcode,
      /* dest */ int x, int y, int cx, int cy,
      /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
    //throw not_implemented_error();
}

void
ui_screenblt(uint8 opcode,
         /* dest */ int x, int y, int cx, int cy,
         /* src */ int srcx, int srcy)
{
    throw not_implemented_error();
}

QColor getColorByOpcode(int opcode, QColor nowc, QColor willc){
    QColor resultc;
    switch(opcode){
    case 0:
        resultc = QColor(0, 0, 0); break;
    case 1:
        resultc = QColor(~(nowc.red() | willc.red()),
                         ~(nowc.green() | willc.green()),
                         ~(nowc.blue() | willc.blue())
                         ); break;
    case 2:
        resultc = QColor(~nowc.red() & willc.red(),
                         ~nowc.green() & willc.green(),
                         ~nowc.blue() & willc.blue()
                         ); break;
    case 3:
    case 5:
        resultc = QColor(~willc.red(),
                         ~willc.green(),
                         ~willc.blue()
                         ); break;
    case 4:
        resultc = QColor(nowc.red() & ~willc.red(),
                         nowc.green() & ~willc.green(),
                         nowc.blue() & ~willc.blue()
                         ); break;
    case 6:
        resultc = QColor(nowc.red() ^ willc.red(),
                         nowc.green() ^ willc.green(),
                         nowc.blue() ^ willc.blue()
                         ); break;
    case 7:
        resultc = QColor(~(nowc.red() & willc.red()),
                         ~(nowc.green() & willc.green()),
                         ~(nowc.blue() & willc.blue())
                         ); break;
    case 8:
        resultc = QColor(nowc.red() & willc.red(),
                         nowc.green() & willc.green(),
                         nowc.blue() & willc.blue()
                         ); break;
    case 9:
        resultc = QColor(~nowc.red() ^ willc.red(),
                         ~nowc.green() ^ willc.green(),
                         ~nowc.blue() ^ willc.blue()
                         ); break;
    case 11:
        resultc = QColor(~nowc.red() | willc.red(),
                         ~nowc.green() | willc.green(),
                         ~nowc.blue() | willc.blue()
                         ); break;
    case 13:
        resultc = QColor(nowc.red() | ~willc.red(),
                         nowc.green() | ~willc.green(),
                         nowc.blue() | ~willc.blue()
                         ); break;
    case 14:
        resultc = QColor(nowc.red() | willc.red(),
                         nowc.green() | willc.green(),
                         nowc.blue() | willc.blue()
                         ); break;
    case 15:
        resultc = QColor(255, 255, 255); break;
    case 10:
    case 12:
        /* Do nothing */; break;
    default:
        warning("do_array unsupported opcode: %d", opcode);
    }
    return resultc;
}

void
ui_memblt(uint8 opcode,
      /* dest */ int x, int y, int cx, int cy,
      /* src */ HRDPBITMAP src, int srcx, int srcy)
{
    //throw not_implemented_error();
    QImage image = *src;
    QImage willImage = pixmap->toImage();

    for (int i = 0; i < cx; ++i){
        for (int j = 0; j < cy; ++j){
            int nowx = srcx + i;
            int nowy = srcy + j;
            int willx = x + i;
            int willy = y + j;
            if (nowx >= image.width() || nowy >= image.height()){
                throw not_implemented_error();
            }
            else if (willx >= willImage.width() || willy >= willImage.height()){
                throw not_implemented_error();
            }
            QColor nowc = image.pixelColor(nowx, nowy);
            QColor willc = willImage.pixelColor(willx, willy);
            QColor resultc = getColorByOpcode(0, nowc, willc);
            image.setPixelColor(nowx, nowy, resultc);
        }
    }

    QPixmap srcPixmap = QPixmap::fromImage(image);

    QPainter painter(pixmap);
    painter.drawPixmap(x, y, srcPixmap, srcx, srcy, cx, cy);

    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();

}

void
ui_triblt(uint8 opcode,
      /* dest */ int x, int y, int cx, int cy,
      /* src */ HRDPBITMAP src, int srcx, int srcy,
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
    // throw not_implemented_error();
    QPainter painter(pixmap);
    QPen mypen;
    mypen.setColor(pen->colour);
    painter.setPen(mypen);
    painter.drawLine(startx, starty, endx, endy);
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
}

void
ui_rect(
           /* dest */ int x, int y, int cx, int cy,
           /* brush */ int colour)
{
    QPainter painter(pixmap);
    QPen mypen;
    mypen.setColor(colour);
    painter.setPen(mypen);
    painter.drawRect(x, y, cx, cy);
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
}

/* warning, this function only draws on wnd or backstore, not both */
void
ui_draw_glyph(int mixmode,
          /* dest */ int x, int y, int cx, int cy,
          /* src */ HGLYPH glyph, int srcx, int srcy,
          int bgcolour, int fgcolour)
{
    //throw not_implemented_error();
}

void
ui_draw_text(uint8 font, uint8 flags, int mixmode, int x, int y,
         int clipx, int clipy, int clipcx, int clipcy,
         int boxx, int boxy, int boxcx, int boxcy, int bgcolour,
         int fgcolour, uint8 * text, uint8 length)
{
    //throw not_implemented_error();
}

void
ui_desktop_save(uint32 offset, int x, int y, int cx, int cy)
{
    QPixmap newPixmap(cx, cy);
    QPainter painter(&newPixmap);
    painter.drawPixmap(0, 0, *pixmap, x, y, cx, cy);
    QImage image = newPixmap.toImage();
    uint8 * data = image.bits();
    cache_put_desktop(offset, cx, cy, image.bytesPerLine(), bpp / 8, data);
}

void
ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy)
{
    offset *= bpp / 8;
    uint8 * data = cache_get_desktop(offset, cx, cy, bpp / 8);
    QImage image(data, width, height, bpp);
    ui_memblt(12, x, y, cx, cy, &image, 0, 0);
}
