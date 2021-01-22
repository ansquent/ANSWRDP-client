#ifndef TCP_H
#define TCP_H

#include <errno.h>        /* errno */
#include "constants.h"
#include <QTcpSocket>
#include <QHostInfo>

class TcpTool: public QObject {
Q_OBJECT
private:
    QTcpSocket *sock;
    struct stream in;
    struct stream out;
    int tcp_port_rdp;

public:
    TcpTool();

    virtual ~TcpTool();

    STREAM tcp_init(int maxlen);

    BOOL tcp_connect(char *server);

    void tcp_disconnect(void);

    STREAM tcp_recv(unsigned length);

    void tcp_send(STREAM s);

    bool get_ready();

    void trynext();
};

#endif
