#include "modulelist.h"
#include "nxdb/Util.h"
#include "ui_modulelist.h"
#include <QPushButton>
#include <QStandardItemModel>

ModuleList::ModuleList(QMainWindow* parent, const nxdb::Process& process)
    : ui(new Ui::ModuleList)
    , mProcess(process) {
    ui->setupUi(this);

    ui->moduleTree->setColumnCount(1);
    ui->moduleTree->setHeaderHidden(true);

    for (auto& mod : process.modules) {
        size_t pos = mod.name.rfind('\\');
        const char* moduleNameTrimmed = pos == std::string::npos ? mod.name.c_str() : mod.name.c_str() + pos + 1;

        auto* moduleEntry = new QTreeWidgetItem(ui->moduleTree);
        moduleEntry->setText(0, moduleNameTrimmed);
        moduleEntry->setToolTip(0, mod.name.c_str());

        auto* moduleRange = new QTreeWidgetItem(moduleEntry);
        moduleRange->setText(0, QString("%1-%2").arg(nxdb::util::hexStrDigit(mod.base)).arg(nxdb::util::hexStrDigit(mod.base + mod.size)));

        ui->moduleTree->addTopLevelItem(moduleEntry);
    }
}

ModuleList::~ModuleList() {
    delete ui;
}
