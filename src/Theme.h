#pragma once

#include <QColor>
#include <QString>

struct KeyColors {
    QColor base;
    QColor hover;
    QColor pressed;
    QColor border;
    QColor text;
};

// Every colour in the app. The key style sheet and the custom-painted display
// both read from here, so a new look only needs edits in this file.
struct Theme {
    QColor window       = QColor(0xE4, 0xE7, 0xEC);
    QColor displayFill  = QColor(0x1F, 0x24, 0x30);
    QColor displayText  = QColor(0xF3, 0xF5, 0xF8);
    QColor displayMuted = QColor(0x8F, 0x98, 0xAA);
    QColor displayError = QColor(0xFF, 0x8A, 0x7A);

    KeyColors digits = {QColor(0xFF, 0xFF, 0xFF), QColor(0xF2, 0xF4, 0xF7), QColor(0xDD, 0xE1, 0xE8),
                        QColor(0xD3, 0xD8, 0xE0), QColor(0x1F, 0x24, 0x30)};
    KeyColors actions = {QColor(0xCB, 0xD1, 0xDA), QColor(0xBF, 0xC6, 0xD1), QColor(0xAF, 0xB7, 0xC4),
                         QColor(0xCB, 0xD1, 0xDA), QColor(0x1F, 0x24, 0x30)};
    KeyColors operators = {QColor(0x3A, 0x41, 0x52), QColor(0x4A, 0x52, 0x66), QColor(0x2B, 0x31, 0x3F),
                           QColor(0x3A, 0x41, 0x52), QColor(0xF3, 0xF5, 0xF8)};
    KeyColors equals = {QColor(0xEF, 0x4F, 0x1E), QColor(0xF4, 0x6A, 0x3E), QColor(0xD6, 0x3F, 0x12),
                        QColor(0xEF, 0x4F, 0x1E), QColor(0xFF, 0xFF, 0xFF)};

    QString styleSheet() const
    {
        return keyRule(QStringLiteral("digit"), digits) + keyRule(QStringLiteral("action"), actions)
             + keyRule(QStringLiteral("operator"), operators) + keyRule(QStringLiteral("equals"), equals);
    }

private:
    static QString keyRule(const QString& kind, const KeyColors& c)
    {
        return QStringLiteral(
                   "QPushButton[kind=\"%1\"] {"
                   " background-color: %2; color: %6; border: 1px solid %5; border-radius: 12px; }"
                   "QPushButton[kind=\"%1\"]:hover { background-color: %3; }"
                   "QPushButton[kind=\"%1\"]:pressed { background-color: %4; }")
            .arg(kind, c.base.name(), c.hover.name(), c.pressed.name(), c.border.name(), c.text.name());
    }
};