#include "memoryedit.h"
#include <QTimer>
#include <QPainter>
#include <QStyleOption>
#include "nxdb/Util.h"

MemoryEdit::MemoryEdit(QWidget* parent)
    : QWidget { parent } {
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MemoryEdit::update));
    timer->start(1000);

    mMonospaceFont = {"Fira Code"};
    mMonospaceFont.setStyleHint(QFont::Monospace);

    QFontMetrics metrics(mMonospaceFont);
    mCharWidth = metrics.horizontalAdvance('0');
    mCharHeight = metrics.height();
}

void MemoryEdit::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setFont(mMonospaceFont);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRect box = getBox();

    const QColor bgColor(palette().color(QPalette::Base));
    const QColor edgeColor(palette().color(QPalette::Dark));

    painter.setPen(edgeColor);
    painter.setBrush(bgColor);
    painter.drawRect(box);

    drawAddrHeader(painter);
    drawMemory(painter);
}

void MemoryEdit::drawMemory(QPainter& painter)
{
    const QRect box = getMemoryBox();
    const QColor textColor(palette().color(QPalette::Text));

    const QSize cellSize = calcCellSize();
    const QSize res = calcCellResolution();

    painter.setPen(textColor);
    for (int x = 0; x < res.width(); x++)
        for (int y = 0; y < res.height(); y++)
        {
            QRect cell(box.x() + cellSize.width() * x, box.y() + cellSize.height() * y, cellSize.width(), cellSize.height());
            painter.drawText(cell, Qt::AlignCenter, "00");
        }
}

void MemoryEdit::drawAddrHeader(QPainter& painter)
{
    const QColor textColor(palette().color(QPalette::Text));
    const QRect box = getBox();
    const QSize headerSize = calcAddrHeaderSize();
    const QSize res = calcCellResolution();
    const QSize cellSize = calcCellSize();

    QStyleOptionHeader option;
    option.initFrom(this);
    option.state = QStyle::State_Active | QStyle::State_Enabled;
    option.orientation = Qt::Horizontal;

    painter.save();
    for (int y = 0; y < res.height(); y++)
    {
        QRect cell(box.x(), box.y() + cellSize.height() * y, headerSize.width(), cellSize.height());
        option.rect = cell;
        style()->drawControl(QStyle::CE_Header, &option, &painter, this);
    }
    painter.restore();

    painter.setPen(textColor);

    for (int y = 0; y < res.height(); y++)
    {
        QRect cell(box.x(), box.y() + cellSize.height() * y, headerSize.width(), cellSize.height());

        painter.drawText(cell, Qt::AlignCenter, nxdb::util::hexStrDigit(mRegionAddress + res.width() * y, 10));
    }
}
