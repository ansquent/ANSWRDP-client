#include "mainwindow.h"
#include <QPaintEvent>
#include "constants.h"
#include "xwin.h"
#include "client.h"
#include "tcp.h"
#include "third_party/openssl/include/rc4.h"
#include "third_party/openssl/include/md5.h"
#include "third_party/openssl/include/sha.h"
#include "third_party/openssl/include/bn.h"
#include "third_party/x11/keysymdef.h"

void info(const char *format, ...);

#ifdef _WIN32
#ifndef _WIN64
#pragma comment(lib,"../rdesktop-wrap/third_party/openssl/lib/libcrypto32MT.lib")
#pragma comment(lib,"../rdesktop-wrap/third_party/openssl/lib/libssl32MT.lib")
#else
#pragma comment(lib,"../rdesktop-wrap/third_party/openssl/lib/libcrypto64MT.lib")
#pragma comment(lib,"../rdesktop-wrap/third_party/openssl/lib/libssl64MT.lib")
#endif
#endif

Client::Client(XWin_Ui *ui, TcpTool *tool, char *hostname, char *username) : canned_caps{
        0x01, 0x00, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x08, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x0E, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x34, 0x00, 0xFE,
        0x00, 0x04, 0x00, 0xFE, 0x00, 0x04, 0x00, 0xFE, 0x00, 0x08, 0x00,
        0xFE, 0x00, 0x08, 0x00, 0xFE,
        0x00, 0x10, 0x00, 0xFE, 0x00, 0x20, 0x00, 0xFE, 0x00, 0x40, 0x00,
        0xFE, 0x00, 0x80, 0x00, 0xFE,
        0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x01,
        0x02, 0x00, 0x00, 0x00
}, licence_issued(false), keylayout(0x409),
                                                                             bitmap_compression(true), encryption(true),
                                                                             desktop_save(true),
                                                                             remote_modifier_state(0), xwin_ui(ui),
                                                                             tcptool(tool) {
    pad_54 = new uint8[40];
    for (int i = 0; i < 40; ++i) {
        pad_54[i] = 54;
    }
    pad_92 = new uint8[48];
    for (int i = 0; i < 48; ++i) {
        pad_92[i] = 92;
    }
    this->hostname = hostname;
    this->username = username;
    this->width = xwin_ui->get_width();
    this->height = xwin_ui->get_height();
    min_keycode = 8;
}

void Client::rdp_main_loop() {
    uint8 type;
    STREAM s;

    if ((s = rdp_recv(&type)) != nullptr) {
        switch (type) {
            case RDP_PDU_DEMAND_ACTIVE:
                process_demand_active(s);
                break;

            case RDP_PDU_DEACTIVATE:
                break;

            case RDP_PDU_DATA:
                process_data_pdu(s);
                break;

            case 0:
                break;

            default:
                throw not_implemented_error{};
        }
    }
}

STREAM Client::rdp_recv(uint8 *type) {
    static STREAM rdp_s;
    uint16 length, pdu_type;

    if ((rdp_s == nullptr) || (next_packet >= rdp_s->end)) {
        rdp_s = sec_recv();
        if (rdp_s == nullptr)
            return nullptr;

        next_packet = rdp_s->p;
    } else {
        rdp_s->p = next_packet;
    }

    in_uint16_le(rdp_s, length);
    /* 32k packets are really 8, keepalive fix */
    if (length == 0x8000) {
        next_packet += 8;
        *type = 0;
        return rdp_s;
    }
    in_uint16_le(rdp_s, pdu_type);
    in_uint8s(rdp_s, 2);    /* userid */
    *type = pdu_type & 0xf;

#if WITH_DEBUG
    hexdump(next_packet, length);
#endif /*  */

    next_packet += length;
    return rdp_s;
}

/* Initialise an RDP packet */
STREAM Client::rdp_init(int maxlen) {
    STREAM s;

    s = sec_init(encryption ? SEC_ENCRYPT : 0, maxlen + 6);
    s_push_layer(s, rdp_hdr, 6);

    return s;
}

/* Send an RDP packet */
void Client::rdp_send(STREAM s, uint8 pdu_type) {
    uint16 length;

    s_pop_layer(s, rdp_hdr);
    length = s->end - s->p;

    out_uint16_le(s, length);
    out_uint16_le(s, (pdu_type | 0x10));    /* Version 1 */
    out_uint16_le(s, (mcs_userid + 1001));

    sec_send(s, encryption ? SEC_ENCRYPT : 0);
}

/* Initialise an RDP data packet */
STREAM Client::rdp_init_data(int maxlen) {
    STREAM s;

    s = sec_init(encryption ? SEC_ENCRYPT : 0, maxlen + 18);
    s_push_layer(s, rdp_hdr, 18);

    return s;
}

/* Send an RDP data packet */
void Client::rdp_send_data(STREAM s, uint8 data_pdu_type) {
    uint16 length;

    s_pop_layer(s, rdp_hdr);
    length = s->end - s->p;

    out_uint16_le(s, length);
    out_uint16_le(s, (RDP_PDU_DATA | 0x10));
    out_uint16_le(s, (mcs_userid + 1001));

    out_uint32_le(s, rdp_shareid);
    out_uint8(s, 0);    /* pad */
    out_uint8(s, 1);    /* streamid */
    out_uint16_le(s, (length - 14));
    out_uint8(s, data_pdu_type);
    out_uint8(s, 0);    /* compress_type */
    out_uint16(s, 0);    /* compress_len */

    sec_send(s, encryption ? SEC_ENCRYPT : 0);
}

/* Output a string in Unicode */
void Client::rdp_out_unistr(STREAM s, const char *string, int len) {
    int i = 0, j = 0;

    len += 2;

    while (i < len) {
        s->p[i++] = string[j++];
        s->p[i++] = 0;
    }

    s->p += len;
}

/* Parse a logon info packet */
void Client::rdp_send_logon_info(uint32 flags, char *domain, char *user,
                                 char *password, char *program, char *directory) {
    int len_domain = 2 * strlen(domain);
    int len_user = 2 * strlen(user);
    int len_password = 2 * strlen(password);
    int len_program = 2 * strlen(program);
    int len_directory = 2 * strlen(directory);
    uint32 sec_flags = encryption ? (SEC_LOGON_INFO | SEC_ENCRYPT) : SEC_LOGON_INFO;
    STREAM s;

    s = sec_init(sec_flags, 18 + len_domain + len_user + len_password
                            + len_program + len_directory + 10);

    out_uint32(s, 0);
    out_uint32_le(s, flags);
    out_uint16_le(s, len_domain);
    out_uint16_le(s, len_user);
    out_uint16_le(s, len_password);
    out_uint16_le(s, len_program);
    out_uint16_le(s, len_directory);
    rdp_out_unistr(s, domain, len_domain);
    rdp_out_unistr(s, user, len_user);
    rdp_out_unistr(s, password, len_password);
    rdp_out_unistr(s, program, len_program);
    rdp_out_unistr(s, directory, len_directory);

    s_mark_end(s);
    sec_send(s, sec_flags);
}

/* Send a control PDU */
void Client::rdp_send_control(uint16 action) {
    STREAM s;

    s = rdp_init_data(8);

    out_uint16_le(s, action);
    out_uint16(s, 0);    /* userid */
    out_uint32(s, 0);    /* control id */

    s_mark_end(s);
    rdp_send_data(s, RDP_DATA_PDU_CONTROL);
}

/* Send a synchronisation PDU */
void Client::rdp_send_synchronise() {
    STREAM s;

    s = rdp_init_data(4);

    out_uint16_le(s, 1);    /* type */
    out_uint16_le(s, 1002);

    s_mark_end(s);
    rdp_send_data(s, RDP_DATA_PDU_SYNCHRONISE);
}

/* Send a single input event */
void Client::rdp_send_input(uint32 time, uint16 message_type, uint16 device_flags, uint16 param1, uint16 param2) {
    STREAM s;

    s = rdp_init_data(16);

    out_uint16_le(s, 1);    /* number of events */
    out_uint16(s, 0);    /* pad */

    out_uint32_le(s, time);
    out_uint16_le(s, message_type);
    out_uint16_le(s, device_flags);
    out_uint16_le(s, param1);
    out_uint16_le(s, param2);

    s_mark_end(s);
    rdp_send_data(s, RDP_DATA_PDU_INPUT);
}

/* Send an (empty) font information PDU */
void Client::rdp_send_fonts(uint16 seq) {
    STREAM s;

    s = rdp_init_data(8);

    out_uint16(s, 0);    /* number of fonts */
    out_uint16_le(s, 0x3e);    /* unknown */
    out_uint16_le(s, seq);    /* unknown */
    out_uint16_le(s, 0x32);    /* entry size */

    s_mark_end(s);
    rdp_send_data(s, RDP_DATA_PDU_FONT2);
}

/* Output general capability set */
void Client::rdp_out_general_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_GENERAL);
    out_uint16_le(s, RDP_CAPLEN_GENERAL);

    out_uint16_le(s, 1);    /* OS major type */
    out_uint16_le(s, 3);    /* OS minor type */
    out_uint16_le(s, 0x200);    /* Protocol version */
    out_uint16(s, 0);    /* Pad */
    out_uint16(s, 0);    /* Compression types */
    out_uint16(s, 0);    /* Pad */
    out_uint16(s, 0);    /* Update capability */
    out_uint16(s, 0);    /* Remote unshare capability */
    out_uint16(s, 0);    /* Compression level */
    out_uint16(s, 0);    /* Pad */
}

/* Output bitmap capability set */
void Client::rdp_out_bitmap_caps(STREAM s) const {
    out_uint16_le(s, RDP_CAPSET_BITMAP);
    out_uint16_le(s, RDP_CAPLEN_BITMAP);

    out_uint16_le(s, 8);    /* Preferred BPP */
    out_uint16_le(s, 1);    /* Receive 1 BPP */
    out_uint16_le(s, 1);    /* Receive 4 BPP */
    out_uint16_le(s, 1);    /* Receive 8 BPP */
    out_uint16_le(s, 800);    /* Desktop width */
    out_uint16_le(s, 600);    /* Desktop height */
    out_uint16(s, 0);    /* Pad */
    out_uint16(s, 0);    /* Allow resize */
    out_uint16_le(s, bitmap_compression ? 1 : 0);    /* Support compression */
    out_uint16(s, 0);    /* Unknown */
    out_uint16_le(s, 1);    /* Unknown */
    out_uint16(s, 0);    /* Pad */
}

