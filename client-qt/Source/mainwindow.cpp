#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/Packets/DataPackets.h"
#include <QResizeEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    mTableModel = new QStandardItemModel();

    ui->processTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

MainWindow::~MainWindow() {
    delete ui;
    delete mTableModel;
}

void MainWindow::getProcesses()
{
    pe::enet::getNetClient()->makeRequest<pe::enet::ProcessList>({}, [this](pe::enet::ProcessList_::Response* response) {
        mProcessList = *response;
        updateProcessTable();
    });
}

void MainWindow::updateProcessTable()
{
    mTableModel->clear();

    mTableModel->setRowCount(mProcessList.num);
    mTableModel->setColumnCount(3);

    mTableModel->setHorizontalHeaderLabels({"Name", "Process ID", "Program ID"});

    for (int i = 0; i < mProcessList.num; i++)
    {
        auto& p = mProcessList.processes[i];
        QStandardItem* nameItem = new QStandardItem(QString(p.name));
        QStandardItem* processIdItem = new QStandardItem(QString("%1").arg(p.processId));
        QStandardItem* programIdItem = new QStandardItem(QString("%1").arg(p.programId, 16, 16, QChar('0')).toUpper());

        nameItem->setEditable(false);
        processIdItem->setEditable(false);
        programIdItem->setEditable(false);

        mTableModel->setItem(i, 0, nameItem);
        mTableModel->setItem(i, 1, processIdItem);
        mTableModel->setItem(i, 2, programIdItem);
    }

    ui->processTable->setModel(mTableModel);
    ui->processTable->resizeColumnsToContents();
    ui->processTable->resizeRowsToContents();
    ui->processTable->verticalHeader()->setVisible(false);
}

void MainWindow::updateProcessTableSelection()
{
    auto sel = ui->processTable->selectionModel()->selection();
    ui->buttonDebugSelectedProcess->setEnabled(sel.count() == 1);
}

void MainWindow::on_buttonUpdateProcessList_clicked()
{
    getProcesses();
}

void MainWindow::on_buttonDebugSelectedProcess_clicked()
{
    auto sel = ui->processTable->selectionModel()->selection();
    if (sel.count() == 1)
    {
        int rowIdx = sel.at(0).indexes().at(0).row();
        printf("In row %d\n", rowIdx);
        u64 processId = mProcessList.processes[rowIdx].processId;

        pe::enet::getNetClient()->makeRequest<pe::enet::StartDebugging>({processId}, [this](pe::enet::StartDebugging_::Response* response) {
            printf("Session ID: %zu\n", response->sessionId);

            pe::enet::getNetClient()->makeRequest<pe::enet::GetDebuggingSessionInfo>({response->sessionId}, [this](pe::enet::GetDebuggingSessionInfo_::Response* response) {
                printf("numModules %d\n", response->numModules);
                for (int i = 0; i< response->numModules; i++)
                {
                    auto& mod = response->modules[i];
                    printf("module %s\n", response->moduleNameStringTable + mod.nameOffsetIntoStringTable);
                    printf("module base %016lx\n", mod.base);
                    printf("module size %016lx\n", mod.size);
                }
            });
        });
    }
}

void MainWindow::on_processTable_pressed(const QModelIndex &index)
{
    updateProcessTableSelection();
}

