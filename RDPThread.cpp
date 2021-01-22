//
// Created by User on 2021/1/22.
//

#include "rdpthread.h"
#include "client.h"
#include "xwin.h"
#include "tcp.h"

void info(const char *format, ...);

RDPThread::RDPThread(){
    client = NULL;
}

[[noreturn]] void RDPThread::run() {
    TcpTool * tcptool = new TcpTool();
    XWin_Ui * xwin_ui = new XWin_Ui(800, 600, 32);
    client = new Client(xwin_ui, tcptool);
    char server[256] = "192.168.192.128";
    uint32 flags = RDP_LOGON_NORMAL;
    char domain[256] = {0};
    char password[256] = "123456";
    char shell[256] = {0};
    char directory[256] = {0};
    if (!client->rdp_connect(server, flags, domain, password, shell, directory))
        exit(-1);
    memset(password, 0, sizeof(password));
    info("Initialize finished.");
    while (true){
        client->rdp_main_loop();
    }
}

Client *RDPThread::getClient() {
    return client;
}

RDPThread::~RDPThread() {
    delete client;
}