/* Output order capability set */
void Client::rdp_out_order_caps(STREAM s) const {
    uint8 order_caps[32];


    memset(order_caps, 0, 32);
    order_caps[0] = 1;    /* dest blt */
    order_caps[1] = 1;    /* pat blt */
    order_caps[2] = 1;    /* screen blt */
    order_caps[3] = 1;    /* required for memblt? */
    order_caps[8] = 1;    /* line */
    order_caps[9] = 1;    /* line */
    order_caps[10] = 1;    /* rect */
    order_caps[11] = (desktop_save == false ? 0 : 1);    /* desksave */
    order_caps[13] = 1;    /* memblt */
    order_caps[14] = 1;    /* triblt */
    order_caps[22] = 1;    /* polyline */
    order_caps[27] = 1;    /* text2 */
    out_uint16_le(s, RDP_CAPSET_ORDER);
    out_uint16_le(s, RDP_CAPLEN_ORDER);

    out_uint8s(s, 20);    /* Terminal desc, pad */
    out_uint16_le(s, 1);    /* Cache X granularity */
    out_uint16_le(s, 20);    /* Cache Y granularity */
    out_uint16(s, 0);    /* Pad */
    out_uint16_le(s, 1);    /* Max order level */
    out_uint16_le(s, 0x147);    /* Number of fonts */
    out_uint16_le(s, 0x2a);    /* Capability flags */
    out_uint8p(s, order_caps, 32);    /* Orders supported */
    out_uint16_le(s, 0x6a1);    /* Text capability flags */
    out_uint8s(s, 6);    /* Pad */
    out_uint32_le(s, desktop_save == false ? 0 : 0x38400);    /* Desktop cache size */
    out_uint32(s, 0);    /* Unknown */
    out_uint32_le(s, 0x4e4);    /* Unknown */
}

/* Output bitmap cache capability set */
void Client::rdp_out_bmpcache_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_BMPCACHE);
    out_uint16_le(s, RDP_CAPLEN_BMPCACHE);

    out_uint8s(s, 24);    /* unused */
    out_uint16_le(s, 0x258);    /* entries */
    out_uint16_le(s, 0x100);    /* max cell size */
    out_uint16_le(s, 0x12c);    /* entries */
    out_uint16_le(s, 0x400);    /* max cell size */
    out_uint16_le(s, 0x106);    /* entries */
    out_uint16_le(s, 0x1000);    /* max cell size */
}

/* Output control capability set */
void Client::rdp_out_control_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_CONTROL);
    out_uint16_le(s, RDP_CAPLEN_CONTROL);

    out_uint16(s, 0);    /* Control capabilities */
    out_uint16(s, 0);    /* Remote detach */
    out_uint16_le(s, 2);    /* Control interest */
    out_uint16_le(s, 2);    /* Detach interest */
}

/* Output activation capability set */
void Client::rdp_out_activate_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_ACTIVATE);
    out_uint16_le(s, RDP_CAPLEN_ACTIVATE);

    out_uint16(s, 0);    /* Help key */
    out_uint16(s, 0);    /* Help index key */
    out_uint16(s, 0);    /* Extended help key */
    out_uint16(s, 0);    /* Window activate */
}

/* Output pointer capability set */
void Client::rdp_out_pointer_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_POINTER);
    out_uint16_le(s, RDP_CAPLEN_POINTER);

    out_uint16(s, 0);    /* Color pointer */
    out_uint16_le(s, 20);    /* Cache size */
}

/* Output share capability set */
void Client::rdp_out_share_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_SHARE);
    out_uint16_le(s, RDP_CAPLEN_SHARE);

    out_uint16(s, 0);    /* userid */
    out_uint16(s, 0);    /* pad */
}

/* Output color cache capability set */
void Client::rdp_out_colcache_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_COLCACHE);
    out_uint16_le(s, RDP_CAPLEN_COLCACHE);

    out_uint16_le(s, 6);    /* cache size */
    out_uint16(s, 0);    /* pad */
}

/* Output unknown capability set */
void Client::rdp_out_unknown_caps(STREAM s) {
    out_uint16_le(s, RDP_CAPSET_UNKNOWN);
    out_uint16_le(s, 0x58);

    out_uint8p(s, canned_caps, RDP_CAPLEN_UNKNOWN - 4);
}

/* Send a confirm active PDU */
void Client::rdp_send_confirm_active() {
    STREAM s;
    uint16 caplen =
            RDP_CAPLEN_GENERAL + RDP_CAPLEN_BITMAP + RDP_CAPLEN_ORDER +
            RDP_CAPLEN_BMPCACHE + RDP_CAPLEN_COLCACHE +
            RDP_CAPLEN_ACTIVATE + RDP_CAPLEN_CONTROL +
            RDP_CAPLEN_POINTER + RDP_CAPLEN_SHARE + RDP_CAPLEN_UNKNOWN + 4 /* w2k fix, why? */ ;

    s = rdp_init(14 + caplen + sizeof(RDP_SOURCE));

    out_uint32_le(s, rdp_shareid);
    out_uint16_le(s, 0x3ea);    /* userid */
    out_uint16_le(s, sizeof(RDP_SOURCE));
    out_uint16_le(s, caplen);

    out_uint8p(s, RDP_SOURCE, sizeof(RDP_SOURCE));
    out_uint16_le(s, 0xd);    /* num_caps */
    out_uint8s(s, 2);    /* pad */

    rdp_out_general_caps(s);
    rdp_out_bitmap_caps(s);
    rdp_out_order_caps(s);
    rdp_out_bmpcache_caps(s);
    rdp_out_colcache_caps(s);
    rdp_out_activate_caps(s);
    rdp_out_control_caps(s);
    rdp_out_pointer_caps(s);
    rdp_out_share_caps(s);
    rdp_out_unknown_caps(s);

    s_mark_end(s);
    rdp_send(s, RDP_PDU_CONFIRM_ACTIVE);
}

/* Respond to a demand active PDU */
void Client::process_demand_active(STREAM s) {
    uint8 type;

    in_uint32_le(s, rdp_shareid);

    rdp_send_confirm_active();
    rdp_send_synchronise();
    rdp_send_control(RDP_CTL_COOPERATE);
    rdp_send_control(RDP_CTL_REQUEST_CONTROL);
    rdp_recv(&type);    /* RDP_PDU_SYNCHRONIZE */
    rdp_recv(&type);    /* RDP_CTL_COOPERATE */
    rdp_recv(&type);    /* RDP_CTL_GRANT_CONTROL */
    rdp_send_input(0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);
    rdp_send_fonts(1);
    rdp_send_fonts(2);
    rdp_recv(&type);    /* RDP_PDU_UNKNOWN 0x28 */
    reset_order_state();
}

/* Process a pointer PDU */
void Client::process_pointer_pdu(STREAM s) {
    uint16 message_type;
    uint16 x, y, width, height, cache_idx, masklen, datalen;
    uint8 *mask, *data;
    HRDPCURSOR cursor;

    in_uint16_le(s, message_type);
    in_uint8s(s, 2);    /* pad */

    switch (message_type) {
        case RDP_POINTER_MOVE: in_uint16_le(s, x);
            in_uint16_le(s, y);
            if (s_check(s))
                xwin_ui->ui_move_pointer(x, y);
            break;

        case RDP_POINTER_COLOR: in_uint16_le(s, cache_idx);
            in_uint16_le(s, x);
            in_uint16_le(s, y);
            in_uint16_le(s, width);
            in_uint16_le(s, height);
            in_uint16_le(s, masklen);
            in_uint16_le(s, datalen);
            in_uint8p(s, data, datalen);
            in_uint8p(s, mask, masklen);
            cursor = XWin_Ui::ui_create_cursor(x, y, width, height, mask, data);
            xwin_ui->ui_set_cursor(cursor);
            xwin_ui->cache_put_cursor(cache_idx, cursor);
            break;

        case RDP_POINTER_CACHED: in_uint16_le(s, cache_idx);
            xwin_ui->ui_set_cursor(xwin_ui->cache_get_cursor(cache_idx));
            break;

        default:
            info("Pointer message 0x%x\n", message_type);
    }
}

/* Process bitmap updates */
void Client::process_bitmap_updates(STREAM s) {
    uint16 num_updates;
    uint16 left, top, right, bottom, width, height;
    uint16 cx, cy, bpp, compress, bufsize, size;
    uint8 *data, *bmpdata;
    int i;

    in_uint16_le(s, num_updates);

    for (i = 0; i < num_updates; i++) {
        in_uint16_le(s, left);
        in_uint16_le(s, top);
        in_uint16_le(s, right);
        in_uint16_le(s, bottom);
        in_uint16_le(s, width);
        in_uint16_le(s, height);
        in_uint16_le(s, bpp);
        in_uint16_le(s, compress);
        in_uint16_le(s, bufsize);

        cx = right - left + 1;
        cy = bottom - top + 1;

        if (!compress) {
            int y;
            bmpdata = new uint8[width * height];
            for (y = 0; y < height; y++) {
                in_uint8a(s, &bmpdata[(height - y - 1) * width], width);
            }
            xwin_ui->ui_paint_bitmap(left, top, cx, cy, width, height, bmpdata);
            delete[]bmpdata;
            continue;
        }

        in_uint8s(s, 2);    /* pad */
        in_uint16_le(s, size);
        in_uint8s(s, 4);    /* line_size, final_size */
        in_uint8p(s, data, size);

        bmpdata = new uint8[width * height];
        if (bitmap_decompress(bmpdata, width, height, data, size)) {
            xwin_ui->ui_paint_bitmap(left, top, cx, cy, width, height, bmpdata);
        }

        delete[]bmpdata;
    }
}

/* Process a palette update */
void Client::process_palette(STREAM s) {
    COLORENTRY *entry;
    COLORMAP map;
    HCOLORMAP hmap;
    int i;

    in_uint8s(s, 2);    /* pad */
    in_uint16_le(s, map.ncolors);
    in_uint8s(s, 2);    /* pad */

    map.colors = new COLORENTRY[3 * map.ncolors];

    for (i = 0; i < map.ncolors; i++) {
        entry = &map.colors[i];
        in_uint8(s, entry->red);
        in_uint8(s, entry->green);
        in_uint8(s, entry->blue);
    }

    xwin_ui->ui_create_colormap(&map);

    delete[]map.colors;
}

