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
#include <algorithm>
#include <QDebug>
#include <QBitmap>

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
static int bpp = 32;
static QPixmap * pixmap = nullptr;
static uint32 * colmap;

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

static QPainter::CompositionMode rop2_map[] = {
    /* https://doc.qt.io/qt-5/qpainter.html#CompositionMode-enum */
    QPainter::RasterOp_ClearDestination,		/* 0 */
    QPainter::RasterOp_NotSourceAndNotDestination,			/* DPon */
    QPainter::RasterOp_NotSourceAndDestination,		/* DPna */
    QPainter::RasterOp_NotSource,		/* Pn */
    QPainter::RasterOp_SourceAndNotDestination,		/* PDna */
    QPainter::RasterOp_NotDestination,		/* Dn */
    QPainter::RasterOp_SourceXorDestination,			/* DPx */
    QPainter::RasterOp_NotSourceOrNotDestination,			/* DPan */
    QPainter::RasterOp_SourceAndDestination,			/* DPa */
    QPainter::RasterOp_NotSourceXorDestination,		/* DPxn */
    QPainter::CompositionMode_Destination,			/* D */
    QPainter::RasterOp_NotSourceOrDestination,		/* DPno */
    QPainter::CompositionMode_Source,			/* P */
    QPainter::RasterOp_SourceOrNotDestination,		/* PDno */
    QPainter::RasterOp_SourceOrDestination,			/* DPo */
    QPainter::RasterOp_SetDestination			/* 1 */
};


static std::map<int, QImage::Format> bit_to_format;

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
    int size = width * height * bpp / 8;
    uint8 *out = (uint8 *)malloc(size * sizeof(uint8));
    uint8 *end = out + size;

    switch (bpp)
    {
        case 8:
            translate8(data, out, end);
            break;

        case 16:
            translate16(data, (uint16 *) out, (uint16 *) end);
            break;

        case 24:
            translate24(data, out, end);
            break;

        case 32:
            translate32(data, (uint32 *) out, (uint32 *) end);
            break;
    }

    return out;
}

void
mwm_hide_decorations(void)
{
    throw not_implemented_error();
}

BOOL
get_key_state(unsigned int state, uint32 keysym)
{
    assert(state == keysym);
    throw not_implemented_error();
}

BOOL
ui_init(MainWindow * w)
{
    window = w;
    window->setFixedSize(width, height);
    window->getPanel()->setGeometry(0, 0, width, height);
    pixmap = new QPixmap(width, height);    
    window->getPanel()->setPixmap(*pixmap);
    window->show();

    bit_to_format[8] = QImage::Format_Grayscale8;
    bit_to_format[16] = QImage::Format_RGB16;
    bit_to_format[24] = QImage::Format_RGB888;
    bit_to_format[32] = QImage::Format_RGBX8888;
    return True;
}

void
ui_deinit(void)
{
    return;
}

BOOL
ui_create_window(void)
{
    return True;
}

void
ui_destroy_window(void)
{
    return;
}

void
xwin_toggle_fullscreen(void)
{
    throw not_implemented_error();
}


/* Returns 0 after user quit, 1 otherwise */
int
ui_select(int rdp_socket)
{
    assert(rdp_socket == 0);
    throw not_implemented_error();
}

void
ui_move_pointer(int x, int y)
{
    assert(x == y);
    throw not_implemented_error();
}

HRDPBITMAP
ui_create_bitmap(int width, int height, uint8 * data)
{
    QImage *image;
    uint8 *tdata;
    tdata = translate_image(width, height, data);
    image = new QImage(tdata, width, height, bit_to_format[bpp]);
    return image;
}

void
ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 * data)
{
    uint8 *tdata;
    tdata = translate_image(width, height, data);
    QImage * image = new QImage(tdata, width, height, bit_to_format[bpp]);
    QPainter *painter = new QPainter(pixmap);
    QRect srcRect(0, 0, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, *image, srcRect); 
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
ui_destroy_bitmap(HRDPBITMAP bmp)
{
    uint8 * data = (uint8 *)bmp->constBits();
    delete bmp;
    free(data);
}

#include <stdio.h>
void print_to_file(const char * filename, uchar * data, int width, int height) {
    FILE * fp = fopen(filename, "w");
    if (!fp) {
        perror("ERROR");
    }
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            fprintf(fp, "%s ", data[i * width + j] ? "@" : " ");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}
void print_data(const char * filename, const uchar * data, int size) {
    FILE * fp = fopen(filename, "w");
    if (!fp) {
        perror("ERROR");
    }
    for (int i = 0; i < size; ++i) {
        fprintf(fp, "%d ", (int)data[i]);
    }
    printf("\n");
    fclose(fp);
}

HGLYPH
ui_create_glyph(int width, int height, uint8 * data)
{
    uint8 * convdata = new uint8[width * height];
    memset(convdata, 0, sizeof(uint8) * width * height);
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        int Lwidth = (width + 7) / 8;
        int start = (i * Lwidth) + j / 8;
        int shift = j % 8;
        if ((data[start] & (0x80 >> shift)) != 0)
        {
            convdata[i * width + j] = 255;
        }
      }
    }
    if (width > 10) {
        print_data("/home/user/debug.txt", data, width * height / 8);
        print_to_file("/home/user/1.txt", convdata, width, height);
    }
    QImage *image = new QImage(convdata, width, height, QImage::Format_Grayscale8);
    return image;
}

