#ifndef MEMORY_H
#define MEMORY_H

#include "memoryedit.h"
#include <QDockWidget>
#include <QMainWindow>
#include "nxdb/Process.h"

namespace Ui {
    class Memory;
}

class Memory : public QDockWidget {
    Q_OBJECT

public:
    explicit Memory(QMainWindow* parent, const nxdb::Process& process);
    ~Memory();

    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
private:
    Ui::Memory* ui;
    MemoryEdit* mMemoryEdit;
    const nxdb::Process& mProcess;
};

#endif // MEMORY_H