/* Process an update PDU */
void Client::process_update_pdu(STREAM s) {
    uint16 update_type;

    in_uint16_le(s, update_type);

    switch (update_type) {
        case RDP_UPDATE_ORDERS:
            process_orders(s);
            break;

        case RDP_UPDATE_BITMAP:
            process_bitmap_updates(s);
            break;

        case RDP_UPDATE_PALETTE:
            process_palette(s);
            break;

        case RDP_UPDATE_SYNCHRONIZE:
            break;

        default:
            throw not_implemented_error{};
    }

}

/* Process data PDU */
void Client::process_data_pdu(STREAM s) {
    uint8 data_pdu_type;

    in_uint8s(s, 8);    /* shareid, pad, streamid, length */
    in_uint8(s, data_pdu_type);
    in_uint8s(s, 3);    /* compress_type, compress_len */

    switch (data_pdu_type) {
        case RDP_DATA_PDU_UPDATE:
            process_update_pdu(s);
            break;

        case RDP_DATA_PDU_POINTER:
            process_pointer_pdu(s);
            break;

        case RDP_DATA_PDU_BELL:
            xwin_ui->ui_bell();
            break;

        case RDP_DATA_PDU_LOGON:
            /* User logged on */
            break;

        default:
            throw not_implemented_error{};
    }
}

/* Establish a connection up to the RDP layer */
BOOL Client::rdp_connect(char *server, uint32 flags, char *domain, char *password, char *command,
                         char *directory) {
    if (!sec_connect(server))
        return false;

    rdp_send_logon_info(flags, domain, (char *) username, password, command, directory);
    return true;
}

/* Disconnect from the RDP layer */
void Client::rdp_disconnect() {
    sec_disconnect();
}

/* Parse an ASN.1 BER header */
BOOL Client::ber_parse_header(STREAM s, int tagval, int *length) {
    int tag, len;

    if (tagval > 0xff) {
        in_uint16_be(s, tag);
    } else {
        in_uint8(s, tag)
    }

    if (tag != tagval) {
        info("expected tag %d, got %d\n", tagval, tag);
        return false;
    }

    in_uint8(s, len);

    if (len & 0x80) {
        len &= ~0x80;
        *length = 0;
        while (len--)
            next_be(s, *length);
    } else
        *length = len;

    return s_check(s);
}

/* Output an ASN.1 BER header */
void Client::ber_out_header(STREAM s, int tagval, int length) {
    if (tagval > 0xff) {
        out_uint16_be(s, tagval);
    } else {
        out_uint8(s, tagval);
    }

    if (length >= 0x80) {
        out_uint8(s, 0x82);
        out_uint16_be(s, length);
    } else
        out_uint8(s, length);
}

/* Output an ASN.1 BER integer */
void Client::ber_out_integer(STREAM s, int value) {
    ber_out_header(s, BER_TAG_INTEGER, 2);
    out_uint16_be(s, value);
}

/* Output a DOMAIN_PARAMS structure (ASN.1 BER) */
void Client::mcs_out_domain_params(STREAM s, int max_channels, int max_users, int max_tokens, int max_pdusize) {
    ber_out_header(s, MCS_TAG_DOMAIN_PARAMS, 32);
    ber_out_integer(s, max_channels);
    ber_out_integer(s, max_users);
    ber_out_integer(s, max_tokens);
    ber_out_integer(s, 1);    /* num_priorities */
    ber_out_integer(s, 0);    /* min_throughput */
    ber_out_integer(s, 1);    /* max_height */
    ber_out_integer(s, max_pdusize);
    ber_out_integer(s, 2);    /* ver_protocol */
}

/* Parse a DOMAIN_PARAMS structure (ASN.1 BER) */
BOOL Client::mcs_parse_domain_params(STREAM s) {
    int length;

    ber_parse_header(s, MCS_TAG_DOMAIN_PARAMS, &length);
    in_uint8s(s, length);

    return s_check(s);
}

/* Send an MCS_CONNECT_INITIAL message (ASN.1 BER) */
void Client::mcs_send_connect_initial(STREAM mcs_data) {
    int datalen = mcs_data->end - mcs_data->data;
    int length = 7 + 3 * 34 + 4 + datalen;
    STREAM s;

    s = iso_init(length + 5);

    ber_out_header(s, MCS_CONNECT_INITIAL, length);
    ber_out_header(s, BER_TAG_OCTET_STRING, 0);    /* calling domain */
    ber_out_header(s, BER_TAG_OCTET_STRING, 0);    /* called domain */

    ber_out_header(s, BER_TAG_BOOLEAN, 1);
    out_uint8(s, 0xff);    /* upward flag */

    mcs_out_domain_params(s, 2, 2, 0, 0xffff);    /* target params */
    mcs_out_domain_params(s, 1, 1, 1, 0x420);    /* min params */
    mcs_out_domain_params(s, 0xffff, 0xfc17, 0xffff, 0xffff);    /* max params */

    ber_out_header(s, BER_TAG_OCTET_STRING, datalen);
    out_uint8p(s, mcs_data->data, datalen);

    s_mark_end(s);
    iso_send(s);
}

/* Expect a MCS_CONNECT_RESPONSE message (ASN.1 BER) */
BOOL Client::mcs_recv_connect_response(STREAM mcs_data) {
    uint8 result;
    int length;
    STREAM s;

    s = iso_recv();
    if (s == nullptr)
        return false;

    ber_parse_header(s, MCS_CONNECT_RESPONSE, &length);

    ber_parse_header(s, BER_TAG_RESULT, &length);
    in_uint8(s, result);
    if (result != 0) {
        info("MCS connect: %d\n", result);
        return false;
    }

    ber_parse_header(s, BER_TAG_INTEGER, &length);
    in_uint8s(s, length);    /* connect id */
    mcs_parse_domain_params(s);

    ber_parse_header(s, BER_TAG_OCTET_STRING, &length);
    if (length > mcs_data->size) {
        info("MCS data length %d\n", length);
        length = mcs_data->size;
    }

    in_uint8a(s, mcs_data->data, length);
    mcs_data->p = mcs_data->data;
    mcs_data->end = mcs_data->data + length;

    return s_check_end(s);
}

/* Send an EDrq message (ASN.1 PER) */
void Client::mcs_send_edrq() {
    STREAM s;

    s = iso_init(5);

    out_uint8(s, (MCS_EDRQ << 2));
    out_uint16_be(s, 1);    /* height */
    out_uint16_be(s, 1);    /* interval */

    s_mark_end(s);
    iso_send(s);
}

/* Send an AUrq message (ASN.1 PER) */
void Client::mcs_send_aurq() {
    STREAM s;

    s = iso_init(1);

    out_uint8(s, (MCS_AURQ << 2));

    s_mark_end(s);
    iso_send(s);
}

/* Expect a AUcf message (ASN.1 PER) */
BOOL Client::mcs_recv_aucf(uint16 *mcs_userid) {
    uint8 opcode, result;
    STREAM s;

    s = iso_recv();
    if (s == nullptr)
        return false;

    in_uint8(s, opcode);
    if ((opcode >> 2) != MCS_AUCF) {
        info("expected AUcf, got %d\n", opcode);
        return false;
    }

    in_uint8(s, result);
    if (result != 0) {
        info("AUrq: %d\n", result);
        return false;
    }

    if (opcode & 2) in_uint16_be(s, *mcs_userid);

    return s_check_end(s);
}

/* Send a CJrq message (ASN.1 PER) */
void Client::mcs_send_cjrq(uint16 chanid) {
    STREAM s;

    s = iso_init(5);

    out_uint8(s, (MCS_CJRQ << 2));
    out_uint16_be(s, mcs_userid);
    out_uint16_be(s, chanid);

    s_mark_end(s);
    iso_send(s);
}

/* Expect a CJcf message (ASN.1 PER) */
BOOL Client::mcs_recv_cjcf() {
    uint8 opcode, result;
    STREAM s;

    s = iso_recv();
    if (s == nullptr)
        return false;

    in_uint8(s, opcode);
    if ((opcode >> 2) != MCS_CJCF) {
        info("expected CJcf, got %d\n", opcode);
        return false;
    }

    in_uint8(s, result);
    if (result != 0) {
        info("CJrq: %d\n", result);
        return false;
    }

    in_uint8s(s, 4);    /* mcs_userid, req_chanid */
    if (opcode & 2)
        in_uint8s(s, 2);    /* join_chanid */

    return s_check_end(s);
}

/* Initialise an MCS transport data packet */
STREAM Client::mcs_init(int length) {
    STREAM s;

    s = iso_init(length + 8);
    s_push_layer(s, mcs_hdr, 8);

    return s;
}

/* Send an MCS transport data packet */
void Client::mcs_send(STREAM s) {
    uint16 length;

    s_pop_layer(s, mcs_hdr);
    length = s->end - s->p - 8;
    length |= 0x8000;

    out_uint8(s, (MCS_SDRQ << 2));
    out_uint16_be(s, mcs_userid);
    out_uint16_be(s, MCS_GLOBAL_CHANNEL);
    out_uint8(s, 0x70);    /* flags */
    out_uint16_be(s, length);

    iso_send(s);
}

/* Receive an MCS transport data packet */
STREAM Client::mcs_recv() {
    uint8 opcode, appid, length;
    STREAM s;

    s = iso_recv();
    if (s == nullptr)
        return nullptr;

    in_uint8(s, opcode);
    appid = opcode >> 2;
    if (appid != MCS_SDIN) {
        if (appid != MCS_DPUM) {
            info("expected data, got %d\n", opcode);
        }
        return nullptr;
    }

    in_uint8s(s, 5);    /* userid, chanid, flags */
    in_uint8(s, length);
    if (length & 0x80)
        in_uint8s(s, 1);    /* second byte of length */

    return s;
}

/* Establish a connection up to the MCS layer */
BOOL Client::mcs_connect(char *server, STREAM mcs_data) {
    if (!iso_connect(server))
        return false;

    mcs_send_connect_initial(mcs_data);
    if (!mcs_recv_connect_response(mcs_data))
        goto error;

    mcs_send_edrq();

    mcs_send_aurq();
    if (!mcs_recv_aucf(&mcs_userid))
        goto error;

    mcs_send_cjrq(mcs_userid + 1001);
    if (!mcs_recv_cjcf())
        goto error;

    mcs_send_cjrq(MCS_GLOBAL_CHANNEL);
    if (!mcs_recv_cjcf())
        goto error;

    return true;

    error:
    iso_disconnect();
    return false;
}

