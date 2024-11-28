#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "nxdb/Util.h"
#include "pe/Enet/NetClient.h"
#include "pe/Enet/Packets/DataPackets.h"

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

    for (ProcessDockspace* dock : mDebuggedProcessList)
        delete dock;
}

void MainWindow::getProcesses() {
    pe::enet::getNetClient()->makeRequest<pe::enet::ProcessList>({}, [this](pe::enet::ProcessList_::Response* response) {
        mProcessList = *response;
        updateProcessTable();
    });
}

void MainWindow::updateProcessTable() {
    mTableModel->clear();

    mTableModel->setRowCount(mProcessList.num);
    mTableModel->setColumnCount(3);

    mTableModel->setHorizontalHeaderLabels({ "Name", "Process ID", "Program ID" });

    for (int i = 0; i < mProcessList.num; i++) {
        auto& p = mProcessList.processes[i];
        QStandardItem* nameItem = new QStandardItem(QString(p.name));
        QStandardItem* processIdItem = new QStandardItem(QString("%1").arg(p.processId));
        QStandardItem* programIdItem = new QStandardItem(nxdb::util::hexStrDigit(p.programId));

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

void MainWindow::updateProcessTableSelection() {
    auto sel = ui->processTable->selectionModel()->selection();
    ui->buttonDebugSelectedProcess->setEnabled(sel.count() == 1);
}

void MainWindow::on_buttonUpdateProcessList_clicked() {
    getProcesses();
}

void MainWindow::debugProcess(int row) {
    printf("In row %d\n", row);
    u64 processId = mProcessList.processes[row].processId;

    pe::enet::getNetClient()->makeRequest<pe::enet::StartDebugging>({ processId }, [this, processId](pe::enet::StartDebugging_::Response* response) {
        printf("Session ID: %zu\n", response->sessionId);

        if (response->sessionId == 0)
        {
            statusBar()->showMessage(QString("Debugging process %1 failed").arg(processId));
            return;
        }

        auto sessionId = response->sessionId;

        pe::enet::getNetClient()->makeRequest<pe::enet::GetDebuggingSessionInfo>({ response->sessionId }, [this, sessionId](pe::enet::GetDebuggingSessionInfo_::Response* response) {
            nxdb::Process process;
            process.processId = response->processId;
            process.programId = response->programId;
            process.processName = response->processName;
            process.sessionId = sessionId;

            for (int i = 0; i < response->numModules; i++) {
                auto& mod = response->modules[i];
                process.modules.push_back({ mod.base, mod.size, response->moduleNameStringTable + mod.nameOffsetIntoStringTable });
            }

            printf("Num Modules: %d Name: %s\n", response->numModules, response->processName);
            statusBar()->showMessage(QString("Now debugging %1").arg(process.processName.c_str()));

            QMetaObject::invokeMethod(this, "createProcessDockspace", Qt::QueuedConnection, Q_ARG(nxdb::Process, process));
        });
    });
}

void MainWindow::on_buttonDebugSelectedProcess_clicked() {
    auto sel = ui->processTable->selectionModel()->selection();
    if (sel.count() == 1) {
        int rowIdx = sel.at(0).indexes().at(0).row();
        debugProcess(rowIdx);
    }
}

void MainWindow::createProcessDockspace(const nxdb::Process& process) {
    auto* window = new ProcessDockspace(this, process);
    window->show();
    mDebuggedProcessList.push_back(window);
}

void MainWindow::on_processTable_pressed(const QModelIndex& index) {
    updateProcessTableSelection();
}

void MainWindow::on_processTable_doubleClicked(const QModelIndex& index) {
    debugProcess(index.row());
}
