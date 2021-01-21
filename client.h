#ifndef CLIENT_H
#define CLIENT_H

#include <ctime>
#include <cerrno>
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
#include <QBitmap>
#include <third_party/openssl/rc4.h>
#include "xwin.h"
#include "tcp.h"
#include "constants.h"

class Client {
private:
    XWin_Ui *xwin_ui;
    TcpTool *tcptool;
    char title[32];
    char username[16];
    char hostname[16];
    char keymapname[16];
    int keylayout;        /* Defaults to US keyboard layout */
    int width;        /* If width or height are reset to zero, the geometry will
                        be fetched from _NET_WORKAREA */
    int height;
    int tcp_port_rdp;
    BOOL bitmap_compression;
    BOOL sendmotion;
    BOOL orders;
    BOOL encryption;
    BOOL desktop_save;
    BOOL fullscreen;
    BOOL grab_keyboard;
    BOOL hide_decorations;

    /* rdp.cpp */
    STREAM rdp_recv(uint8 *type);

    STREAM rdp_init(int maxlen);

    void rdp_send(STREAM s, uint8 pdu_type);

    STREAM rdp_init_data(int maxlen);

    void rdp_send_data(STREAM s, uint8 data_pdu_type);

    void rdp_send_logon_info(uint32 flags, char *domain,
                             char *user, char *password, char *program, char *directory);

    void rdp_send_control(uint16 action);

    void rdp_send_synchronise();

    void rdp_send_fonts(uint16 seq);

    void rdp_out_general_caps(STREAM s);

    void rdp_out_bitmap_caps(STREAM s);

    void rdp_out_order_caps(STREAM s);

    void rdp_out_bmpcache_caps(STREAM s);

    void rdp_out_control_caps(STREAM s);

    void rdp_out_activate_caps(STREAM s);

    void rdp_out_pointer_caps(STREAM s);

    void rdp_out_share_caps(STREAM s);

    void rdp_out_colcache_caps(STREAM s);

    void rdp_out_unknown_caps(STREAM s);

    void rdp_send_confirm_active();

    void process_demand_active(STREAM s);

    void process_pointer_pdu(STREAM s);

    void process_bitmap_updates(STREAM s);

    void process_palette(STREAM s);

    void process_update_pdu(STREAM s);

    void process_data_pdu(STREAM s);

    uint8 *next_packet;
    uint32 rdp_shareid;
    uint8 canned_caps[152];

    /* mcs.cpp */
    uint16 mcs_userid;

    BOOL ber_parse_header(STREAM s, int tagval, int *length);

    void ber_out_header(STREAM s, int tagval, int length);

    void ber_out_integer(STREAM s, int value);

    void mcs_out_domain_params(STREAM s, int max_channels,
                               int max_users, int max_tokens, int max_pdusize);

    BOOL mcs_parse_domain_params(STREAM s);

    void mcs_send_connect_initial(STREAM mcs_data);

    BOOL mcs_recv_connect_response(STREAM mcs_data);

    void mcs_send_edrq(void);

    void mcs_send_aurq(void);

    BOOL mcs_recv_aucf(uint16 *mcs_userid);

    void mcs_send_cjrq(uint16 chanid);

    BOOL mcs_recv_cjcf(void);

    STREAM mcs_init(int length);

    void mcs_send(STREAM s);

    STREAM mcs_recv(void);

    BOOL mcs_connect(char *server, STREAM mcs_data);

    void mcs_disconnect(void);

    /* bitmap.c */
    BOOL bitmap_decompress(unsigned char *output, int width, int height, unsigned char *input,
                           int size);

    /* ewmhints.c */
    int get_current_workarea(uint32 *x, uint32 *y, uint32 *width, uint32 *height);

    /* iso.c */
    void iso_send_msg(uint8 code);

    STREAM iso_recv_msg(uint8 *code);

    STREAM iso_init(int length);

    void iso_send(STREAM s);

    STREAM iso_recv(void);

    BOOL iso_connect(char *server);

    void iso_disconnect(void);

    /* licence.c */
    uint8 licence_key[16];
    uint8 licence_sign_key[16];
    BOOL licence_issued;

    void licence_generate_keys(uint8 *client_key, uint8 *server_key, uint8 *client_rsa);

    void licence_generate_hwid(uint8 *hwid);

    void licence_send_request(uint8 *client_random, uint8 *rsa_data, char *user, char *host);

    void licence_process_demand(STREAM s);

    void licence_send_authresp(uint8 *token, uint8 *crypt_hwid, uint8 *signature);

    BOOL licence_parse_authreq(STREAM s, uint8 **token, uint8 **signature);

    void licence_process_authreq(STREAM s);

    void licence_process_issue(STREAM s);

    void licence_process(STREAM s);

    /* orders.c */
    RDP_ORDER_STATE order_state;

    void rdp_in_present(STREAM s, uint32 *present, uint8 flags, int size);

    void rdp_in_coord(STREAM s, uint16 *coord, BOOL delta);

    void rdp_in_colour(STREAM s, uint8 *colour);

    BOOL rdp_parse_bounds(STREAM s, BOUNDS *bounds);

