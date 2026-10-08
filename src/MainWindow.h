#pragma once

#include "CalcButton.h"
#include "CalculatorEngine.h"
#include "DisplayWidget.h"
#include "Theme.h"

#include <QChar>
#include <QHash>
#include <QString>
#include <QWidget>

#include <functional>

class QGridLayout;
class QKeyEvent;

class MainWindow : public QWidget {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    using Action = std::function<void(calc::CalculatorEngine&)>;

    void buildUi();
    CalcButton* addKey(QGridLayout* grid, int row, int column, const QString& label,
                       CalcButton::Kind kind, Action action, QChar key = QChar());
    void perform(const Action& action);
    void refresh();
    bool handleCharacter(QChar ch);
    void flash(CalcButton* button);
    void copyDisplay();
    void pasteText();

    Theme theme_;
    calc::CalculatorEngine engine_;
    DisplayWidget* display_ = nullptr;
    CalcButton* clearButton_ = nullptr;
    CalcButton* backspaceButton_ = nullptr;
    CalcButton* signButton_ = nullptr;
    QHash<int, CalcButton*> keyButtons_;  // typed character -> on-screen key to flash
};