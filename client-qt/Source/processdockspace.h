#ifndef PROCESSDOCKSPACE_H
#define PROCESSDOCKSPACE_H

#include <QMainWindow>
#include "nxdb/Process.h"
#include "modulelist.h"
#include "memory.h"

namespace Ui {
    class ProcessDockspace;
}

class ProcessDockspace : public QMainWindow {
    Q_OBJECT

public:
    explicit ProcessDockspace(QWidget* parent, const nxdb::Process& process);
    ~ProcessDockspace();

private:
    Ui::ProcessDockspace* ui;
    const nxdb::Process& mProcess;
    ModuleList* mModuleList;
    Memory* mMemory;
};

#endif // PROCESSDOCKSPACE_H
