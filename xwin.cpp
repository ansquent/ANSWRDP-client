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

#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QApplication>
#include <QVector>
#include <QRgb>
#include <algorithm>
#include <QBitmap>
#include "xwin.h"

void info(const char *format, ...);

typedef struct {
    uint32 flags;
    uint32 functions;
    uint32 decorations;
    sint32 inputMode;
    uint32 status;
}
        PropMotifWmHints;

XWin_Ui::XWin_Ui(int width, int height, int bpp) {
    //info("XWin_Ui");
    this->width = width;
    this->height = height;
    this->bpp = bpp;
    init_bit_to_format();
    init_rop2_map();
    memset_0();
    pixmap = new QPixmap(width, height);
    colmap = nullptr;
    this->clipx = 0;
    this->clipy = 0;
    this->clipcx = width;
    this->clipcy = height;
}

XWin_Ui::~XWin_Ui() {
    //info("~XWin_Ui");
    delete[]rop2_map;
    delete[]colmap;
    delete pixmap;
    /* cursorcache delete */
    for (size_t i = 0; i < NUM_ELEMENTS(cursorcache); ++i) {
        ui_destroy_cursor(cursorcache[i]);
        cursorcache[i] = nullptr;
    }
    /* deskcache delete */

    /* textcache delete */
    for (size_t i = 0; i < NUM_ELEMENTS(textcache); ++i) {
        delete[] textcache[i].data;
    }
    /* fontcache delete */
    for (size_t i = 0; i < sizeof(fontcache) / sizeof(fontcache[0]); ++i) {
        for (int j = 0; j < sizeof(fontcache[0]) / sizeof(fontcache[0][0]); ++j) {
            ui_destroy_glyph(fontcache[i][j].pixmap);
        }
    }
    /* bmpcache delete */
    for (size_t i = 0; i < sizeof(bmpcache) / sizeof(bmpcache[0]); ++i) {
        for (int j = 0; j < sizeof(bmpcache[0]) / sizeof(bmpcache[0][0]); ++j) {
            ui_destroy_bitmap(bmpcache[i][j]);
        }
    }
}


void
XWin_Ui::ui_move_pointer(int x, int y) {
    info("ui_move_pointer not implemented");
    throw not_implemented_error();
}

HRDPBITMAP
XWin_Ui::ui_create_bitmap(int width, int height, uint8 *data) {
    //info("ui_create_bitmap");
    uint8 *tdata = translate_image(width, height, data);
    QImage *image = new QImage(width, height, QImage::Format_RGB32);
    for (int i = 0; i < height; ++i) {
        memcpy(image->scanLine(i), &tdata[i * width * bpp / 8], width * bpp / 8);
    }
    delete[]tdata;
    return image;
}

void
XWin_Ui::ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 *data) {
    //info("ui_paint_bitmap");
    uint8 *tdata = translate_image(width, height, data);
    QImage *image = new QImage(width, height, QImage::Format_RGB32);
    for (int i = 0; i < height; ++i) {
        memcpy(image->scanLine(i), &tdata[i * width * bpp / 8], width * bpp / 8);
    }
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    QRect srcRect(0, 0, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, *image, srcRect);
    delete image;
    delete painter;
    delete[]tdata;
}

void
XWin_Ui::ui_destroy_bitmap(HRDPBITMAP bmp) {
    delete bmp;
}


HGLYPH
XWin_Ui::ui_create_glyph(int width, int height, const uint8 *data) {
    //info("ui_create_glyph");
    uint8 *convdata = new uint8[width * height];
    memset(convdata, 0, sizeof(uint8) * width * height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int start = (width + 7) / 8 * i + j / 8;
            int shift = j % 8;
            if ((data[start] & (0x80 >> shift)) != 0)
                convdata[i * width + j] = 255;
        }
    }
    QImage *image = new QImage(width, height, QImage::Format_Grayscale8);
    for (int i = 0; i < height; ++i) {
        memcpy(image->scanLine(i), convdata + i * width, width);
    }
    delete[]convdata;
    return image;
}

void
XWin_Ui::ui_destroy_glyph(HGLYPH glyph) {
    //info("ui_destroy_glyph");
    delete glyph;
}