    BOOL rdp_parse_pen(STREAM s, PEN *pen, uint32 present);

    BOOL rdp_parse_brush(STREAM s, BRUSH *brush, uint32 present);

    void process_destblt(STREAM s, DESTBLT_ORDER *os, uint32 present, BOOL delta);

    void process_patblt(STREAM s, PATBLT_ORDER *os, uint32 present, BOOL delta);

    void process_screenblt(STREAM s, SCREENBLT_ORDER *os, uint32 present, BOOL delta);

    void process_line(STREAM s, LINE_ORDER *os, uint32 present, BOOL delta);

    void process_rect(STREAM s, RECT_ORDER *os, uint32 present, BOOL delta);

    void process_desksave(STREAM s, DESKSAVE_ORDER *os, uint32 present, BOOL delta);

    void process_memblt(STREAM s, MEMBLT_ORDER *os, uint32 present, BOOL delta);

    void process_triblt(STREAM s, TRIBLT_ORDER *os, uint32 present, BOOL delta);

    int parse_delta(uint8 *buffer, int *offset);

    void process_polyline(STREAM s, POLYLINE_ORDER *os, uint32 present, BOOL delta);

    void process_text2(STREAM s, TEXT2_ORDER *os, uint32 present, BOOL delta);

    void process_raw_bmpcache(STREAM s);

    void process_bmpcache(STREAM s);

    void process_colcache(STREAM s);

    void process_fontcache(STREAM s);

    void process_secondary_order(STREAM s);

    void process_orders(STREAM s);

    void reset_order_state();

    /* secure.c */
    int rc4_key_len;
    RC4_KEY rc4_decrypt_key;
    RC4_KEY rc4_encrypt_key;
    uint8 sec_sign_key[16];
    uint8 sec_decrypt_key[16];
    uint8 sec_encrypt_key[16];
    uint8 sec_decrypt_update_key[16];
    uint8 sec_encrypt_update_key[16];
    uint8 sec_crypted_random[SEC_MODULUS_SIZE];
    uint8 *pad_54;
    uint8 *pad_92;

    void buf_out_uint32(uint8 *buffer, uint32 value);

    void sec_hash_48(uint8 *out, uint8 *in, uint8 *salt1, uint8 *salt2, uint8 salt);

    void sec_hash_16(uint8 *out, uint8 *in, uint8 *salt1, uint8 *salt2);

    void sec_make_40bit(uint8 *key);

    void sec_generate_keys(uint8 *client_key, uint8 *server_key, int rc4_key_size);

    void sec_sign(uint8 *signature, int siglen, uint8 *session_key, int keylen, uint8 *data, int datalen);

    void sec_update(uint8 *key, uint8 *update_key);

    void sec_encrypt(uint8 *data, int length);

    void sec_decrypt(uint8 *data, int length);

    void reverse(uint8 *p, int len);

    void sec_rsa_encrypt(uint8 *out, uint8 *in, int len, uint8 *modulus, uint8 *exponent);

    STREAM sec_init(uint32 flags, int maxlen);

    void sec_send(STREAM s, uint32 flags);

    void sec_establish_key();

    void sec_out_mcs_data(STREAM s);

    BOOL sec_parse_public_key(STREAM s, uint8 **modulus, uint8 **exponent);

    BOOL sec_parse_crypt_info(STREAM s, uint32 *rc4_key_size,
                              uint8 **server_random, uint8 **modulus, uint8 **exponent);

    void generate_random(uint8 *random);

    void sec_process_crypt_info(STREAM s);

    void sec_process_mcs_data(STREAM s);

    STREAM sec_recv();

    BOOL sec_connect(char *server);

    void sec_disconnect();

    /* xkeymap.c */
    BOOL keymap_loaded;
    key_translation keymap[KEYMAP_SIZE];
    int min_keycode;
    uint16 remote_modifier_state;

    void add_to_keymap(char *keyname, uint8 scancode, uint16 modifiers, char *mapname);

    BOOL xkeymap_read(char *mapname);

    void xkeymap_init();

    key_translation xkeymap_translate_key(uint32 keysym, unsigned int keycode, unsigned int state);

    char *get_ksname(uint32 keysym);

    void ensure_remote_modifiers(uint32 ev_time, key_translation tr);

    void reset_modifier_keys(unsigned int state);

    void update_modifier_state(uint8 scancode, BOOL pressed);

public:
    Client(XWin_Ui *ui, TcpTool *tool);

    void rdp_main_loop();

    void rdp_out_unistr(stream *s, char *string, int len);

    BOOL rdp_connect(char *server, uint32 flags, char *domain,
                     char *password, char *command, char *directory);

    void rdp_send_scancode(uint32 time, uint16 flags, uint8 scancode);

    BOOL handle_special_keys(int key, uint32 ev_time, BOOL pressed);

    uint16 xkeymap_translate_button(Qt::MouseButton button);

    void rdp_send_input(uint32 time, uint16 message_type,
                        uint16 device_flags, uint16 param1, uint16 param2);

    void rdp_disconnect();
};

#endif
