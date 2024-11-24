#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/Packets/DataPackets.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    pe::enet::getNetClient()->makeRequest<pe::enet::ProcessList>({}, [](pe::enet::ProcessList_::Response* response) {
        printf("Processes: %d\n", response->num);
        for (int i = 0; i < response->num; i++)
        {
            auto& p = response->processes[i];
            printf("  %s:\n", p.name);
            printf("    ProcessId: %ld:\n", p.processId);
            printf("    ProgramId: %016lX:\n", p.programId);
        }
    });
}

