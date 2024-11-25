#include "memory.h"
#include "ui_memory.h"
#include "nxdb/Util.h"

Memory::Memory(QMainWindow* parent)
    : QDockWidget(parent)
    , ui(new Ui::Memory) {
    ui->setupUi(this);

    ui->table->setFont(QFont("Monospace"));
    ui->table->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->table->setShowGrid(false);
    ui->table->setColumnCount(0x10);

    mRowHeight = 22;
    mRowWidth = 27;
    printf("mRowHeight %d\n",mRowHeight);
}

Memory::~Memory() {
    delete ui;
}

void Memory::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);

    int numRowsFit = ui->table->height() / mRowHeight;
    int numColsFit = (ui->table->width()-78) / mRowWidth;
    numColsFit -= numColsFit % 0x10;
    if (numColsFit < 0x10)
        numColsFit = 0x10;

    ui->table->clear();
    ui->table->setRowCount(numRowsFit);
    ui->table->setColumnCount(numColsFit);

    for (int x = 0; x < numColsFit; x++)
        for (int y = 0; y < numRowsFit; y++)
        {
            auto* item = new QTableWidgetItem(QString("00"));
            ui->table->setItem(y, x, item);
            item->setTextAlignment(Qt::AlignCenter);
        }

    QStringList horizontalIdxList;
    for (int i = 0; i < numColsFit; i++)
        horizontalIdxList.append(nxdb::util::hexStrDigit(i, 2));
    ui->table->setHorizontalHeaderLabels(horizontalIdxList);

    QStringList verticalAddrList;
    for (int i = 0; i < numRowsFit; i++)
        verticalAddrList.append(nxdb::util::hexStrDigit(mRegionAddress + numColsFit * i, 8));
    ui->table->setVerticalHeaderLabels(verticalAddrList);

    ui->table->resizeColumnsToContents();
    ui->table->resizeRowsToContents();
}

void Memory::on_table_itemSelectionChanged()
{
    //printf("asdofihjjsdfuojkgdsh\n");
    ui->table->selectionModel()->clearSelection();
}


void Memory::on_table_cellEntered(int row, int column)
{
}

