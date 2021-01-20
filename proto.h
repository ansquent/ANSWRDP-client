#ifndef PROTO_H
#define PROTO_H
#include <types.h>
#include <parse.h>
#include <mainwindow.h>
#include <QColor>
/* bitmap.c */
BOOL bitmap_decompress(unsigned char *output, int width, int height, unsigned char *input,
		       int size);
/* cache.c */
HRDPBITMAP cache_get_bitmap(uint8 cache_id, uint16 cache_idx);
void cache_put_bitmap(uint8 cache_id, uint16 cache_idx, HRDPBITMAP bitmap);
FONTGLYPH *cache_get_font(uint8 font, uint16 character);
void cache_put_font(uint8 font, uint16 character, uint16 offset, uint16 baseline, uint16 width,
		    uint16 height, HGLYPH pixmap);
DATABLOB *cache_get_text(uint8 cache_id);
void cache_put_text(uint8 cache_id, void *data, int length);
uint8 *cache_get_desktop(uint32 offset, int cx, int cy, int bytes_per_pixel);
void cache_put_desktop(uint32 offset, int cx, int cy, int scanline, int bytes_per_pixel,
		       uint8 * data);
HRDPCURSOR cache_get_cursor(uint16 cache_idx);
void cache_put_cursor(uint16 cache_idx, HRDPCURSOR cursor);
/* ewmhints.c */
int get_current_workarea(uint32 * x, uint32 * y, uint32 * width, uint32 * height);
/* iso.c */
STREAM iso_init(int length);
void iso_send(STREAM s);
STREAM iso_recv(void);
BOOL iso_connect(char *server);
void iso_disconnect(void);
/* licence.c */
void licence_process(STREAM s);
/* mcs.c */
STREAM mcs_init(int length);
void mcs_send(STREAM s);
STREAM mcs_recv(void);
BOOL mcs_connect(char *server, STREAM mcs_data);
void mcs_disconnect(void);
/* orders.c */
void process_orders(STREAM s);
void reset_order_state(void);
/* rdesktop.c */
int tmain(int argc, char *argv[]);
void generate_random(uint8 * random);
void *xmalloc(int size);
void *xrealloc(void *oldmem, int size);
void xfree(void *mem);
void info(const char *format, ...);
void info(STREAM s);
void error(const char *format, ...);
void warning(const char *format, ...);
void unimpl(const char *format, ...);
void hexdump(unsigned char *p, unsigned int len);
/* rdp.c */
void rdp_out_unistr(STREAM s, char *string, int len);
void rdp_send_input(uint32 time, uint16 message_type, uint16 device_flags, uint16 param1,
		    uint16 param2);
void rdp_main_loop(void);
BOOL rdp_connect(char *server, uint32 flags, char *domain, char *password, char *command,
		 char *directory);
void rdp_disconnect(void);
/* secure.c */
void sec_hash_48(uint8 * out, uint8 * in, uint8 * salt1, uint8 * salt2, uint8 salt);
void sec_hash_16(uint8 * out, uint8 * in, uint8 * salt1, uint8 * salt2);
void buf_out_uint32(uint8 * buffer, uint32 value);
void sec_sign(uint8 * signature, int siglen, uint8 * session_key, int keylen, uint8 * data,
	      int datalen);
STREAM sec_init(uint32 flags, int maxlen);
void sec_send(STREAM s, uint32 flags);
STREAM sec_recv(void);
BOOL sec_connect(char *server);
void sec_disconnect(void);
/* tcp.c */
STREAM tcp_init(int maxlen);
void tcp_send(STREAM s);
STREAM tcp_recv(unsigned length);
BOOL tcp_connect(char *server);
void tcp_disconnect(void);
/* xkeymap.c */
void xkeymap_init(void);
BOOL handle_special_keys(uint32 keysym, unsigned int state, uint32 ev_time, BOOL pressed);
key_translation xkeymap_translate_key(uint32 keysym, unsigned int keycode, unsigned int state);
uint16 xkeymap_translate_button(unsigned int button);
char *get_ksname(uint32 keysym);
void ensure_remote_modifiers(uint32 ev_time, key_translation tr);
void reset_modifier_keys(unsigned int state);
void rdp_send_scancode(uint32 time, uint16 flags, uint8 scancode);
#endif