/* Disconnect from the MCS layer */
void Client::mcs_disconnect() {
    iso_disconnect();
}

BOOL Client::bitmap_decompress(unsigned char *output, int width, int height, unsigned char *input, int size) {
    unsigned char *end = input + size;
    unsigned char *prevline = nullptr, *line = nullptr;
    int opcode, count, offset, isfillormix, x = width;
    int lastopcode = -1, insertmix = false, bicolor = false;
    uint8 code, color1 = 0, color2 = 0;
    uint8 mixmask, mask = 0, mix = 0xff;
    int fom_mask = 0;

    while (input < end) {
        fom_mask = 0;
        code = CVAL(input);
        opcode = code >> 4;

        /* Handle different opcode forms */
        switch (opcode) {
            case 0xc:
            case 0xd:
            case 0xe:
                opcode -= 6;
                count = code & 0xf;
                offset = 16;
                break;

            case 0xf:
                opcode = code & 0xf;
                if (opcode < 9) {
                    count = CVAL(input);
                    count |= CVAL(input) << 8;
                } else {
                    count = (opcode < 0xb) ? 8 : 1;
                }
                offset = 0;
                break;

            default:
                opcode >>= 1;
                count = code & 0x1f;
                offset = 32;
                break;
        }

        /* Handle strange cases for counts */
        if (offset != 0) {
            isfillormix = ((opcode == 2) || (opcode == 7));

            if (count == 0) {
                if (isfillormix)
                    count = CVAL(input) + 1;
                else
                    count = CVAL(input) + offset;
            } else if (isfillormix) {
                count <<= 3;
            }
        }

        /* Read preliminary data */
        switch (opcode) {
            case 0:    /* Fill */
                if ((lastopcode == opcode) && !((x == width) && (prevline == nullptr)))
                    insertmix = true;
                break;
            case 8:    /* Bicolor */
                color1 = CVAL(input);
            case 3:    /* Color */
                color2 = CVAL(input);
                break;
            case 6:    /* SetMix/Mix */
            case 7:    /* SetMix/FillOrMix */
                mix = CVAL(input);
                opcode -= 5;
                break;
            case 9:    /* FillOrMix_1 */
                mask = 0x03;
                opcode = 0x02;
                fom_mask = 3;
                break;
            case 0x0a:    /* FillOrMix_2 */
                mask = 0x05;
                opcode = 0x02;
                fom_mask = 5;
                break;
            default:
                break;
        }

        lastopcode = opcode;
        mixmask = 0;

        /* Output body */
        while (count > 0) {
            if (x >= width) {
                if (height <= 0)
                    return false;

                x = 0;
                height--;

                prevline = line;
                line = output + height * width;
            }

            switch (opcode) {
                case 0:    /* Fill */
                    if (insertmix) {
                        if (prevline == nullptr)
                            line[x] = mix;
                        else
                            line[x] = prevline[x] ^ mix;

                        insertmix = false;
                        count--;
                        x++;
                    }

                    if (prevline == nullptr) {
                        REPEAT(line[x] = 0);
                    } else {
                        REPEAT(line[x] = prevline[x]);
                    }
                    break;

                case 1:    /* Mix */
                    if (prevline == nullptr) {
                        REPEAT(line[x] = mix);
                    } else {
                        REPEAT(line[x] = prevline[x] ^ mix);
                    }
                    break;

                case 2:    /* Fill or Mix */
                    if (prevline == nullptr) {
                        REPEAT(MASK_UPDATE();
                                       if (mask & mixmask) line[x] = mix;
                                       else
                                           line[x] = 0;);
                    } else {
                        REPEAT(MASK_UPDATE();
                                       if (mask & mixmask)
                                           line[x] = prevline[x] ^ mix;
                                       else
                                           line[x] = prevline[x];);
                    }
                    break;

                case 3:    /* Color */
                REPEAT(line[x] = color2);
                    break;

                case 4:    /* Copy */
                REPEAT(line[x] = CVAL(input));
                    break;

                case 8:    /* Bicolor */
                REPEAT(if (bicolor) {
                    line[x] = color2;
                    bicolor = false;
                } else {
                    line[x] = color1;
                    bicolor = true;
                    count++;
                }
                );
                    break;

                case 0xd:    /* White */
                REPEAT(line[x] = 0xff);
                    break;

                case 0xe:    /* Black */
                REPEAT(line[x] = 0x00);
                    break;

                default:
                    throw not_implemented_error{};
                    return false;
            }
        }
    }

    return true;
}

/* Send a self-contained ISO PDU */
void Client::iso_send_msg(uint8 code) {
    STREAM s;

    s = tcptool->tcp_init(11);

    out_uint8(s, 3);    /* version */
    out_uint8(s, 0);    /* reserved */
    out_uint16_be(s, 11);    /* length */

    out_uint8(s, 6);    /* hdrlen */
    out_uint8(s, code);
    out_uint16(s, 0);    /* dst_ref */
    out_uint16(s, 0);    /* src_ref */
    out_uint8(s, 0);    /* class */

    s_mark_end(s);
    tcptool->tcp_send(s);
}

/* Receive a message on the ISO layer, return code */
STREAM Client::iso_recv_msg(uint8 *code) {
    STREAM s;
    uint16 length;
    uint8 version;

    s = tcptool->tcp_recv(4);
    if (s == nullptr)
        return nullptr;

    in_uint8(s, version);
    if (version != 3) {
        info("TPKT v%d\n", version);
        return nullptr;
    }

    in_uint8s(s, 1);    /* pad */
    in_uint16_be(s, length);

    s = tcptool->tcp_recv(length - 4);
    if (s == nullptr)
        return nullptr;

    in_uint8s(s, 1);    /* hdrlen */
    in_uint8(s, *code);

    if (*code == ISO_PDU_DT) {
        in_uint8s(s, 1);    /* eot */
        return s;
    }

    in_uint8s(s, 5);    /* dst_ref, src_ref, class */
    return s;
}

/* Initialise ISO transport data packet */
STREAM Client::iso_init(int length) {
    STREAM s;

    s = tcptool->tcp_init(length + 7);
    s_push_layer(s, iso_hdr, 7);

    return s;
}

/* Send an ISO data PDU */
void Client::iso_send(STREAM s) {
    uint16 length;

    s_pop_layer(s, iso_hdr);
    length = s->end - s->p;

    out_uint8(s, 3);    /* version */
    out_uint8(s, 0);    /* reserved */
    out_uint16_be(s, length);

    out_uint8(s, 2);    /* hdrlen */
    out_uint8(s, ISO_PDU_DT);    /* code */
    out_uint8(s, 0x80);    /* eot */

    tcptool->tcp_send(s);
}

/* Receive ISO transport data packet */
STREAM Client::iso_recv() {
    STREAM s;
    uint8 code;

    s = iso_recv_msg(&code);
    if (s == nullptr)
        return nullptr;

    if (code != ISO_PDU_DT) {
        info("expected DT, got 0x%x\n", code);
        return s;
    }

    return s;
}

/* Establish a connection up to the ISO layer */
BOOL Client::iso_connect(char *server) {
    uint8 code;

    if (!tcptool->tcp_connect(server))
        return false;

    iso_send_msg(ISO_PDU_CR);

    if (iso_recv_msg(&code) == nullptr)
        return false;

    if (code != ISO_PDU_CC) {
        info("expected CC, got 0x%x\n", code);
        tcptool->tcp_disconnect();
        return false;
    }

    return true;
}

/* Disconnect from the ISO layer */
void Client::iso_disconnect() {
    iso_send_msg(ISO_PDU_DR);
    tcptool->tcp_disconnect();
}


uint16 Client::xkeymap_translate_button(Qt::MouseButton button) {
    switch (button) {
        case Qt::MouseButton::LeftButton:    /* left */
            return MOUSE_FLAG_BUTTON1;
        case Qt::MouseButton::MidButton:    /* middle */
            return MOUSE_FLAG_BUTTON3;
        case Qt::MouseButton::RightButton:    /* right */
            return MOUSE_FLAG_BUTTON2;
        default:
            return 0;
    }
}


void Client::update_modifier_state(uint8 scancode, BOOL pressed) {
#ifdef WITH_DEBUG
    uint16 old_modifier_state;

    old_modifier_state = remote_modifier_state;
#endif

    switch (scancode) {
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
            if (pressed) {
                BOOL newNumLockState;
                newNumLockState =
                        (MASK_HAS_BITS
                         (remote_modifier_state, MapNumLockMask) == false);
                MASK_CHANGE_BIT(remote_modifier_state,
                                MapNumLockMask, newNumLockState);
            }
            break;
        default:
            break;
    }

#ifdef WITH_DEBUG
    if (old_modifier_state != remote_modifier_state)
    {
        info("Before updating modifier_state:0x%x, pressed=0x%x\n",
               old_modifier_state, pressed);
    }
#endif

}

/* Send keyboard input */
void Client::rdp_send_scancode(uint32 time, uint16 flags, uint8 scancode) {
    update_modifier_state(scancode, !(flags & RDP_KEYRELEASE));

    if (scancode & SCANCODE_EXTENDED) {
        rdp_send_input(time, RDP_INPUT_SCANCODE, flags | KBD_FLAG_EXT,
                       scancode & ~SCANCODE_EXTENDED, 0);
    } else {
        rdp_send_input(time, RDP_INPUT_SCANCODE, flags, scancode, 0);
    }
}

/* Generate a session key and RC4 keys, given client and server randoms */
void Client::licence_generate_keys(uint8 *client_key, uint8 *server_key, uint8 *client_rsa) {
    uint8 session_key[48];
    uint8 temp_hash[48];

    /* Generate session key - two rounds of sec_hash_48 */
    sec_hash_48(temp_hash, client_rsa, client_key, server_key, 65);
    sec_hash_48(session_key, temp_hash, server_key, client_key, 65);

    /* Store first 16 bytes of session key, for generating signatures */
    memcpy(licence_sign_key, session_key, 16);

    /* Generate RC4 key */
    sec_hash_16(licence_key, &session_key[16], client_key, server_key);
}