HRDPCURSOR
XWin_Ui::ui_create_cursor(unsigned int x, unsigned int y, int width, int height,
                          uint8 *andmask, uint8 *xormask) {
    //info("ui_create_cursor not implemented");
    //throw not_implemented_error();
    return nullptr;
}

void
XWin_Ui::ui_set_cursor(HRDPCURSOR cursor) {
    //info("ui_set_cursor not implemented");
    //throw not_implemented_error();
}

void
XWin_Ui::ui_destroy_cursor(HRDPCURSOR cursor) {
    //info("ui_destroy_cursor not implemented");
    //throw not_implemented_error();
}

void
XWin_Ui::ui_create_colourmap(COLOURMAP *colours) {
    //info("ui_create_colourmap");
    delete[]colmap;
    int n = std::min(256, (int) colours->ncolours);
    colmap = new unsigned[n];
    for (int i = 0; i < n; i++) {
        colmap[i] = (colours->colours[i].red << 16) |
                    (colours->colours[i].green << 8) |
                    colours->colours[i].blue;
    }
}

void
XWin_Ui::ui_set_clip(int x, int y, int cx, int cy) {
    //info("ui_set_clip");
    this->clipx = x;
    this->clipy = y;
    this->clipcx = cx;
    this->clipcy = cy;
}

void
XWin_Ui::ui_reset_clip() {
//    info("ui_reset_clip");
    this->clipx = 0;
    this->clipy = 0;
    this->clipcx = width;
    this->clipcy = height;
}

void
XWin_Ui::ui_bell() {
    //info("ui_bell");
    QApplication::beep();
}

void
XWin_Ui::ui_destblt(uint8 opcode,
        /* dest */ int x, int y, int cx, int cy) {
    //info("ui_destblt");
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    painter->setCompositionMode(rop2_map[opcode]);
    QBrush brush;
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    painter->fillRect(x, y, cx, cy, brush);
    delete painter;
}

void
XWin_Ui::ui_patblt(uint8 opcode, int x, int y, int cx, int cy, BRUSH *brush, int fgcolour) {
    //info("ui_patblt");
    uint8 ipattern[8];
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    painter->setCompositionMode(rop2_map[opcode]);
    QBrush realBrush;
    QPen pen;
    switch (brush->style) {
        case 0:
            realBrush.setColor(colmap[fgcolour]);
            realBrush.setStyle(Qt::BrushStyle::SolidPattern);
            painter->setBrush(realBrush);
            painter->fillRect(x, y, cx, cy, realBrush);
            break;
        case 3:
            for (int i = 0; i != 8; i++)
                ipattern[7 - i] = brush->pattern[i];
            QImage *image = ui_create_glyph(8, 8, ipattern);
            QBitmap fill = QBitmap::fromImage(*image);
            realBrush.setColor(colmap[fgcolour]);
            realBrush.setTexture(fill);
            realBrush.setStyle(Qt::BrushStyle::TexturePattern);
            painter->setBrush(realBrush);
            painter->fillRect(x, y, cx, cy, realBrush);
            ui_destroy_glyph(image);
            break;
    }
    delete painter;
}

void
XWin_Ui::ui_screenblt(uint8 opcode,
        /* dest */ int x, int y, int cx, int cy,
        /* src */ int srcx, int srcy) {
    //info("ui_screenblt");
    QImage src = pixmap->toImage();
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    painter->setCompositionMode(rop2_map[opcode]);
    QRect srcRect(srcx, srcy, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, src, srcRect);
    delete painter;
}

void
XWin_Ui::ui_memblt(uint8 opcode,
        /* dest */ int x, int y, int cx, int cy,
        /* src */ HRDPBITMAP src, int srcx, int srcy) {
    //info("ui_memblt");
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    painter->setCompositionMode(rop2_map[opcode]);
    QRect srcRect(srcx, srcy, cx, cy), destRect(x, y, cx, cy);
    painter->drawImage(destRect, *src, srcRect);
    delete painter;
}

