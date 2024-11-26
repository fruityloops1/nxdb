#ifndef MEMORYEDIT_H
#define MEMORYEDIT_H

#include <QLayoutItem>
#include <QWidget>

class MemoryEdit : public QWidget, public QLayoutItem {
    Q_OBJECT
public:
    explicit MemoryEdit(QWidget* parent = nullptr);

    QWidget* widget() const override { return const_cast<MemoryEdit*>(this); } // ???????????

    QSize sizeHint() const override { return { -1, -1 }; }
    QSize minimumSize() const override { return { 200, 150 }; }
    QSize maximumSize() const override { return { 20000, 20000 }; }
    Qt::Orientations expandingDirections() const override { return Qt::Orientation::Horizontal; }
    void setGeometry(const QRect& rect) override { QWidget::setGeometry(rect); }
    QRect geometry() const override { return QWidget::geometry(); }
    bool isEmpty() const override { return false; }

    void drawMemory(QPainter& painter);
    void drawAddrHeader(QPainter& painter);

    QRect getBox() const {
        const auto& geometry = MemoryEdit::geometry();
        QRect box(0, geometry.y(), geometry.width(), geometry.height());
        return box;
    }

    QRect getMemoryBox() const {
        QRect box = getBox();
        const QSize headerSize = calcAddrHeaderSize();
        box.setWidth(box.width() - headerSize.width());
        box.setX(box.x() + headerSize.width());
        return box;
    }

    QSize calcPaddingSize() const { return { mCharWidth, mCharHeight / 7 }; }
    QSize calcCellSize() const {
        const QSize paddingSize = calcPaddingSize();
        return { mCharWidth + paddingSize.width(), mCharHeight + paddingSize.height() };
    }

    QSize calcCellResolution() const {
        const QSize cellSize = calcCellSize();
        const QRect box = getMemoryBox();

        int width = box.right() / cellSize.width();
        width -= width % 8;

        return { width < 8 ? 8 : width, box.height() / cellSize.height() + 1 };
    }

    QSize calcAddrHeaderSize() const {
        const QSize paddingSize = calcPaddingSize();
        const QRect box = getBox();
        return { mCharWidth * 10 + paddingSize.width(), box.height() };
    }

    // actual event doesnt work in this widget idk why
    void onWheelEvent(QWheelEvent* event);

signals:

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QFont mMonospaceFont;
    int mCharWidth = 0;
    int mCharHeight = 0;
    uintptr_t mRegionAddress = 0x1230;
};

#endif // MEMORYEDIT_H
