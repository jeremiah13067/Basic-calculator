#pragma once

#include <string>

namespace calc {

// The calculator's brain: it knows what every key does and what the display
// should show, but nothing about windows or widgets.
//
// The expression being typed is plain ASCII: digits, '.', the operators + - * /,
// a '%' suffix, and the letter 'A', which stands for the previous answer
// ("A*3" continues a calculation from the last result).
class CalculatorEngine {
public:
    enum class Operator { Add, Subtract, Multiply, Divide };

    // Keys
    void inputDigit(int digit);  // 0-9
    void inputDecimalPoint();
    void inputOperator(Operator op);
    void inputPercent();
    void toggleSign();  // +/-
    void backspace();
    void clearEntry();  // "C": remove the number being typed
    void clearAll();    // "AC"
    void equals();

    // What to show
    std::string mainLine() const;       // expression, result or error message
    std::string secondaryLine() const;  // "= preview" while typing, "expression =" after equals
    std::string copyText() const;       // clipboard text (empty on error)
    bool hasError() const { return phase_ == Phase::Error; }
    bool canClearEntry() const;         // true: the key should read "C", otherwise "AC"

    // Up to 15 significant digits, no trailing zeros: 0.1 + 0.2 -> "0.3".
    static std::string formatNumber(double value);

private:
    enum class Phase { Editing, Result, Error };

    void startNewExpression();
    void continueFromAnswer();
    std::string displayText(const std::string& expression) const;
    std::string preview() const;

    std::string expression_;
    std::string history_;  // "2 + 3 =" shown above a result or error
    std::string error_;
    double answer_ = 0.0;
    Phase phase_ = Phase::Editing;
};

}  // namespace calc