void
XWin_Ui::ui_triblt(uint8 opcode,
        /* dest */ int x, int y, int cx, int cy,
        /* src */ HRDPBITMAP src, int srcx, int srcy,
        /* brush */ BRUSH *brush, int bgcolour, int fgcolour) {
    /* This is potentially difficult to do in general. Until someone
       comes up with a more efficient way of doing it I am using cases. */
    //info("ui_triblt");
    switch (opcode) {
        case 0x69:    /* PDSxxn */
            ui_memblt(ROP2_XOR, x, y, cx, cy, src, srcx, srcy);
            ui_patblt(ROP2_NXOR, x, y, cx, cy, brush, fgcolour);
            break;

        case 0xb8:    /* PSDPxax */
            ui_patblt(ROP2_XOR, x, y, cx, cy, brush, fgcolour);
            ui_memblt(ROP2_AND, x, y, cx, cy, src, srcx, srcy);
            ui_patblt(ROP2_XOR, x, y, cx, cy, brush, fgcolour);
            break;

        case 0xc0:    /* PSa */
            ui_memblt(ROP2_COPY, x, y, cx, cy, src, srcx, srcy);
            ui_patblt(ROP2_AND, x, y, cx, cy, brush, fgcolour);
            break;

        default:
            //info("Not implemented opcode %d in triblt", opcode);
            ui_memblt(ROP2_COPY, x, y, cx, cy, src, srcx, srcy);
    }
}

void
XWin_Ui::ui_line(uint8 opcode,
        /* dest */ int startx, int starty, int endx, int endy,
        /* pen */ PEN *pen) {
    //info("ui_line");
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    painter->setCompositionMode(rop2_map[opcode]);
    QPen mypen;
    mypen.setColor(colmap[pen->colour]);
    painter->setPen(mypen);
    painter->drawLine(startx, starty, endx, endy);
    delete painter;
}

void
XWin_Ui::ui_rect(
        /* dest */ int x, int y, int cx, int cy,
        /* brush */ int colour) {
    //info("ui_rect");
    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    QBrush brush;
    brush.setColor(colmap[colour]);
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    painter->setBrush(brush);
    painter->fillRect(x, y, cx, cy, brush);
    delete painter;
}

void
XWin_Ui::draw_glyph(int x, int y, QImage *glyphImage, int fgcolor) {
    //info("draw_glyph");
    QImage image = pixmap->toImage();
    int bpp = image.depth() / 8;
    int glyph_width = glyphImage->width();
    int glyph_height = glyphImage->height();
    //uint8 * bs = new uint8[width * height * bpp * sizeof(uint8)];
    for (int i = 0; i < glyph_height; ++i) {
        for (int j = 0; j < glyph_width; ++j) {
            uint8 *pixel = glyphImage->scanLine(i) + j;
            if (*pixel != 0) {
                uint8 *p = image.scanLine(y + i) + ((x + j) * bpp);
                *((unsigned int *) p) = colmap[fgcolor];
            }
        }
    }
    *pixmap = QPixmap::fromImage(image);
}

void XWin_Ui::DO_GLYPH(uint8 &font, unsigned char *ttext, int &idx,
                       unsigned char &flags, int &xyoffset,
                       int &x, int &y, int &mixmode, int &bgcolour, int &fgcolour) {
    //info("DO_GLYPH");
    FONTGLYPH *glyph = cache_get_font(font, ttext[idx]);
    if (!(flags & TEXT2_IMPLICIT_X)) {
        xyoffset = ttext[++idx];
        if ((xyoffset & 0x80)) {
            if (flags & TEXT2_VERTICAL)
                y += ttext[idx + 1] | (ttext[idx + 2] << 8);
            else
                x += ttext[idx + 1] | (ttext[idx + 2] << 8);
            idx += 2;
        } else {
            if (flags & TEXT2_VERTICAL)
                y += xyoffset;
            else
                x += xyoffset;
        }
    }
    if (glyph != nullptr) {
        draw_glyph(x + glyph->offset, y + glyph->baseline, glyph->pixmap,
                   fgcolour);
        if (flags & TEXT2_IMPLICIT_X)
            x += glyph->width;
    }
}

