#include "MainWindow.h"

#include <QClipboard>
#include <QGridLayout>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QPalette>
#include <QTimer>
#include <QVBoxLayout>

namespace {

using Engine = calc::CalculatorEngine;
using Op = Engine::Operator;

// The engine works in plain ASCII; the screen shows the proper symbols
// (multiplication sign, division sign, minus sign).
QString toDisplayText(const std::string& ascii)
{
    QString text = QString::fromStdString(ascii);
    text.replace(QLatin1Char('*'), QChar(0x00D7));
    text.replace(QLatin1Char('/'), QChar(0x00F7));
    text.replace(QLatin1Char('-'), QChar(0x2212));
    return text;
}

// "C" clears the number being typed; when there is none the key reads "AC".
void clearPressed(Engine& engine)
{
    if (engine.canClearEntry())
        engine.clearEntry();
    else
        engine.clearAll();
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Calculator"));
    setMinimumSize(300, 460);
    setFocusPolicy(Qt::StrongFocus);
    buildUi();
    refresh();
    resize(380, 620);
}

void MainWindow::buildUi()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, theme_.window);
    setPalette(pal);
    setAutoFillBackground(true);
    setStyleSheet(theme_.styleSheet());

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    display_ = new DisplayWidget(theme_, this);
    root->addWidget(display_, 2);

    auto* grid = new QGridLayout;
    grid->setSpacing(10);
    for (int column = 0; column < 4; ++column)
        grid->setColumnStretch(column, 1);
    for (int row = 0; row < 5; ++row)
        grid->setRowStretch(row, 1);
    root->addLayout(grid, 5);

    using Kind = CalcButton::Kind;

    // Row 0:  AC  <x]  %  /
    clearButton_ = addKey(grid, 0, 0, QStringLiteral("AC"), Kind::Action, clearPressed);
    backspaceButton_ = addKey(grid, 0, 1, QString(QChar(0x232B)), Kind::Action,
                              [](Engine& e) { e.backspace(); });
    addKey(grid, 0, 2, QStringLiteral("%"), Kind::Action,
           [](Engine& e) { e.inputPercent(); }, QLatin1Char('%'));
    addKey(grid, 0, 3, QString(QChar(0x00F7)), Kind::Operator,
           [](Engine& e) { e.inputOperator(Op::Divide); }, QLatin1Char('/'));

    // Rows 1-3: digits 7-9, 4-6, 1-3 and the operators * - +
    auto addDigit = [this, grid](int row, int column, int digit) {
        addKey(grid, row, column, QString::number(digit), Kind::Digit,
               [digit](Engine& e) { e.inputDigit(digit); },
               QChar::fromLatin1(static_cast<char>('0' + digit)));
    };
    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 3; ++column)
            addDigit(row + 1, column, 7 - 3 * row + column);

    addKey(grid, 1, 3, QString(QChar(0x00D7)), Kind::Operator,
           [](Engine& e) { e.inputOperator(Op::Multiply); }, QLatin1Char('*'));
    addKey(grid, 2, 3, QString(QChar(0x2212)), Kind::Operator,
           [](Engine& e) { e.inputOperator(Op::Subtract); }, QLatin1Char('-'));
    addKey(grid, 3, 3, QStringLiteral("+"), Kind::Operator,
           [](Engine& e) { e.inputOperator(Op::Add); }, QLatin1Char('+'));

    // Row 4:  +/-  0  .  =
    signButton_ = addKey(grid, 4, 0, QString(QChar(0x00B1)), Kind::Action,
                         [](Engine& e) { e.toggleSign(); });
    addDigit(4, 1, 0);
    addKey(grid, 4, 2, QStringLiteral("."), Kind::Digit,
           [](Engine& e) { e.inputDecimalPoint(); }, QLatin1Char('.'));
    addKey(grid, 4, 3, QStringLiteral("="), Kind::Equals,
           [](Engine& e) { e.equals(); }, QLatin1Char('='));

    clearButton_->setToolTip(QStringLiteral("Clear entry / all clear (Delete, Esc)"));
    backspaceButton_->setToolTip(QStringLiteral("Backspace"));
    signButton_->setToolTip(QStringLiteral("Change sign (F9)"));
}

