/*
   rdesktop: A Remote Desktop Protocol client.
   Protocol services - TCP layer
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

#include <cerrno>        /* errno */
#include <QTcpSocket>
#include <QHostInfo>
#include "tcp.h"
#include <iostream>

void info(const char *format, ...);

TcpTool::TcpTool() {
    sock = new QTcpSocket();
    tcp_port_rdp = TCP_PORT_RDP;
}

/* Initialise TCP transport data packet */
STREAM TcpTool::tcp_init(int maxlen) {
    if (maxlen > out.size) {
        delete out.data;
        out.data = new unsigned char[maxlen];
        out.size = maxlen;
    }

    out.p = out.data;
    out.end = out.data + out.size;
    return &out;
}

/* Send TCP transport data packet */
void TcpTool::tcp_send(STREAM s) {
    int length = s->end - s->data;
    int sent, total = 0;

    while (total < length) {
        sent = sock->write((char *) (s->data + total), length - total);
        if (sent <= 0) {
            info("send: %s", strerror(errno));
            exit(-1);
        }
        total += sent;
    }

    if (!sock->waitForBytesWritten(3000)) {
        info("Error: waitForBytesWritten Failed.");
    }
}

/* Receive a message on the TCP layer */
STREAM TcpTool::tcp_recv(unsigned length) {
    int rcvd = 0;
    if (length > in.size) {
        delete in.data;
        in.data = new unsigned char[length];
        in.size = length;
    }
    in.end = in.p = in.data;
    while (length > 0) {
        if (sock->bytesAvailable() <= 0) {
            sock->waitForReadyRead(-1);
        }
        rcvd = sock->read((char *) in.end, length);
        if (rcvd <= 0) {
            info("recv: %d: %s", errno, strerror(errno));
        }
        in.end += rcvd;
        length -= rcvd;
    }
    return &in;
}

/* Establish a connection on the TCP layer */
BOOL TcpTool::tcp_connect(char *server) {
    QHostInfo host = QHostInfo::fromName(QString(server));
    sock->connectToHost(server, tcp_port_rdp);
    if (!sock->waitForConnected(30000)) {
        info("Error: WaitForConnected Failed. %s", sock->errorString().toStdString().c_str());
        exit(-1);
    }

    in.size = 4096;
    in.data = new uchar[in.size];

    out.size = 4096;
    out.data = new uchar[out.size];
    return true;
}

/* Disconnect on the TCP layer */
void TcpTool::tcp_disconnect(void) {
    sock->disconnectFromHost();
}

TcpTool::~TcpTool() {
    tcp_disconnect();
    delete sock;
}


bool TcpTool::get_ready() {
    bool ready = (sock->bytesAvailable() > 0);
    return ready;
}

void TcpTool::trynext() {
    sock->waitForReadyRead(0);
}