void
XWin_Ui::ui_draw_text(uint8 font, uint8 flags, int mixmode, int x, int y,
                      int clipx, int clipy, int clipcx, int clipcy,
                      int boxx, int boxy, int boxcx, int boxcy, int bgcolour,
                      int fgcolour, uint8 *text, uint8 length) {
    //info("ui_draw_text");
    int i, j, xyoffset;
    DATABLOB *entry;

    QPainter *painter = new QPainter(pixmap);
    painter->setClipRect(clipx, clipy, clipcx, clipcy);
    QBrush realBrush;
    realBrush.setStyle(Qt::BrushStyle::SolidPattern);
    realBrush.setColor(colmap[bgcolour]);
    painter->setBrush(realBrush);
    if (boxcx > 1) {
        painter->fillRect(boxx, boxy, boxcx, boxcy, realBrush);
    } else if (mixmode == MIX_OPAQUE) {
        painter->fillRect(clipx, clipy, clipcx, clipcy, realBrush);
    }
    delete painter;

    /* Paint text, character by character */
    for (i = 0; i < length;) {
        switch (text[i]) {
            case 0xff:
                if (i + 2 < length)
                    cache_put_text(text[i + 1], text, text[i + 2]);
                else {
                    info("this shouldn't be happening\n");
                    exit(1);
                }
                /* this will move pointer from start to first character after FF command */
                length -= i + 3;
                text = &(text[i + 3]);
                i = 0;
                break;

            case 0xfe:
                entry = cache_get_text(text[i + 1]);
                if (entry != nullptr) {
                    if ((((uint8 *) (entry->data))[1] ==
                         0) && (!(flags & TEXT2_IMPLICIT_X))) {
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
}

void
XWin_Ui::ui_desktop_save(uint32 offset, int x, int y, int cx, int cy) {
    //info("ui_desktop_save");
    QPixmap newPixmap(cx, cy);
    QPainter painter(&newPixmap);
    painter.drawPixmap(0, 0, *pixmap, x, y, cx, cy);
    QImage image = newPixmap.toImage();
    uint8 *data = new uint8[cx * cy * bpp / 8];
    for (int i = 0; i < cy; ++i) {
        memcpy(data + i * cx * bpp / 8, image.scanLine(i), cx * bpp / 8);
    }
    cache_put_desktop(offset, cx, cy, image.bytesPerLine(), bpp / 8, data);
    delete[]data;
}

void
XWin_Ui::ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy) {
    //info("ui_desktop_restore");
    offset *= bpp / 8;
    uint8 *data = cache_get_desktop(offset, cx, cy, bpp / 8);
    QImage image(width, height, bit_to_format[bpp]);
    for (int i = 0; i < cy; ++i) {
        memcpy(image.scanLine(i), data + i * cx * bpp / 8, cx * bpp / 8);
    }
    ui_memblt(12, x, y, cx, cy, &image, 0, 0);
}

/* Retrieve a bitmap from the cache */
HRDPBITMAP XWin_Ui::cache_get_bitmap(uint8 cache_id, uint16 cache_idx) {
    //info("cache_get_bitmap");
    HRDPBITMAP bitmap;

    if ((cache_id < NUM_ELEMENTS(bmpcache)) && (cache_idx < NUM_ELEMENTS(bmpcache[0]))) {
        bitmap = bmpcache[cache_id][cache_idx];
        if (bitmap != nullptr)
            return bitmap;
    }

    info("get bitmap %d:%d\n", cache_id, cache_idx);
    return nullptr;
}

/* Store a bitmap in the cache */
void XWin_Ui::cache_put_bitmap(uint8 cache_id, uint16 cache_idx, HRDPBITMAP bitmap) {
    //info("cache_put_bitmap");
    HRDPBITMAP old;

    if ((cache_id < NUM_ELEMENTS(bmpcache)) && (cache_idx < NUM_ELEMENTS(bmpcache[0]))) {
        if (bmpcache[cache_id][cache_idx] != nullptr) {
            ui_destroy_bitmap(bmpcache[cache_id][cache_idx]);
            bmpcache[cache_id][cache_idx] = nullptr;
        }

        bmpcache[cache_id][cache_idx] = bitmap;
    } else {
        info("put bitmap %d:%d\n", cache_id, cache_idx);
    }
}

/* Retrieve a glyph from the font cache */
FONTGLYPH *XWin_Ui::cache_get_font(uint8 font, uint16 character) {
    //info("cache_get_font");
    FONTGLYPH *glyph;

    if ((font < NUM_ELEMENTS(fontcache)) && (character < NUM_ELEMENTS(fontcache[0]))) {
        glyph = &fontcache[font][character];
        if (glyph->pixmap != nullptr)
            return glyph;
    }

    info("get font %d:%d\n", font, character);
    return nullptr;
}

/* Store a glyph in the font cache */
void XWin_Ui::cache_put_font(uint8 font, uint16 character, uint16 offset,
                             uint16 baseline, uint16 width, uint16 height, HGLYPH pixmap) {
    //info("cache_put_font");
    FONTGLYPH *glyph;

    if ((font < NUM_ELEMENTS(fontcache)) && (character < NUM_ELEMENTS(fontcache[0]))) {
        glyph = &fontcache[font][character];
        if (glyph->pixmap != nullptr) {
            ui_destroy_glyph(glyph->pixmap);
            glyph->pixmap = nullptr;
        }

        glyph->offset = offset;
        glyph->baseline = baseline;
        glyph->width = width;
        glyph->height = height;
        glyph->pixmap = pixmap;
    } else {
        info("put font %d:%d\n", font, character);
    }
}

/* Retrieve a text item from the cache */
DATABLOB *XWin_Ui::cache_get_text(uint8 cache_id) {
    //info("cache_get_text");
    DATABLOB *text;

    if (cache_id < NUM_ELEMENTS(textcache)) {
        text = &textcache[cache_id];
        if (text->data != nullptr)
            return text;
    }

    info("get text %d\n", cache_id);
    return nullptr;
}

/* Store a text item in the cache */
void XWin_Ui::cache_put_text(uint8 cache_id, void *data, int length) {
    //info("cache_put_text");
    DATABLOB *text;

    if (cache_id < NUM_ELEMENTS(textcache)) {
        text = &textcache[cache_id];
        delete[]text->data;

        text->data = new uint8[length];
        text->size = length;
        memcpy(text->data, data, length);
    } else {
        info("put text %d\n", cache_id);
    }
}


/* Retrieve desktop data from the cache */
uint8 *XWin_Ui::cache_get_desktop(uint32 offset, int cx, int cy, int bytes_per_pixel) {
    //info("cache_get_desktop");
    int length = cx * cy * bytes_per_pixel;

    if ((offset + length) <= sizeof(deskcache)) {
        return &deskcache[offset];
    }

    info("get desktop %d:%d\n", offset, length);
    return nullptr;
}

/* Store desktop data in the cache */
void XWin_Ui::cache_put_desktop(uint32 offset, int cx, int cy, int scanline, int bytes_per_pixel, uint8 *data) {
    //info("cache_put_desktop");
    int length = cx * cy * bytes_per_pixel;

    if ((offset + length) <= sizeof(deskcache)) {
        cx *= bytes_per_pixel;
        while (cy--) {
            memcpy(&deskcache[offset], data, cx);
            data += scanline;
            offset += cx;
        }
    } else {
        info("put desktop %d:%d\n", offset, length);
    }
}


/* Retrieve cursor from cache */
HRDPCURSOR XWin_Ui::cache_get_cursor(uint16 cache_idx) {
    //info("cache_get_cursor");
    HRDPCURSOR cursor;

    if (cache_idx < NUM_ELEMENTS(cursorcache)) {
        cursor = cursorcache[cache_idx];
        if (cursor != nullptr)
            return cursor;
    }

    //info("get cursor %d\n", cache_idx);
    return nullptr;
}

/* Store cursor in cache */
void XWin_Ui::cache_put_cursor(uint16 cache_idx, HRDPCURSOR cursor) {
    //info("cache_put_cursor");

    if (cache_idx < NUM_ELEMENTS(cursorcache)) {
        if (cursorcache[cache_idx] != nullptr) {
            ui_destroy_cursor(cursorcache[cache_idx]);
        }

        cursorcache[cache_idx] = cursor;
    } else {
        info("put cursor %d\n", cache_idx);
    }
}

int XWin_Ui::get_height() const {
    return height;
}

int XWin_Ui::get_width() const {
    return width;
}