/* Send a licence request packet */
void Client::licence_send_request(uint8 *client_random, uint8 *rsa_data, char *user, char *host) {
    uint32 sec_flags = SEC_LICENCE_NEG;
    uint16 userlen = strlen(user) + 1;
    uint16 hostlen = strlen(host) + 1;
    uint16 length = 128 + userlen + hostlen;
    STREAM s;

    s = sec_init(sec_flags, length + 2);

    out_uint16_le(s, LICENCE_TAG_REQUEST);
    out_uint16_le(s, length);

    out_uint32_le(s, 1);
    out_uint16(s, 0);
    out_uint16_le(s, 0xff01);

    out_uint8p(s, client_random, SEC_RANDOM_SIZE);
    out_uint16(s, 0);
    out_uint16_le(s, (SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
    out_uint8p(s, rsa_data, SEC_MODULUS_SIZE);
    out_uint8s(s, SEC_PADDING_SIZE);

    out_uint16(s, LICENCE_TAG_USER);
    out_uint16(s, userlen);
    out_uint8p(s, user, userlen);

    out_uint16(s, LICENCE_TAG_HOST);
    out_uint16(s, hostlen);
    out_uint8p(s, host, hostlen);

    s_mark_end(s);
    sec_send(s, sec_flags);
}

/* Process a licence demand packet */
void Client::licence_process_demand(STREAM s) {
    uint8 nullptr_data[SEC_MODULUS_SIZE];
    uint8 *server_random;
#ifdef SAVE_LICENCE
    uint8 signature[LICENCE_SIGNATURE_SIZE];
    uint8 hwid[LICENCE_HWID_SIZE];
    uint8 *licence_data;
    int licence_size;
    RC4_KEY crypt_key;
#endif

    /* Retrieve the server random from the incoming packet */
    in_uint8p(s, server_random, SEC_RANDOM_SIZE);

    /* We currently use nullptr client keys. This is a bit naughty but, hey,
       the security of licence negotiation isn't exactly paramount. */
    memset(nullptr_data, 0, sizeof(nullptr_data));
    licence_generate_keys(nullptr_data, server_random, nullptr_data);
    licence_send_request(nullptr_data, nullptr_data, username, hostname);
}

/* Process an authentication request packet */
void Client::licence_process_authreq(STREAM s) {
    throw not_implemented_error();
}

/* Process an licence issue packet */
void Client::licence_process_issue(STREAM s) {
    throw not_implemented_error();
}

/* Process a licence packet */
void Client::licence_process(STREAM s) {
    uint16 tag;

    in_uint16_le(s, tag);
    in_uint8s(s, 2);    /* length */

    switch (tag) {
        case LICENCE_TAG_DEMAND:
            licence_process_demand(s);
            break;

        case LICENCE_TAG_AUTHREQ:
            licence_process_authreq(s);
            break;

        case LICENCE_TAG_ISSUE:
            licence_process_issue(s);
            break;

        case LICENCE_TAG_REISSUE:

        case LICENCE_TAG_RESULT:
            break;

        default:
            throw not_implemented_error{};
    }
}

/* Read field indicating which parameters are present */
void Client::rdp_in_present(STREAM s, uint32 *present, uint8 flags, int size) {
    uint8 bits;
    int i;

    if (flags & RDP_ORDER_SMALL) {
        size--;
    }

    if (flags & RDP_ORDER_TINY) {
        if (size < 2)
            size = 0;
        else
            size -= 2;
    }

    *present = 0;
    for (i = 0; i < size; i++) {
        in_uint8(s, bits);
        *present |= bits << (i * 8);
    }
}

/* Read a co-ordinate (16-bit, or 8-bit delta) */
void Client::rdp_in_coord(STREAM s, uint16 *coord, BOOL delta) {
    sint8 change;

    if (delta) {
        in_uint8(s, change);
        *coord += change;
    } else {
        in_uint16_le(s, *coord);
    }
}

/* Read a color entry */
void Client::rdp_in_color(STREAM s, uint8 *color) {
    in_uint8(s, *color);
    s->p += 2;
}

/* Parse bounds information */
BOOL Client::rdp_parse_bounds(STREAM s, BOUNDS *bounds) {
    uint8 present;

    in_uint8(s, present);

    if (present & 1)
        rdp_in_coord(s, &bounds->left, false);
    else if (present & 16)
        rdp_in_coord(s, &bounds->left, true);

    if (present & 2)
        rdp_in_coord(s, &bounds->top, false);
    else if (present & 32)
        rdp_in_coord(s, &bounds->top, true);

    if (present & 4)
        rdp_in_coord(s, &bounds->right, false);
    else if (present & 64)
        rdp_in_coord(s, &bounds->right, true);

    if (present & 8)
        rdp_in_coord(s, &bounds->bottom, false);
    else if (present & 128)
        rdp_in_coord(s, &bounds->bottom, true);

    return s_check(s);
}

/* Parse a pen */
BOOL Client::rdp_parse_pen(STREAM s, PEN *pen, uint32 present) {
    if (present & 1)
        in_uint8(s, pen->style);

    if (present & 2)
        in_uint8(s, pen->width);

    if (present & 4)
        rdp_in_color(s, &pen->color);

    return s_check(s);
}

/* Parse a brush */
BOOL Client::rdp_parse_brush(STREAM s, BRUSH *brush, uint32 present) {
    if (present & 1)
        in_uint8(s, brush->xorigin);

    if (present & 2)
        in_uint8(s, brush->yorigin);

    if (present & 4)
        in_uint8(s, brush->style);

    if (present & 8)
        in_uint8(s, brush->pattern[0]);

    if (present & 16) in_uint8a(s, &brush->pattern[1], 7);

    return s_check(s);
}

/* Process a destination blt order */
void Client::process_destblt(STREAM s, DESTBLT_ORDER *os, uint32 present, BOOL delta) {
    if (present & 0x01)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x02)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x04)
        rdp_in_coord(s, &os->cx, delta);

    if (present & 0x08)
        rdp_in_coord(s, &os->cy, delta);

    if (present & 0x10)
        in_uint8(s, os->opcode);

    xwin_ui->ui_destblt(ROP2_S(os->opcode), os->x, os->y, os->cx, os->cy);
}

/* Process a pattern blt order */
void Client::process_patblt(STREAM s, PATBLT_ORDER *os, uint32 present, BOOL delta) {
    if (present & 0x0001)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x0002)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x0004)
        rdp_in_coord(s, &os->cx, delta);

    if (present & 0x0008)
        rdp_in_coord(s, &os->cy, delta);

    if (present & 0x0010)
        in_uint8(s, os->opcode);

    if (present & 0x0020)
        rdp_in_color(s, &os->bgcolor);

    if (present & 0x0040)
        rdp_in_color(s, &os->fgcolor);

    rdp_parse_brush(s, &os->brush, present >> 7);

    xwin_ui->ui_patblt(ROP2_P(os->opcode), os->x, os->y, os->cx, os->cy,
                       &os->brush, os->fgcolor);
}

/* Process a screen blt order */
void Client::process_screenblt(STREAM s, SCREENBLT_ORDER *os, uint32 present, BOOL delta) {
    if (present & 0x0001)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x0002)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x0004)
        rdp_in_coord(s, &os->cx, delta);

    if (present & 0x0008)
        rdp_in_coord(s, &os->cy, delta);

    if (present & 0x0010)
        in_uint8(s, os->opcode);

    if (present & 0x0020)
        rdp_in_coord(s, &os->srcx, delta);

    if (present & 0x0040)
        rdp_in_coord(s, &os->srcy, delta);

    xwin_ui->ui_screenblt(ROP2_S(os->opcode), os->x, os->y, os->cx, os->cy, os->srcx, os->srcy);
}

/* Process a line order */
void Client::process_line(STREAM s, LINE_ORDER *os, uint32 present, BOOL delta) {
    if (present & 0x0001) in_uint16_le(s, os->mixmode);

    if (present & 0x0002)
        rdp_in_coord(s, &os->startx, delta);

    if (present & 0x0004)
        rdp_in_coord(s, &os->starty, delta);

    if (present & 0x0008)
        rdp_in_coord(s, &os->endx, delta);

    if (present & 0x0010)
        rdp_in_coord(s, &os->endy, delta);

    if (present & 0x0020)
        rdp_in_color(s, &os->bgcolor);

    if (present & 0x0040)
        in_uint8(s, os->opcode);

    rdp_parse_pen(s, &os->pen, present >> 7);

    if (os->opcode < 0x01 || os->opcode > 0x10) {
        info("bad ROP2 0x%x\n", os->opcode);
        return;
    }

    xwin_ui->ui_line(os->opcode - 1, os->startx, os->starty, os->endx, os->endy, &os->pen);
}

/* Process an opaque rectangle order */
void Client::process_rect(STREAM s, RECT_ORDER *os, uint32 present, BOOL delta) {
    if (present & 0x01)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x02)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x04)
        rdp_in_coord(s, &os->cx, delta);

    if (present & 0x08)
        rdp_in_coord(s, &os->cy, delta);

    if (present & 0x10)
        in_uint8(s, os->color);

    xwin_ui->ui_rect(os->x, os->y, os->cx, os->cy, os->color);
}

/* Process a desktop save order */
void Client::process_desksave(STREAM s, DESKSAVE_ORDER *os, uint32 present, BOOL delta) {
    int width, height;

    if (present & 0x01) in_uint32_le(s, os->offset);

    if (present & 0x02)
        rdp_in_coord(s, &os->left, delta);

    if (present & 0x04)
        rdp_in_coord(s, &os->top, delta);

    if (present & 0x08)
        rdp_in_coord(s, &os->right, delta);

    if (present & 0x10)
        rdp_in_coord(s, &os->bottom, delta);

    if (present & 0x20)
        in_uint8(s, os->action);

    width = os->right - os->left + 1;
    height = os->bottom - os->top + 1;

    if (os->action == 0)
        xwin_ui->ui_desktop_save(os->offset, os->left, os->top, width, height);
    else
        xwin_ui->ui_desktop_restore(os->offset, os->left, os->top, width, height);
}

