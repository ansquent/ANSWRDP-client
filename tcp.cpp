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

#include <errno.h>		/* errno */
#include "rdesktop.h"
#include <exception.h>
#include <QTcpSocket>
#include <QHostInfo>
#include <QDebug>

QTcpSocket * sock;
static struct stream in;
static struct stream out;
extern int tcp_port_rdp;

/* Initialise TCP transport data packet */
STREAM
tcp_init(int maxlen)
{
    if (maxlen > out.size)
    {
        out.data = (unsigned char *)xrealloc(out.data, maxlen);
        out.size = maxlen;
    }

    out.p = out.data;
    out.end = out.data + out.size;
    return &out;
}

/* Send TCP transport data packet */
void
tcp_send(STREAM s)
{
    int length = s->end - s->data;
    int sent, total = 0;

    while (total < length)
    {
        sent = sock->write((char *)(s->data + total), length - total);
        if (sent <= 0)
        {
            error("send: %s", strerror(errno));
        }
        total += sent;
    }

    if (!sock->waitForBytesWritten(3000)){
        error("Error: waitForBytesWritten Failed.");
    }
}

/* Receive a message on the TCP layer */
STREAM
tcp_recv(unsigned length)
{
    int rcvd = 0;
    if (length > in.size)
    {
        in.data = (unsigned char *)xrealloc(in.data, length);
        in.size = length;
    }
    in.end = in.p = in.data;
    while (length > 0)
    {
        if (sock->bytesAvailable() <= 0){
            if (!sock->waitForReadyRead(3000)){
                warning("Error: waitForReadyRead Failed.");
                return nullptr;
            }
        }
        rcvd = sock->read((char *)in.end, length);
        if (rcvd <= 0)
        {
            error("recv: %s", strerror(errno));
        }
        in.end += rcvd;
        length -= rcvd;
    }
    return &in;
}

/* Establish a connection on the TCP layer */
BOOL
tcp_connect(char *server)
{
    QHostInfo host = QHostInfo::fromName(QString(server));
    sock->connectToHost(server, tcp_port_rdp);
    if (!sock->waitForConnected(30000)){
        qDebug() << sock->errorString();
        error("Error: WaitForConnected Failed.");
    }

    in.size = 4096;
    in.data = (unsigned char *)xmalloc(in.size);

    out.size = 4096;
    out.data = (unsigned char *)xmalloc(out.size);
    return True;
}

/* Disconnect on the TCP layer */
void
tcp_disconnect(void)
{
    sock->disconnectFromHost();
}
