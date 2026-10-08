#include "DisplayWidget.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

#include <algorithm>

DisplayWidget::DisplayWidget(const Theme& theme, QWidget* parent)
    : QWidget(parent), theme_(theme)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(96);
}

void DisplayWidget::setContent(const QString& primary, const QString& secondary, bool isError)
{
    if (primary == primary_ && secondary == secondary_ && isError == isError_)
        return;
    primary_ = primary;
    secondary_ = secondary;
    isError_ = isError;
    update();
}

QSize DisplayWidget::sizeHint() const { return QSize(340, 150); }
QSize DisplayWidget::minimumSizeHint() const { return QSize(240, 96); }

void DisplayWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(theme_.displayFill);
    painter.drawRoundedRect(rect(), 18, 18);

    const QRect area = rect().adjusted(20, 14, -20, -14);

    // Small line on top.
    QFont secondaryFont = font();
    secondaryFont.setPixelSize(std::clamp(height() / 8, 13, 22));
    const QFontMetrics secondaryMetrics(secondaryFont);
    const QRect secondaryRect(area.left(), area.top(), area.width(), secondaryMetrics.height());
    painter.setFont(secondaryFont);
    painter.setPen(theme_.displayMuted);
    painter.drawText(secondaryRect, Qt::AlignRight | Qt::AlignVCenter,
                     secondaryMetrics.elidedText(secondary_, Qt::ElideLeft, area.width()));

    // Large line: start big, shrink to fit the width.
    const int minSize = 16;
    int size = std::clamp(height() * 40 / 100, 28, 64);
    QFont primaryFont = font();
    primaryFont.setWeight(QFont::Light);
    primaryFont.setPixelSize(size);
    const int textWidth = QFontMetrics(primaryFont).horizontalAdvance(primary_);
    if (textWidth > area.width()) {
        size = std::max(minSize, size * area.width() / textWidth);
        primaryFont.setPixelSize(size);
        while (size > minSize && QFontMetrics(primaryFont).horizontalAdvance(primary_) > area.width()) {
            --size;
            primaryFont.setPixelSize(size);
        }
    }

    const QFontMetrics primaryMetrics(primaryFont);
    const QRect primaryRect(area.left(), secondaryRect.bottom() + 1, area.width(),
                            area.bottom() - secondaryRect.bottom());
    painter.setFont(primaryFont);
    painter.setPen(isError_ ? theme_.displayError : theme_.displayText);
    painter.drawText(primaryRect, Qt::AlignRight | Qt::AlignVCenter,
                     primaryMetrics.elidedText(primary_, Qt::ElideLeft, area.width()));
}