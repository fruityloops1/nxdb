#ifndef MODULELIST_H
#define MODULELIST_H

#include <QDockWidget>
#include "nxdb/Process.h"
#include <QMainWindow>

namespace Ui {
    class ModuleList;
}

class ModuleList : public QDockWidget {
    Q_OBJECT

public:
    explicit ModuleList(QMainWindow* parent, const nxdb::Process& process);
    ~ModuleList();

private:
    Ui::ModuleList* ui;
    const nxdb::Process& mProcess;
};

#endif // MODULELIST_H
