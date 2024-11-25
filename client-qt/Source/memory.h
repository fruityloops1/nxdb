#ifndef MEMORY_H
#define MEMORY_H

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

private slots:

    void on_table_itemSelectionChanged();

    void on_table_cellEntered(int row, int column);

private:
    Ui::Memory* ui;
    uintptr_t mRegionAddress = 0x00000069149F0000;
    int mRowHeight = 0;
    int mRowWidth = 0;
};

#endif // MEMORY_H
