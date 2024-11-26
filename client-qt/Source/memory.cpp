#include "memory.h"
#include "nxdb/Util.h"
#include "ui_memory.h"
#include <QWheelEvent>

Memory::Memory(QMainWindow* parent)
    : QDockWidget(parent)
    , ui(new Ui::Memory)
    , mMemoryEdit(new MemoryEdit(this)) {
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
