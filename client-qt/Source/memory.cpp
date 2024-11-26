#include "memory.h"
#include "ui_memory.h"
#include "nxdb/Util.h"

Memory::Memory(QMainWindow* parent)
    : QDockWidget(parent)
    , ui(new Ui::Memory)
    , mMemoryEdit(new MemoryEdit(this))
{
    ui->setupUi(this);
    ui->verticalLayout->addItem(mMemoryEdit);
}

Memory::~Memory() {
    delete ui;
}

void Memory::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);
}