/* Process a memory blt order */
void Client::process_memblt(STREAM s, MEMBLT_ORDER *os, uint32 present, BOOL delta) {
    HRDPBITMAP bitmap;

    if (present & 0x0001) {
        in_uint8(s, os->cache_id);
        in_uint8(s, os->color_table);
    }

    if (present & 0x0002)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x0004)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x0008)
        rdp_in_coord(s, &os->cx, delta);

    if (present & 0x0010)
        rdp_in_coord(s, &os->cy, delta);

    if (present & 0x0020)
        in_uint8(s, os->opcode);

    if (present & 0x0040)
        rdp_in_coord(s, &os->srcx, delta);

    if (present & 0x0080)
        rdp_in_coord(s, &os->srcy, delta);

    if (present & 0x0100) in_uint16_le(s, os->cache_idx);

    bitmap = xwin_ui->cache_get_bitmap(os->cache_id, os->cache_idx);
    if (bitmap == nullptr)
        return;

    xwin_ui->ui_memblt(ROP2_S(os->opcode), os->x, os->y, os->cx, os->cy, bitmap, os->srcx, os->srcy);
}

/* Process a 3-way blt order */
void Client::process_triblt(STREAM s, TRIBLT_ORDER *os, uint32 present, BOOL delta) {
    HRDPBITMAP bitmap;

    if (present & 0x000001) {
        in_uint8(s, os->cache_id);
        in_uint8(s, os->color_table);
    }

    if (present & 0x000002)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x000004)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x000008)
        rdp_in_coord(s, &os->cx, delta);

    if (present & 0x000010)
        rdp_in_coord(s, &os->cy, delta);

    if (present & 0x000020)
        in_uint8(s, os->opcode);

    if (present & 0x000040)
        rdp_in_coord(s, &os->srcx, delta);

    if (present & 0x000080)
        rdp_in_coord(s, &os->srcy, delta);

    if (present & 0x000100)
        rdp_in_color(s, &os->bgcolor);

    if (present & 0x000200)
        rdp_in_color(s, &os->fgcolor);

    rdp_parse_brush(s, &os->brush, present >> 10);

    if (present & 0x008000) in_uint16_le(s, os->cache_idx);

    if (present & 0x010000) in_uint16_le(s, os->unknown);

    bitmap = xwin_ui->cache_get_bitmap(os->cache_id, os->cache_idx);
    if (bitmap == nullptr)
        return;

    xwin_ui->ui_triblt(os->opcode, os->x, os->y, os->cx, os->cy,
                       bitmap, os->srcx, os->srcy, &os->brush, os->bgcolor, os->fgcolor);
}

/* Parse a delta co-ordinate in polyline order form */
int Client::parse_delta(const uint8 *buffer, int *offset) {
    int value = buffer[(*offset)++];
    int two_byte = value & 0x80;

    if (value & 0x40)    /* sign bit */
        value |= ~0x3f;
    else
        value &= 0x3f;

    if (two_byte)
        value = (value << 8) | buffer[(*offset)++];

    return value;
}

/* Process a polyline order */
void Client::process_polyline(STREAM s, POLYLINE_ORDER *os, uint32 present, BOOL delta) {
    int index, line, data;
    int x, y, xfrom, yfrom;
    uint8 flags = 0;
    PEN pen;
    uint8 opcode;

    if (present & 0x01)
        rdp_in_coord(s, &os->x, delta);

    if (present & 0x02)
        rdp_in_coord(s, &os->y, delta);

    if (present & 0x04)
        in_uint8(s, os->opcode);

    if (present & 0x10)
        rdp_in_color(s, &os->fgcolor);

    if (present & 0x20)
        in_uint8(s, os->lines);

    if (present & 0x40) {
        in_uint8(s, os->datasize);
        in_uint8a(s, os->data, os->datasize);
    }
    if (os->opcode < 0x01 || os->opcode > 0x10) {
        info("bad ROP2 0x%x\n", os->opcode);
        return;
    }

    opcode = os->opcode - 1;
    x = os->x;
    y = os->y;
    pen.style = pen.width = 0;
    pen.color = os->fgcolor;

    index = 0;
    data = ((os->lines - 1) / 4) + 1;
    for (line = 0; (line < os->lines) && (data < os->datasize); line++) {
        xfrom = x;
        yfrom = y;

        if (line % 4 == 0)
            flags = os->data[index++];

        if ((flags & 0xc0) == 0)
            flags |= 0xc0;    /* none = both */

        if (flags & 0x40)
            x += parse_delta(os->data, &data);

        if (flags & 0x80)
            y += parse_delta(os->data, &data);

        xwin_ui->ui_line(opcode, xfrom, yfrom, x, y, &pen);

        flags <<= 2;
    }
}

/* Process a text order */
void Client::process_text2(STREAM s, TEXT2_ORDER *os, uint32 present, BOOL delta) {
    int i;

    if (present & 0x000001)
        in_uint8(s, os->font);

    if (present & 0x000002)
        in_uint8(s, os->flags);

    if (present & 0x000004)
        in_uint8(s, os->unknown);

    if (present & 0x000008)
        in_uint8(s, os->mixmode);

    if (present & 0x000010)
        rdp_in_color(s, &os->fgcolor);

    if (present & 0x000020)
        rdp_in_color(s, &os->bgcolor);

    if (present & 0x000040) in_uint16_le(s, os->clipleft);

    if (present & 0x000080) in_uint16_le(s, os->cliptop);

    if (present & 0x000100) in_uint16_le(s, os->clipright);

    if (present & 0x000200) in_uint16_le(s, os->clipbottom);

    if (present & 0x000400) in_uint16_le(s, os->boxleft);

    if (present & 0x000800) in_uint16_le(s, os->boxtop);

    if (present & 0x001000) in_uint16_le(s, os->boxright);

    if (present & 0x002000) in_uint16_le(s, os->boxbottom);

    if (present & 0x080000) in_uint16_le(s, os->x);

    if (present & 0x100000) in_uint16_le(s, os->y);

    if (present & 0x200000) {
        in_uint8(s, os->length);
        in_uint8a(s, os->text, os->length);
    }

    xwin_ui->ui_draw_text(os->font, os->flags, os->mixmode, os->x, os->y,
                          os->clipleft, os->cliptop,
                          os->clipright - os->clipleft,
                          os->clipbottom - os->cliptop,
                          os->boxleft, os->boxtop,
                          os->boxright - os->boxleft,
                          os->boxbottom - os->boxtop, os->bgcolor, os->fgcolor, os->text, os->length);
}

/* Process a raw bitmap cache order */
void Client::process_raw_bmpcache(STREAM s) {
    HRDPBITMAP bitmap;
    uint16 cache_idx, bufsize;
    uint8 cache_id, width, height, bpp;
    uint8 *data, *inverted;
    int y;

    in_uint8(s, cache_id);
    in_uint8s(s, 1);    /* pad */
    in_uint8(s, width);
    in_uint8(s, height);
    in_uint8(s, bpp);
    in_uint16_le(s, bufsize);
    in_uint16_le(s, cache_idx);
    in_uint8p(s, data, bufsize);

    inverted = new uint8[width * height];
    for (y = 0; y < height; y++) {
        memcpy(&inverted[(height - y - 1) * width], &data[y * width], width);
    }

    bitmap = xwin_ui->ui_create_bitmap(width, height, inverted);
    delete[]inverted;
    xwin_ui->cache_put_bitmap(cache_id, cache_idx, bitmap);
}

/* Process a bitmap cache order */
void Client::process_bmpcache(STREAM s) {
    HRDPBITMAP bitmap;
    uint16 cache_idx, size;
    uint8 cache_id, width, height, bpp;
    uint8 *data, *bmpdata;

    in_uint8(s, cache_id);
    in_uint8s(s, 1);    /* pad */
    in_uint8(s, width);
    in_uint8(s, height);
    in_uint8(s, bpp);
    in_uint8s(s, 2);    /* bufsize */
    in_uint16_le(s, cache_idx);
    in_uint8s(s, 2);    /* pad */
    in_uint16_le(s, size);
    in_uint8s(s, 4);    /* row_size, final_size */
    in_uint8p(s, data, size);

    bmpdata = new uint8[width * height];

    if (bitmap_decompress(bmpdata, width, height, data, size)) {
        bitmap = xwin_ui->ui_create_bitmap(width, height, bmpdata);
        xwin_ui->cache_put_bitmap(cache_id, cache_idx, bitmap);
    }

    delete[]bmpdata;
}

/* Process a colormap cache order */
void Client::process_colcache(STREAM s) {
    COLORENTRY *entry;
    COLORMAP map;
    HCOLORMAP hmap;
    uint8 cache_id;
    int i;

    in_uint8(s, cache_id);
    in_uint16_le(s, map.ncolors);

    map.colors = new COLORENTRY[3 * map.ncolors];

    for (i = 0; i < map.ncolors; i++) {
        entry = &map.colors[i];
        in_uint8(s, entry->blue);
        in_uint8(s, entry->green);
        in_uint8(s, entry->red);
        in_uint8s(s, 1);    /* pad */
    }

    xwin_ui->ui_create_colormap(&map);

    delete[]map.colors;
}

/* Process a font cache order */
void Client::process_fontcache(STREAM s) {
    HGLYPH bitmap;
    uint8 font, nglyphs;
    uint16 character, offset, baseline, width, height;
    int i, datasize;
    uint8 *data;

    in_uint8(s, font);
    in_uint8(s, nglyphs);

    for (i = 0; i < nglyphs; i++) {
        in_uint16_le(s, character);
        in_uint16_le(s, offset);
        in_uint16_le(s, baseline);
        in_uint16_le(s, width);
        in_uint16_le(s, height);

        datasize = (height * ((width + 7) / 8) + 3) & ~3;
        in_uint8p(s, data, datasize);

        bitmap = xwin_ui->ui_create_glyph(width, height, data);
        xwin_ui->cache_put_font(font, character, offset, baseline, width, height, bitmap);
    }
}

/* Process a secondary order */
void Client::process_secondary_order(STREAM s) {
    uint16 length;
    uint8 type;
    uint8 *next_order;

    in_uint16_le(s, length);
    in_uint8s(s, 2);    /* flags */
    in_uint8(s, type);

    next_order = s->p + length + 7;

    switch (type) {
        case RDP_ORDER_RAW_BMPCACHE:
            process_raw_bmpcache(s);
            break;

        case RDP_ORDER_COLCACHE:
            process_colcache(s);
            break;

        case RDP_ORDER_BMPCACHE:
            process_bmpcache(s);
            break;

        case RDP_ORDER_FONTCACHE:
            process_fontcache(s);
            break;

        default:
            throw not_implemented_error{};
    }

    s->p = next_order;
}

