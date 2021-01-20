#ifndef XWIN_H
#define XWIN_H

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

void print_data(const char * filename, const uchar * data, int size);
class UI{
    private:
        std::map<int, QImage::Format> bit_to_format;
        int width, height, bpp;
        MainWindow * window;
        QPixmap * pixmap;
        QPainter::CompositionMode *rop2_map;
        uint32 * colmap;

        void init_bit_to_format(){
            bit_to_format[32] = QImage::Format_RGB32;
        }

        void init_rop2_map(){
            rop2_map = new QPainter::CompositionMode[16];
            rop2_map[0] = QPainter::RasterOp_ClearDestination;		/* 0 */
            rop2_map[1] = QPainter::RasterOp_NotSourceAndNotDestination;			/* DPon */
            rop2_map[2] = QPainter::RasterOp_NotSourceAndDestination;		/* DPna */
            rop2_map[3] = QPainter::RasterOp_NotSource;		/* Pn */
            rop2_map[4] = QPainter::RasterOp_SourceAndNotDestination;		/* PDna */
            rop2_map[5] = QPainter::RasterOp_NotDestination;		/* Dn */
            rop2_map[6] = QPainter::RasterOp_SourceXorDestination;			/* DPx */
            rop2_map[7] = QPainter::RasterOp_NotSourceOrNotDestination;			/* DPan */
            rop2_map[8] = QPainter::RasterOp_SourceAndDestination;			/* DPa */
            rop2_map[9] = QPainter::RasterOp_NotSourceXorDestination;		/* DPxn */
            rop2_map[10] = QPainter::CompositionMode_Destination;			/* D */
            rop2_map[11] = QPainter::RasterOp_NotSourceOrDestination;		/* DPno */
            rop2_map[12] = QPainter::CompositionMode_Source;			/* P */
            rop2_map[13] = QPainter::RasterOp_SourceOrNotDestination;		/* PDno */
            rop2_map[14] = QPainter::RasterOp_SourceOrDestination;			/* DPo */
            rop2_map[15] = QPainter::RasterOp_SetDestination;		/* 1 */
        }
        uint8 *
        translate_image(int width, int height, uint8 * data)
        {
            int size = width * height;
            uint32 *out = (uint32 *)malloc(size * sizeof(uint32));
            memset(out, 0, sizeof(uint32) * size);
            uint32 *p = out;
            uint32 *end = out + size;
            while (p < end){
                *(p++) = colmap[*(data++)];
            }
            return (uint8 *)out;
        }
        void DO_GLYPH(uint8 & font, unsigned char * ttext, int & idx,
                      unsigned char & flags, int & xyoffset,
                      int & x, int & y, int & mixmode, int & bgcolour, int & fgcolour);

    public:
        UI(MainWindow *, int, int, int);
        ~UI();
        void xwin_toggle_fullscreen(void);
        int ui_select(int rdp_socket);
        void ui_move_pointer(int x, int y);
        HRDPBITMAP ui_create_bitmap(int width, int height, uint8 * data);
        void ui_paint_bitmap(int x, int y, int cx, int cy, int width, int height, uint8 * data);
        void ui_destroy_bitmap(HRDPBITMAP bmp);
        HGLYPH ui_create_glyph(int width, int height, uint8 * data);
        void ui_destroy_glyph(HGLYPH glyph);
        HRDPCURSOR ui_create_cursor(unsigned int x, unsigned int y, int width, int height, uint8 * andmask,
                    uint8 * xormask);
        void ui_set_cursor(HRDPCURSOR cursor);
        void ui_destroy_cursor(HRDPCURSOR cursor);
        void ui_create_colourmap(COLOURMAP * colours);
        void ui_set_clip(int x, int y, int cx, int cy);
        void ui_reset_clip(void);
        void ui_bell(void);
        void ui_destblt(uint8 opcode, int x, int y, int cx, int cy);
        void ui_patblt(uint8 opcode, int x, int y, int cx, int cy, BRUSH * brush, int bgcolour,
                int fgcolour);
        void ui_screenblt(uint8 opcode, int x, int y, int cx, int cy, int srcx, int srcy);
        void ui_memblt(uint8 opcode, int x, int y, int cx, int cy, HRDPBITMAP src, int srcx, int srcy);
        void ui_triblt(uint8 opcode, int x, int y, int cx, int cy, HRDPBITMAP src, int srcx, int srcy,
                BRUSH * brush, int bgcolour, int fgcolour);
        void ui_line(uint8 opcode, int startx, int starty, int endx, int endy, PEN * pen);
        void ui_rect(int x, int y, int cx, int cy, int colour);
        void draw_glyph(int x, int y, QImage * glyphImage, int fgcolor);
        void ui_draw_text(uint8 font, uint8 flags, int mixmode, int x, int y, int clipx, int clipy,
                int clipcx, int clipcy, int boxx, int boxy, int boxcx, int boxcy, int bgcolour,
                int fgcolour, uint8 * text, uint8 length);
        void ui_desktop_save(uint32 offset, int x, int y, int cx, int cy);
        void ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy);
        void mwm_hide_decorations(void);
        BOOL get_key_state(unsigned int state, uint32 keysym);
};

#endif
