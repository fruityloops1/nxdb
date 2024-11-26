#include "memory.h"
#include "nxdb/Util.h"
#include "ui_memory.h"
#include <QWheelEvent>

Memory::Memory(QMainWindow* parent, const nxdb::Process& process)
    : QDockWidget(parent)
    , ui(new Ui::Memory)
    , mMemoryEdit(new MemoryEdit(this, process))
    , mProcess(process) {
    ui->setupUi(this);
    ui->verticalLayout->addItem(mMemoryEdit);

    setMouseTracking(true);
}

Memory::~Memory() {
    delete ui;
}

void Memory::resizeEvent(QResizeEvent* event) {
    QDockWidget::resizeEvent(event);
}

void Memory::wheelEvent(QWheelEvent* event) {
    mMemoryEdit->onWheelEvent(event);
}
