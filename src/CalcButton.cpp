#include "CalcButton.h"

#include <QFont>
#include <QResizeEvent>

#include <algorithm>

namespace {

const char* kindName(CalcButton::Kind kind)
{
    switch (kind) {
    case CalcButton::Kind::Digit:    return "digit";
    case CalcButton::Kind::Action:   return "action";
    case CalcButton::Kind::Operator: return "operator";
    case CalcButton::Kind::Equals:   return "equals";
    }
    return "digit";
}

}  // namespace

CalcButton::CalcButton(const QString& text, Kind kind, QWidget* parent)
    : QPushButton(text, parent)
{
    setProperty("kind", QString::fromLatin1(kindName(kind)));
    setFocusPolicy(Qt::NoFocus);  // keyboard input always goes to the main window
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // An explicit minimum stops the layout from growing the window's minimum
    // size when the font gets larger.
    setMinimumSize(52, 44);
}

void CalcButton::resizeEvent(QResizeEvent* event)
{
    QPushButton::resizeEvent(event);

    const int pixelSize = std::clamp(std::min(width(), height()) * 36 / 100, 14, 40);
    QFont f = font();
    if (f.pixelSize() != pixelSize) {
        f.setPixelSize(pixelSize);
        setFont(f);
    }
}