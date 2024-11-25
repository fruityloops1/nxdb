#ifndef PROCESSDOCKSPACE_H
#define PROCESSDOCKSPACE_H

#include <QMainWindow>

namespace Ui {
    class ProcessDockspace;
}

class ProcessDockspace : public QMainWindow {
    Q_OBJECT

public:
    explicit ProcessDockspace(QWidget* parent = nullptr);
    ~ProcessDockspace();

private:
    Ui::ProcessDockspace* ui;
};

#endif // PROCESSDOCKSPACE_H