/* Process an order PDU */
void Client::process_orders(STREAM s) {
    RDP_ORDER_STATE *os = &order_state;
    uint32 present;
    uint16 num_orders;
    uint8 order_flags;
    int size, processed = 0;
    BOOL delta;

    in_uint8s(s, 2);    /* pad */
    in_uint16_le(s, num_orders);
    in_uint8s(s, 2);    /* pad */

    while (processed < num_orders) {
        in_uint8(s, order_flags);

        if (!(order_flags & RDP_ORDER_STANDARD)) {
            info("order parsing failed\n");
            break;
        }

        if (order_flags & RDP_ORDER_SECONDARY) {
            process_secondary_order(s);
        } else {
            if (order_flags & RDP_ORDER_CHANGE) {
                in_uint8(s, os->order_type);
            }

            switch (os->order_type) {
                case RDP_ORDER_TRIBLT:
                case RDP_ORDER_TEXT2:
                    size = 3;
                    break;

                case RDP_ORDER_PATBLT:
                case RDP_ORDER_MEMBLT:
                case RDP_ORDER_LINE:
                    size = 2;
                    break;

                default:
                    size = 1;
            }

            rdp_in_present(s, &present, order_flags, size);

            if (order_flags & RDP_ORDER_BOUNDS) {
                if (!(order_flags & RDP_ORDER_LASTBOUNDS))
                    rdp_parse_bounds(s, &os->bounds);

                xwin_ui->ui_set_clip(os->bounds.left,
                                     os->bounds.top,
                                     os->bounds.right -
                                     os->bounds.left + 1,
                                     os->bounds.bottom - os->bounds.top + 1);
            }

            delta = order_flags & RDP_ORDER_DELTA;

            switch (os->order_type) {
                case RDP_ORDER_DESTBLT:
                    process_destblt(s, &os->destblt, present, delta);
                    break;

                case RDP_ORDER_PATBLT:
                    process_patblt(s, &os->patblt, present, delta);
                    break;

                case RDP_ORDER_SCREENBLT:
                    process_screenblt(s, &os->screenblt, present, delta);
                    break;

                case RDP_ORDER_LINE:
                    process_line(s, &os->line, present, delta);
                    break;

                case RDP_ORDER_RECT:
                    process_rect(s, &os->rect, present, delta);
                    break;

                case RDP_ORDER_DESKSAVE:
                    process_desksave(s, &os->desksave, present, delta);
                    break;

                case RDP_ORDER_MEMBLT:
                    process_memblt(s, &os->memblt, present, delta);
                    break;

                case RDP_ORDER_TRIBLT:
                    process_triblt(s, &os->triblt, present, delta);
                    break;

                case RDP_ORDER_POLYLINE:
                    process_polyline(s, &os->polyline, present, delta);
                    break;

                case RDP_ORDER_TEXT2:
                    process_text2(s, &os->text2, present, delta);
                    break;

                default:
                    throw not_implemented_error{};
                    return;
            }

            if (order_flags & RDP_ORDER_BOUNDS)
                xwin_ui->ui_reset_clip();
        }

        processed++;
    }

    if (s->p != next_packet)
        info("%d bytes remaining\n", (int) (next_packet - s->p));
}

/* Reset order state */
void Client::reset_order_state() {
    memset(&order_state, 0, sizeof(order_state));
    order_state.order_type = RDP_ORDER_PATBLT;
}


/* Output a uint32 into a buffer (little-endian) */
void Client::buf_out_uint32(uint8 *buffer, uint32 value) {
    buffer[0] = (value) & 0xff;
    buffer[1] = (value >> 8) & 0xff;
    buffer[2] = (value >> 16) & 0xff;
    buffer[3] = (value >> 24) & 0xff;
}

/*
 * General purpose 48-byte transformation, using two 32-byte salts (generally,
 * a client and server salt) and a global salt value used for padding.
 * Both SHA1 and MD5 algorithms are used.
 */
void Client::sec_hash_48(uint8 *out, uint8 *in, uint8 *salt1, uint8 *salt2, uint8 salt) {
    uint8 shasig[20];
    uint8 pad[4];
    SHA_CTX sha;
    MD5_CTX md5;
    int i;

    for (i = 0; i < 3; i++) {
        memset(pad, salt + i, i + 1);

        SHA1_Init(&sha);
        SHA1_Update(&sha, pad, i + 1);
        SHA1_Update(&sha, in, 48);
        SHA1_Update(&sha, salt1, 32);
        SHA1_Update(&sha, salt2, 32);
        SHA1_Final(shasig, &sha);

        MD5_Init(&md5);
        MD5_Update(&md5, in, 48);
        MD5_Update(&md5, shasig, 20);
        MD5_Final(&out[i * 16], &md5);
    }
}

/*
 * Weaker 16-byte transformation, also using two 32-byte salts, but
 * only using a single round of MD5.
 */
void Client::sec_hash_16(uint8 *out, uint8 *in, uint8 *salt1, uint8 *salt2) {
    MD5_CTX md5;

    MD5_Init(&md5);
    MD5_Update(&md5, in, 16);
    MD5_Update(&md5, salt1, 32);
    MD5_Update(&md5, salt2, 32);
    MD5_Final(out, &md5);
}

/* Reduce key entropy from 64 to 40 bits */
void Client::sec_make_40bit(uint8 *key) {
    key[0] = 0xd1;
    key[1] = 0x26;
    key[2] = 0x9e;
}

/* Generate a session key and RC4 keys, given client and server randoms */
void Client::sec_generate_keys(uint8 *client_key, uint8 *server_key, int rc4_key_size) {
    uint8 session_key[48];
    uint8 temp_hash[48];
    uint8 input[48];

    /* Construct input data to hash */
    memcpy(input, client_key, 24);
    memcpy(input + 24, server_key, 24);

    /* Generate session key - two rounds of sec_hash_48 */
    sec_hash_48(temp_hash, input, client_key, server_key, 65);
    sec_hash_48(session_key, temp_hash, client_key, server_key, 88);

    /* Store first 16 bytes of session key, for generating signatures */
    memcpy(sec_sign_key, session_key, 16);

    /* Generate RC4 keys */
    sec_hash_16(sec_decrypt_key, &session_key[16], client_key, server_key);
    sec_hash_16(sec_encrypt_key, &session_key[32], client_key, server_key);

    if (rc4_key_size == 1) {
        sec_make_40bit(sec_sign_key);
        sec_make_40bit(sec_decrypt_key);
        sec_make_40bit(sec_encrypt_key);
        rc4_key_len = 8;
    } else {
        rc4_key_len = 16;
    }

    /* Save initial RC4 keys as update keys */
    memcpy(sec_decrypt_update_key, sec_decrypt_key, 16);
    memcpy(sec_encrypt_update_key, sec_encrypt_key, 16);

    /* Initialise RC4 state arrays */
    RC4_set_key(&rc4_decrypt_key, rc4_key_len, sec_decrypt_key);
    RC4_set_key(&rc4_encrypt_key, rc4_key_len, sec_encrypt_key);
}

/* Generate a signature hash, using a combination of SHA1 and MD5 */
void Client::sec_sign(uint8 *signature, int siglen, uint8 *session_key, int keylen, uint8 *data, int datalen) {
    uint8 shasig[20];
    uint8 md5sig[16];
    uint8 lenhdr[4];
    SHA_CTX sha;
    MD5_CTX md5;

    buf_out_uint32(lenhdr, datalen);

    SHA1_Init(&sha);
    SHA1_Update(&sha, session_key, keylen);
    SHA1_Update(&sha, pad_54, 40);
    SHA1_Update(&sha, lenhdr, 4);
    SHA1_Update(&sha, data, datalen);
    SHA1_Final(shasig, &sha);

    MD5_Init(&md5);
    MD5_Update(&md5, session_key, keylen);
    MD5_Update(&md5, pad_92, 48);
    MD5_Update(&md5, shasig, 20);
    MD5_Final(md5sig, &md5);

    memcpy(signature, md5sig, siglen);
}

/* Update an encryption key - similar to the signing process */
void Client::sec_update(uint8 *key, uint8 *update_key) {
    uint8 shasig[20];
    SHA_CTX sha;
    MD5_CTX md5;
    RC4_KEY update;

    SHA1_Init(&sha);
    SHA1_Update(&sha, update_key, rc4_key_len);
    SHA1_Update(&sha, pad_54, 40);
    SHA1_Update(&sha, key, rc4_key_len);
    SHA1_Final(shasig, &sha);

    MD5_Init(&md5);
    MD5_Update(&md5, update_key, rc4_key_len);
    MD5_Update(&md5, pad_92, 48);
    MD5_Update(&md5, shasig, 20);
    MD5_Final(key, &md5);

    RC4_set_key(&update, rc4_key_len, key);
    RC4(&update, rc4_key_len, key, key);

    if (rc4_key_len == 8)
        sec_make_40bit(key);
}

/* Encrypt data using RC4 */
void Client::sec_encrypt(uint8 *data, int length) {
    static int use_count;

    if (use_count == 4096) {
        sec_update(sec_encrypt_key, sec_encrypt_update_key);
        RC4_set_key(&rc4_encrypt_key, rc4_key_len, sec_encrypt_key);
        use_count = 0;
    }

    RC4(&rc4_encrypt_key, length, data, data);
    use_count++;
}

/* Decrypt data using RC4 */
void Client::sec_decrypt(uint8 *data, int length) {
    static int use_count;

    if (use_count == 4096) {
        sec_update(sec_decrypt_key, sec_decrypt_update_key);
        RC4_set_key(&rc4_decrypt_key, rc4_key_len, sec_decrypt_key);
        use_count = 0;
    }

    RC4(&rc4_decrypt_key, length, data, data);
    use_count++;
}

void Client::reverse(uint8 *p, int len) {
    int i, j;
    uint8 temp;

    for (i = 0, j = len - 1; i < j; i++, j--) {
        temp = p[i];
        p[i] = p[j];
        p[j] = temp;
    }
}