void
ui_destroy_glyph(HGLYPH glyph)
{
    delete glyph;
}

HRDPCURSOR
ui_create_cursor(unsigned int x, unsigned int y, int width, int height,
                 uint8 * andmask, uint8 * xormask)
{
   x = y = width = height = *andmask = *xormask;
   return (HRDPCURSOR)1;
}

void
ui_set_cursor(HRDPCURSOR cursor)
{
    assert(cursor != NULL);
    //throw not_implemented_error();
}

void
ui_destroy_cursor(HRDPCURSOR cursor)
{
    assert(cursor != NULL);
    //throw not_implemented_error();
}

HCOLOURMAP
ui_create_colourmap(COLOURMAP * colours)
{
    int i;
    int n;

    n = std::min(256, (int)colours->ncolours);
    unsigned * pal_entries = new unsigned[n];
    memset(pal_entries, 0, n * sizeof(unsigned));
    for (i = 0; i < n; i++)
    {
      pal_entries[i] = (colours->colours[i].red << 16) |
                       (colours->colours[i].green << 8) |
                       colours->colours[i].blue;
    }
    return pal_entries;
}

void
ui_destroy_colourmap(HCOLOURMAP map)
{
    delete (unsigned *)map;
}

void
ui_set_colourmap(HCOLOURMAP map)
{
    colmap = (unsigned *)map;
}

void
ui_set_clip(int x, int y, int cx, int cy)
{
    assert(x = y = cx = cy);
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
    QApplication::beep();
}

