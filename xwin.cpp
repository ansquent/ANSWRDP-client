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
#include "xwin.h"

extern BOOL sendmotion;
extern BOOL fullscreen;
extern BOOL grab_keyboard;
extern BOOL hide_decorations;
extern char title[];
BOOL enable_compose = False;
BOOL focused;
BOOL mouse_in_wnd;

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
    fprintf(fp, "\n");
    fclose(fp);
}

UI::UI(MainWindow * mainwindow, int width, int height, int bpp){
    this->width = width;
    this->height = height;
    this->bpp = bpp;
    init_bit_to_format();
    init_rop2_map();
    window = mainwindow;
    window->setFixedSize(width, height);
    window->getPanel()->setGeometry(0, 0, width, height);
    pixmap = new QPixmap(width, height);
    window->getPanel()->setPixmap(*pixmap);
    window->show();
}

UI::~UI(){
    delete rop2_map;
    delete colmap;
    delete pixmap;
}

void
UI::mwm_hide_decorations(void)
{
    throw not_implemented_error();
}

BOOL
UI::get_key_state(unsigned int state, uint32 keysym)
{
    assert(state == keysym);
    throw not_implemented_error();
}

void
UI::xwin_toggle_fullscreen(void)
{
    throw not_implemented_error();
}


/* Returns 0 after user quit, 1 otherwise */
int
UI::ui_select(int rdp_socket)
{
    assert(rdp_socket == 0);
    throw not_implemented_error();
}

void
UI::ui_move_pointer(int x, int y)
{
    assert(x == y);
    throw not_implemented_error();
}

HRDPBITMAP
UI::ui_create_bitmap(int width, int height, uint8 * data)
{
    uint8 *tdata = translate_image(width, height, data);
    QImage *image = new QImage(tdata, width, height, 4 * width, QImage::Format_RGB32);
    return image;
}

void
UI::ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 * data)
{
    uint8 *tdata;
    tdata = translate_image(width, height, data);
    QImage * image = new QImage(tdata, width, height, 4 * width, QImage::Format_RGB32);
    QPainter *painter = new QPainter(pixmap);
    QRect srcRect(0, 0, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, *image, srcRect);
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
UI::ui_destroy_bitmap(HRDPBITMAP bmp)
{
    uint8 * data = (uint8 *)bmp->constBits();
    delete bmp;
    free(data);
}


HGLYPH
UI::ui_create_glyph(int width, int height, uint8 * data)
{
    uint8 * convdata = new uint8[width * height];
    memset(convdata, 0, sizeof(uint8) * width * height);
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        int start = (width + 7) / 8 * i + j / 8;
        int shift = j % 8;
        if ((data[start] & (0x80 >> shift)) != 0)
            convdata[i * width + j] = 255;
      }
    }
    QImage *image = new QImage(convdata, width, height, QImage::Format_Grayscale8);
    return image;
}

void
UI::ui_destroy_glyph(HGLYPH glyph)
{
    delete glyph;
}

HRDPCURSOR
UI::ui_create_cursor(unsigned int x, unsigned int y, int width, int height,
                 uint8 * andmask, uint8 * xormask)
{
   x = y = width = height = *andmask = *xormask;
   return (HRDPCURSOR)1;
}

void
UI::ui_set_cursor(HRDPCURSOR cursor)
{
    assert(cursor != NULL);
    //throw not_implemented_error();
}

void
UI::ui_destroy_cursor(HRDPCURSOR cursor)
{
    assert(cursor != NULL);
    //throw not_implemented_error();
}

void
UI::ui_create_colourmap(COLOURMAP * colours)
{
    int n = std::min(256, (int)colours->ncolours);
    colmap = new unsigned[n];
    memset(colmap, 0, n * sizeof(unsigned));
    for (int i = 0; i < n; i++)
    {
      colmap[i] = (colours->colours[i].red << 16) |
                       (colours->colours[i].green << 8) |
                       colours->colours[i].blue;
    }
}

void
UI::ui_set_clip(int x, int y, int cx, int cy)
{
    return;
}

void
UI::ui_reset_clip(void)
{
    return;
}

void
UI::ui_bell(void)
{
    QApplication::beep();
}

void
UI::ui_destblt(uint8 opcode,
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
UI::ui_patblt(uint8 opcode,
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
UI::ui_screenblt(uint8 opcode,
             /* dest */ int x, int y, int cx, int cy,
             /* src */ int srcx, int srcy)
{
    assert(opcode = x = y = cx = cy = srcx = srcy);
    throw not_implemented_error();
}

void
UI::ui_memblt(uint8 opcode,
          /* dest */ int x, int y, int cx, int cy,
          /* src */ HRDPBITMAP src, int srcx, int srcy)
{
    QPainter *painter = new QPainter(pixmap);
    painter->setCompositionMode(rop2_map[opcode]);
    QRect srcRect(srcx, srcy, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, *src, srcRect);
    window->getPanel()->setPixmap(*pixmap);
    window->getPanel()->repaint();
    delete painter;
}

void
UI::ui_triblt(uint8 opcode,
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
UI::ui_line(uint8 opcode,
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
UI::ui_rect(
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
UI::draw_glyph(int x, int y, QImage * glyphImage, int fgcolor)
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

void UI::DO_GLYPH(uint8 & font, unsigned char * ttext, int & idx,
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
UI::ui_draw_text(uint8 font, uint8 flags, int mixmode, int x, int y,
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
    window->getPanel()->repaint();
}

void
UI::ui_desktop_save(uint32 offset, int x, int y, int cx, int cy)
{
    QPixmap newPixmap(cx, cy);
    QPainter painter(&newPixmap);
    painter.drawPixmap(0, 0, *pixmap, x, y, cx, cy);
    QImage image = newPixmap.toImage();
    uint8 * data = image.bits();
    cache_put_desktop(offset, cx, cy, image.bytesPerLine(), bpp / 8, data);
}

void
UI::ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy)
{
    offset *= bpp / 8;
    uint8 * data = cache_get_desktop(offset, cx, cy, bpp / 8);
    QImage image(data, width, height, bit_to_format[bpp]);
    ui_memblt(12, x, y, cx, cy, &image, 0, 0);
}
