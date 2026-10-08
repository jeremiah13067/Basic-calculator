#pragma once

#include <QPushButton>

// A keypad button. Its look comes from the "kind" property (see Theme.h), and
// its text size follows the button size so the keypad scales with the window.
class CalcButton : public QPushButton {
public:
    enum class Kind { Digit, Action, Operator, Equals };

    CalcButton(const QString& text, Kind kind, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
};