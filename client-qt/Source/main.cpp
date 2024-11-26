#include "mainwindow.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/ProjectPacketHandler.h"
#include "pe/Enet/Types.h"

static void memoryFull() {
    fprintf(stderr, "balls");
    abort();
}

#include <QApplication>

constexpr u16 port = 3450;

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    ENetCallbacks callbacks { malloc, free, memoryFull };

    pe::enet::ProjectPacketHandler handler;
    pe::enet::NetClient client(&handler);
    pe::enet::setNetClient(&client);
    client.connect("192.168.188.151", port);

    MainWindow w;
    w.show();

    int rc = a.exec();

    client.disconnect();
    client.kill();
    client.join();

    return rc;
}