/* Perform an RSA public key encryption operation */
void Client::sec_rsa_encrypt(uint8 *out, uint8 *in, int len, uint8 *modulus, uint8 *exponent) {
    BN_CTX *ctx;
    BIGNUM *mod, *exp, *x, *y;
    uint8 inr[SEC_MODULUS_SIZE];
    int outlen;

    reverse(modulus, SEC_MODULUS_SIZE);
    reverse(exponent, SEC_EXPONENT_SIZE);
    memcpy(inr, in, len);
    reverse(inr, len);

    ctx = BN_CTX_new();
    mod = BN_new();
    exp = BN_new();
    x = BN_new();
    y = BN_new();

    BN_bin2bn(modulus, SEC_MODULUS_SIZE, mod);
    BN_bin2bn(exponent, SEC_EXPONENT_SIZE, exp);
    BN_bin2bn(inr, len, x);
    BN_mod_exp(y, x, exp, mod, ctx);
    outlen = BN_bn2bin(y, out);
    reverse(out, outlen);
    if (outlen < SEC_MODULUS_SIZE)
        memset(out + outlen, 0, SEC_MODULUS_SIZE - outlen);

    BN_free(y);
    BN_clear_free(x);
    BN_free(exp);
    BN_free(mod);
    BN_CTX_free(ctx);
}

/* Initialise secure transport packet */
STREAM Client::sec_init(uint32 flags, int maxlen) {
    int hdrlen;
    STREAM s;

    if (!licence_issued)
        hdrlen = (flags & SEC_ENCRYPT) ? 12 : 4;
    else
        hdrlen = (flags & SEC_ENCRYPT) ? 12 : 0;
    s = mcs_init(maxlen + hdrlen);
    s_push_layer(s, sec_hdr, hdrlen);

    return s;
}

/* Transmit secure transport packet */
void Client::sec_send(STREAM s, uint32 flags) {
    int datalen;

    s_pop_layer(s, sec_hdr);
    if (!licence_issued || (flags & SEC_ENCRYPT)) out_uint32_le(s, flags);

    if (flags & SEC_ENCRYPT) {
        flags &= ~SEC_ENCRYPT;
        datalen = s->end - s->p - 8;

#if WITH_DEBUG
        hexdump(s->p + 8, datalen);
#endif

        sec_sign(s->p, 8, sec_sign_key, rc4_key_len, s->p + 8, datalen);
        sec_encrypt(s->p + 8, datalen);
    }

    mcs_send(s);
}

/* Transfer the client random to the server */
void Client::sec_establish_key() {
    uint32 length = SEC_MODULUS_SIZE + SEC_PADDING_SIZE;
    uint32 flags = SEC_CLIENT_RANDOM;
    STREAM s;

    s = sec_init(flags, 76);

    out_uint32_le(s, length);
    out_uint8p(s, sec_crypted_random, SEC_MODULUS_SIZE);
    out_uint8s(s, SEC_PADDING_SIZE);

    s_mark_end(s);
    sec_send(s, flags);
}

/* Output connect initial data blob */
void Client::sec_out_mcs_data(STREAM s) {
    int hostlen = 2 * strlen(hostname);

    if (hostlen > 30)
        hostlen = 30;

    out_uint16_be(s, 5);    /* unknown */
    out_uint16_be(s, 0x14);
    out_uint8(s, 0x7c);
    out_uint16_be(s, 1);

    out_uint16_be(s, (158 | 0x8000));    /* remaining length */

    out_uint16_be(s, 8);    /* length? */
    out_uint16_be(s, 16);
    out_uint8(s, 0);
    out_uint16_le(s, 0xc001);
    out_uint8(s, 0);

    out_uint32_le(s, 0x61637544);    /* "Duca" ?! */
    out_uint16_be(s, (144 | 0x8000));    /* remaining length */

    /* Client information */
    out_uint16_le(s, SEC_TAG_CLI_INFO);
    out_uint16_le(s, 136);    /* length */
    out_uint16_le(s, 1);
    out_uint16_le(s, 8);
    out_uint16_le(s, width);
    out_uint16_le(s, height);
    out_uint16_le(s, 0xca01);
    out_uint16_le(s, 0xaa03);
    out_uint32_le(s, keylayout);
    out_uint32_le(s, 419);    /* client build? we are 419 compatible :-) */

    /* Unicode name of client, padded to 32 bytes */
    rdp_out_unistr(s, hostname, hostlen);
    out_uint8s(s, 30 - hostlen);

    out_uint32_le(s, 4);
    out_uint32(s, 0);
    out_uint32_le(s, 12);
    out_uint8s(s, 64);    /* reserved? 4 + 12 doublewords */

    out_uint16_le(s, 0xca01);
    out_uint16(s, 0);

    /* Client encryption settings */
    out_uint16_le(s, SEC_TAG_CLI_CRYPT);
    out_uint16_le(s, 8);    /* length */
    out_uint32_le(s, encryption ? 0x3 : 0);    /* encryption supported, 128-bit supported */
    s_mark_end(s);
}

/* Parse a public key structure */
BOOL Client::sec_parse_public_key(STREAM s, uint8 **modulus, uint8 **exponent) {
    uint32 magic, modulus_len;

    in_uint32_le(s, magic);
    if (magic != SEC_RSA_MAGIC) {
        info("RSA magic 0x%x\n", magic);
        return false;
    }

    in_uint32_le(s, modulus_len);
    if (modulus_len != SEC_MODULUS_SIZE + SEC_PADDING_SIZE) {
        //info("modulus len 0x%x\n", modulus_len);
        //return false;
    }

    in_uint8s(s, 8);    /* modulus_bits, unknown */
    in_uint8p(s, *exponent, SEC_EXPONENT_SIZE);
    in_uint8p(s, *modulus, SEC_MODULUS_SIZE);
    in_uint8s(s, SEC_PADDING_SIZE);

    return s_check(s);
}

/* Parse a crypto information structure */
BOOL Client::sec_parse_crypt_info(STREAM s, uint32 *rc4_key_size,
                                  uint8 **server_random, uint8 **modulus, uint8 **exponent) {
    uint32 crypt_level, random_len, rsa_info_len;
    uint16 tag, length;
    uint8 *next_tag, *end;

    in_uint32_le(s, *rc4_key_size);    /* 1 = 40-bit, 2 = 128-bit */
    in_uint32_le(s, crypt_level);    /* 1 = low, 2 = medium, 3 = high */
    if (crypt_level == 0)    /* no encryptation */
        return false;
    in_uint32_le(s, random_len);
    in_uint32_le(s, rsa_info_len);

    if (random_len != SEC_RANDOM_SIZE) {
        info("random len %d\n", random_len);
        return false;
    }

    in_uint8p(s, *server_random, random_len);

    /* RSA info */
    end = s->p + rsa_info_len;
    if (end > s->end)
        return false;

    in_uint8s(s, 12);    /* unknown */

    while (s->p < end) {
        in_uint16_le(s, tag);
        in_uint16_le(s, length);

        next_tag = s->p + length;

        switch (tag) {
            case SEC_TAG_PUBKEY:
                if (!sec_parse_public_key(s, modulus, exponent))
                    return false;

                break;

            case SEC_TAG_KEYSIG:
                /* Is this a Microsoft key that we just got? */
                /* Care factor: zero! */
                break;

            default:
                throw not_implemented_error{};
        }

        s->p = next_tag;
    }

    return s_check_end(s);
}


/* Generate a 32-byte random for the secure transport code. */
void Client::generate_random(uint8 *random) {
    for (int i = 0; i < SEC_RANDOM_SIZE; ++i) {
        random[i] = (uint8) rand();
    }
}

/* Process crypto information blob */
void Client::sec_process_crypt_info(STREAM s) {
    uint8 *server_random, *modulus, *exponent;
    uint8 client_random[SEC_RANDOM_SIZE];
    uint32 rc4_key_size;

    if (!sec_parse_crypt_info(s, &rc4_key_size, &server_random, &modulus, &exponent))
        return;

    /* Generate a client random, and hence determine encryption keys */
    generate_random(client_random);
    sec_rsa_encrypt(sec_crypted_random, client_random, SEC_RANDOM_SIZE, modulus, exponent);
    sec_generate_keys(client_random, server_random, rc4_key_size);
}

/* Process connect response data blob */
void Client::sec_process_mcs_data(STREAM s) {
    uint16 tag, length;
    uint8 *next_tag;
    uint8 len;

    in_uint8s(s, 21);    /* header */
    in_uint8(s, len);
    if (len & 0x80)
        in_uint8(s, len);

    while (s->p < s->end) {
        in_uint16_le(s, tag);
        in_uint16_le(s, length);

        if (length <= 4)
            return;

        next_tag = s->p + length - 4;

        switch (tag) {
            case SEC_TAG_SRV_INFO:
            case SEC_TAG_SRV_3:
                break;

            case SEC_TAG_SRV_CRYPT:
                sec_process_crypt_info(s);
                break;

            default:
                throw not_implemented_error{};
        }

        s->p = next_tag;
    }
}

/* Receive secure transport packet */
STREAM Client::sec_recv() {
    uint32 sec_flags;
    STREAM s;

    while ((s = mcs_recv()) != nullptr) {
        if (encryption || !licence_issued) {
            in_uint32_le(s, sec_flags);

            if (sec_flags & SEC_LICENCE_NEG) {
                licence_process(s);
                continue;
            }

            if (sec_flags & SEC_ENCRYPT) {
                in_uint8s(s, 8);    /* signature */
                sec_decrypt(s->p, s->end - s->p);
            }
        }

        return s;
    }

    return nullptr;
}

/* Establish a secure connection */
BOOL Client::sec_connect(char *server) {
    struct stream mcs_data;

    /* We exchange some RDP data during the MCS-Connect */
    mcs_data.size = 512;
    mcs_data.p = mcs_data.data = new uint8[mcs_data.size];
    sec_out_mcs_data(&mcs_data);

    if (!mcs_connect(server, &mcs_data))
        return false;

    sec_process_mcs_data(&mcs_data);
    if (encryption)
        sec_establish_key();
    delete[]mcs_data.data;
    return true;
}

/* Disconnect a connection */
void Client::sec_disconnect() {
    mcs_disconnect();
}

XWin_Ui *Client::getUi() {
    return xwin_ui;
}

Client::~Client() {
    rdp_disconnect();
    delete[]pad_54;
    delete[]pad_92;
}

int Client::getminkeycode() const {
    return min_keycode;
}