void
ui_destblt(uint8 opcode,
           /* dest */ int x, int y, int cx, int cy)
{
    if (opcode != 12){
        throw not_implemented_error{};
    }
    QPainter *painter = new QPainter(pixmap);
    painter->setCompositionMode(rop2_map[opcode]);
    QBrush brush;
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    painter->fillRect(x, y, cx, cy, brush); 
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
ui_patblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
    return;
    QPixmap * fill = new QBitmap();
    uint8 ipattern[8];
    QPainter *painter = new QPainter(pixmap);
    painter->setCompositionMode(rop2_map[opcode]);
    QBrush realBrush;
    QPen pen;
    switch(brush->style){
    case 0:
        realBrush.setColor(colmap[fgcolour]);
        realBrush.setStyle(Qt::BrushStyle::SolidPattern);
        painter->setBrush(realBrush);
        painter->fillRect(x, y, cx, cy, realBrush);
        break;
    case 3:
        *fill = QBitmap::fromImage(*ui_create_glyph(8, 8, ipattern));
        realBrush.setColor(colmap[fgcolour]);
        realBrush.setTexture(*fill);
        realBrush.setStyle(Qt::BrushStyle::TexturePattern);
        painter->setBrush(realBrush);
        painter->fillRect(x, y, cx, cy, realBrush);
        break;
    } 
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
ui_screenblt(uint8 opcode,
             /* dest */ int x, int y, int cx, int cy,
             /* src */ int srcx, int srcy)
{
    assert(opcode = x = y = cx = cy = srcx = srcy);
    throw not_implemented_error();
}

void
ui_memblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* src */ HRDPBITMAP src, int srcx, int srcy)
{
    if (opcode != 12){
        throw not_implemented_error{};
    }
    QPainter *painter = new QPainter(pixmap);
    painter->setCompositionMode(rop2_map[opcode]);
    QRect srcRect(srcx, srcy, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, *src, srcRect);
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
ui_triblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* src */ HRDPBITMAP src, int srcx, int srcy,
          /* brush */ BRUSH * brush, int bgcolour, int fgcolour)
{
    /* This is potentially difficult to do in general. Until someone
       comes up with a more efficient way of doing it I am using cases. */
    if (opcode != 12){
        throw not_implemented_error{};
    }
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
    QPainter *painter = new QPainter(pixmap);
    painter->setCompositionMode(rop2_map[opcode]);
    QPen mypen;
    mypen.setColor(colmap[pen->colour]);
    painter->setPen(mypen);
    painter->drawLine(startx, starty, endx, endy); 
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
ui_rect(
        /* dest */ int x, int y, int cx, int cy,
        /* brush */ int colour)
{
    QPainter *painter = new QPainter(pixmap);
    QBrush realBrush;
    realBrush.setColor(colmap[colour]);
    realBrush.setStyle(Qt::BrushStyle::SolidPattern);
    painter->setBrush(realBrush);
    painter->fillRect(x, y, cx, cy, realBrush); 
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
draw_glyph(int x, int y, QImage * glyphImage, int fgcolor)
{
    const uchar * glyph_data = glyphImage->constBits();
    QImage image = pixmap->toImage();
    int g_width = pixmap->width(), g_height = pixmap->height(), g_Bpp = image.depth() / 8;
    int glyph_width = glyphImage->width();
    int glyph_height = glyphImage->height();
    uchar * g_bs = new uchar[g_width * g_height * g_Bpp * sizeof(uchar)];
    memcpy(g_bs, image.constBits(), g_width * g_height * g_Bpp * sizeof(uchar));
    for (int i = 0; i < glyph_height; ++i){
        for (int j = 0; j < glyph_width; ++j){
            if (glyph_data[i * glyph_width + j] != 0){
                uchar * p = g_bs + ((y + i) * g_width * g_Bpp) + ((x + j) * g_Bpp);
                *((unsigned int *) p) = colmap[fgcolor];
            }
        }
    }
    image = QImage(g_bs, image.width(), image.height(), image.format());
    *pixmap = QPixmap::fromImage(image);
}

void DO_GLYPH(uint8 & font, unsigned char * ttext, int & idx,
              unsigned char & flags, int & xyoffset,
              int & x, int & y, int & mixmode, int & bgcolour, int & fgcolour){
    FONTGLYPH * glyph = cache_get_font(font, ttext[idx]);
    if (!(flags & TEXT2_IMPLICIT_X))
    {
        xyoffset = ttext[++idx];
        if ((xyoffset & 0x80))
        {
            if (flags & TEXT2_VERTICAL)
                y += ttext[idx+1] | (ttext[idx+2] << 8);
            else
                x += ttext[idx+1] | (ttext[idx+2] << 8);
            idx += 2;
        }
        else
        {
            if (flags & TEXT2_VERTICAL)
                y += xyoffset;
            else
                x += xyoffset;
        }
    }
    if (glyph != NULL)
    {
        draw_glyph(x + glyph->offset, y + glyph->baseline, glyph->pixmap,
                      fgcolour);
        if (flags & TEXT2_IMPLICIT_X)
            x += glyph->width;
    }
}

void
ui_draw_text(uint8 font, uint8 flags, int mixmode, int x, int y,
             int clipx, int clipy, int clipcx, int clipcy,
             int boxx, int boxy, int boxcx, int boxcy, int bgcolour,
             int fgcolour, uint8 * text, uint8 length)
{
    int i, j, xyoffset;
    DATABLOB *entry;

    QPainter *painter = new QPainter(pixmap);
    QBrush realBrush;
    realBrush.setStyle(Qt::BrushStyle::SolidPattern);
    realBrush.setColor(colmap[bgcolour]);
    painter->setBrush(realBrush);
    if (boxcx > 1)
    {
        painter->fillRect(boxx, boxy, boxcx, boxcy, realBrush);
    }
    else if (mixmode == MIX_OPAQUE)
    {
        painter->fillRect(clipx, clipy, clipcx, clipcy, realBrush);
    }
    delete painter;

    /* Paint text, character by character */
    for (i = 0; i < length;)
    {
        switch (text[i])
        {
        case 0xff:
            if (i + 2 < length)
                cache_put_text(text[i + 1], text, text[i + 2]);
            else
            {
                error("this shouldn't be happening\n");
                exit(1);
            }
            /* this will move pointer from start to first character after FF command */
            length -= i + 3;
            text = &(text[i + 3]);
            i = 0;
            break;

        case 0xfe:
            entry = cache_get_text(text[i + 1]);
            if (entry != NULL)
            {
                if ((((uint8 *) (entry->data))[1] ==
                     0) && (!(flags & TEXT2_IMPLICIT_X)))
                {
                    if (flags & TEXT2_VERTICAL)
                        y += text[i + 2];
                    else
                        x += text[i + 2];
                }
                for (j = 0; j < entry->size; j++)
                    DO_GLYPH(font, ((uint8 *) (entry->data)), j,
                             flags, xyoffset, x, y, mixmode, bgcolour, fgcolour);
            }
            if (i + 2 < length)
                i += 3;
            else
                i += 2;
            length -= i;
            /* this will move pointer from start to first character after FE command */
            text = &(text[i]);
            i = 0;
            break;

        default:
            DO_GLYPH(font, text, i,
                     flags, xyoffset, x, y, mixmode, bgcolour, fgcolour);
            i++;
            break;
        }
    } 
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
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
    QImage image(data, width, height, bit_to_format[bpp]);
    ui_memblt(12, x, y, cx, cy, &image, 0, 0);
}