CalcButton* MainWindow::addKey(QGridLayout* grid, int row, int column, const QString& label,
                               CalcButton::Kind kind, Action action, QChar key)
{
    auto* button = new CalcButton(label, kind, this);
    grid->addWidget(button, row, column);
    connect(button, &QPushButton::clicked, this, [this, action]() { perform(action); });
    if (!key.isNull())
        keyButtons_.insert(static_cast<int>(key.unicode()), button);
    return button;
}

void MainWindow::perform(const Action& action)
{
    action(engine_);
    refresh();
}

void MainWindow::refresh()
{
    display_->setContent(toDisplayText(engine_.mainLine()),
                         toDisplayText(engine_.secondaryLine()),
                         engine_.hasError());
    clearButton_->setText(engine_.canClearEntry() ? QStringLiteral("C") : QStringLiteral("AC"));
}

// Briefly shows the on-screen key as pressed when its keyboard key is used.
void MainWindow::flash(CalcButton* button)
{
    if (!button)
        return;
    button->setDown(true);
    QTimer::singleShot(90, button, [button]() { button->setDown(false); });
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Copy)) {
        copyDisplay();
        return;
    }
    if (event->matches(QKeySequence::Paste)) {
        pasteText();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        handleCharacter(QLatin1Char('='));
        return;
    case Qt::Key_Backspace:
        flash(backspaceButton_);
        perform([](Engine& e) { e.backspace(); });
        return;
    case Qt::Key_Escape:
        flash(clearButton_);
        perform([](Engine& e) { e.clearAll(); });
        return;
    case Qt::Key_Delete:
    case Qt::Key_C:
        flash(clearButton_);
        perform(clearPressed);
        return;
    case Qt::Key_F9:
        flash(signButton_);
        perform([](Engine& e) { e.toggleSign(); });
        return;
    default:
        break;
    }

    // Everything else is read from the typed text, so it works on any keyboard layout.
    bool handled = false;
    for (const QChar ch : event->text())
        handled = handleCharacter(ch) || handled;
    if (!handled)
        QWidget::keyPressEvent(event);
}

// Feeds one typed (or pasted) character to the engine. Returns false if the
// character means nothing to the calculator.
bool MainWindow::handleCharacter(QChar ch)
{
    const int code = ch.unicode();
    int key = 0;  // character of the matching on-screen key

    if (code >= '0' && code <= '9') {
        key = code;
        engine_.inputDigit(code - '0');
    } else {
        switch (code) {
        case '.':
        case ',':  // decimal key on many European numpads
            key = '.';
            engine_.inputDecimalPoint();
            break;
        case '+':
            key = '+';
            engine_.inputOperator(Op::Add);
            break;
        case '-':
        case 0x2212:  // minus sign
        case 0x2013:  // en dash
            key = '-';
            engine_.inputOperator(Op::Subtract);
            break;
        case '*':
        case 'x':
        case 'X':
        case 0x00D7:  // multiplication sign
            key = '*';
            engine_.inputOperator(Op::Multiply);
            break;
        case '/':
        case 0x00F7:  // division sign
            key = '/';
            engine_.inputOperator(Op::Divide);
            break;
        case '%':
            key = '%';
            engine_.inputPercent();
            break;
        case '=':
            key = '=';
            engine_.equals();
            break;
        default:
            return false;
        }
    }

    flash(keyButtons_.value(key));
    refresh();
    return true;
}

void MainWindow::copyDisplay()
{
    const std::string text = engine_.copyText();
    if (!text.empty())
        QGuiApplication::clipboard()->setText(QString::fromStdString(text));
}

void MainWindow::pasteText()
{
    QString text = QGuiApplication::clipboard()->text();
    if (text.contains(QLatin1Char('.')))
        text.remove(QLatin1Char(','));  // "1,234.5": commas are thousands separators
    for (const QChar ch : text)
        handleCharacter(ch);
}