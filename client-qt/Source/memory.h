#ifndef MEMORY_H
#define MEMORY_H

#include "memoryedit.h"
#include <QDockWidget>
#include <QMainWindow>

namespace Ui {
    class Memory;
}

class Memory : public QDockWidget {
    Q_OBJECT

public:
    explicit Memory(QMainWindow* parent);
    ~Memory();

    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
private:
    Ui::Memory* ui;
    MemoryEdit* mMemoryEdit;
};

#endif // MEMORY_H
