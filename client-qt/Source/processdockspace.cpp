#include "processdockspace.h"
#include "ui_processdockspace.h"

ProcessDockspace::ProcessDockspace(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ProcessDockspace) {
    ui->setupUi(this);
}

ProcessDockspace::~ProcessDockspace() {
    delete ui;
}
