#include "processdockspace.h"
#include "ui_processdockspace.h"
#include <QLayout>

ProcessDockspace::ProcessDockspace(QWidget* parent, const nxdb::Process& process)
    : QMainWindow(parent)
    , ui(new Ui::ProcessDockspace)
    , mProcess(process)
    , mModuleList(new ModuleList(this, mProcess))
    , mMemory(new Memory(this, mProcess)) {
    ui->setupUi(this);

    setDockOptions(DockOption::AnimatedDocks | DockOption::AllowNestedDocks | DockOption::AllowTabbedDocks);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, mModuleList);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, mMemory);

    statusBar()->showMessage("hi");

#define CONNECT_DOCK(NAME)                                                               \
    ui->actionView##NAME->setChecked(true);                                            \
    connect(ui->actionView##NAME, &QAction::toggled, m##NAME, [this](bool checked) {     \
        m##NAME->setVisible(checked && !m##NAME->isFloating());                      \
    });                                                                                  \
    connect(m##NAME, &QDockWidget::visibilityChanged, this, [this](bool visible) {       \
        if (!m##NAME->isFloating() && !visible != ui->actionView##NAME->isChecked()) \
            ui->actionView##NAME->setChecked(visible);                                 \
    })

    CONNECT_DOCK(ModuleList);
    CONNECT_DOCK(Memory);

    setWindowTitle(
        QString("nxdb: Debugging %1, PID: %2")
            .arg(mProcess.processName.c_str())
            .arg(mProcess.processId));
}

ProcessDockspace::~ProcessDockspace() {
    delete ui;
}
