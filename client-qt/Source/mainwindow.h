#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "pe/Enet/Packets/DataPackets.h"
#include <QItemSelectionModel>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void getProcesses();
    void updateProcessTable();
    void updateProcessTableSelection();

private slots:
    void on_buttonUpdateProcessList_clicked();
    void on_buttonDebugSelectedProcess_clicked();

    void on_processTable_pressed(const QModelIndex &index);

private:
    Ui::MainWindow* ui;
    QStandardItemModel* mTableModel;
    pe::enet::ProcessList_::Response mProcessList;
};
#endif // MAINWINDOW_H
