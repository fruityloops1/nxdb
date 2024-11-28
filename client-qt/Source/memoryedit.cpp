#include "memoryedit.h"
#include "nxdb/Util.h"
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QWheelEvent>
#include "pe/Enet/NetClient.h"

MemoryEdit::MemoryEdit(QWidget* parent, const nxdb::Process& process)
    : QWidget { parent }, mProcess(process) {
    // QTimer* timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, QOverload<>::of(&MemoryEdit::update));
    // timer->start(1000);

    printf("MR PROCESS ID %zu\n", mProcess.sessionId);
    mMonospaceFont = { "Fira Code" };
    mMonospaceFont.setStyleHint(QFont::Monospace);

    QFontMetrics metrics(mMonospaceFont);
    mCharWidth = metrics.horizontalAdvance('0');
    mCharHeight = metrics.height();
    setMouseTracking(true);

    printf("Initial sub id: %zu\n", mSubscriptionId);
    mRegionAddress = mProcess.modules[0].base;
    updateSubscription();
}

void MemoryEdit::updateSubscription()
{
    const QSize res = calcCellResolution();

    printf("send sub id: %zu MR PROCESS ID %zu\n", mSubscriptionId, mProcess.sessionId);
    pe::enet::EditSubscription_::Request request;
    request.sessionId = mProcess.sessionId;
    request.subscriptionId = mSubscriptionId;
    request.addr = mRegionAddress;
    request.frequencyHz = 30;
    request.size = res.width() * res.height();


    printf("mIsRequesting %d mIsQueuedRequest %d\n", mIsRequesting, mIsQueuedRequest);

    /*if (mIsRequesting)
    {
        mIsQueuedRequest = true;
        mQueuedRequest = request;
    } else {
        mIsRequesting = true;*/
        pe::enet::getNetClient()->makeRequest<pe::enet::EditSubscription>(request, [this](pe::enet::EditSubscription_::Response* response) {
            printf("received sub id: %zu\n", response->subscriptionId);
            mSubscriptionId = response->subscriptionId;

            /*mIsRequesting = false;
            if (mIsQueuedRequest)
            {
                updateSubscription();
                mIsQueuedRequest = false;
            }*/
        });
    //}
}

void MemoryEdit::paintEvent(QPaintEvent* event) {
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

void MemoryEdit::onWheelEvent(QWheelEvent* event) {
    int rows = event->angleDelta().y() / 120;
    mRegionAddress += -rows * calcCellResolution().width();

    updateSubscription();
    repaint();
}

void MemoryEdit::drawMemory(QPainter& painter) {
    const QRect box = getMemoryBox();
    const QColor textColor(palette().color(QPalette::Text));

    const QSize cellSize = calcCellSize();
    const QSize res = calcCellResolution();

    painter.setPen(textColor);
    for (int x = 0; x < res.width(); x++)
        for (int y = 0; y < res.height(); y++) {
            QRect cell(box.x() + cellSize.width() * x, box.y() + cellSize.height() * y, cellSize.width(), cellSize.height());
            painter.drawText(cell, Qt::AlignCenter, "00");
        }
}

void MemoryEdit::drawAddrHeader(QPainter& painter) {
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
    for (int y = 0; y < res.height(); y++) {
        QRect cell(box.x(), box.y() + cellSize.height() * y, headerSize.width(), cellSize.height());
        option.rect = cell;
        style()->drawControl(QStyle::CE_Header, &option, &painter, this);
    }
    painter.restore();

    painter.setPen(textColor);

    for (int y = 0; y < res.height(); y++) {
        QRect cell(box.x(), box.y() + cellSize.height() * y, headerSize.width(), cellSize.height());

        painter.drawText(cell, Qt::AlignCenter, nxdb::util::hexStrDigit(mRegionAddress + res.width() * y, 10));
    }
}
