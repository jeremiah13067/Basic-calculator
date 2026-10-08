#include "CalculatorEngine.h"

#include "ExpressionEvaluator.h"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <locale>
#include <sstream>

namespace calc {
namespace {

constexpr int kMaxDigits = 15;  // per number, matching the 15 digits shown

bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isOperatorChar(char c) { return c == '+' || c == '-' || c == '*' || c == '/'; }
bool isOperandChar(char c) { return isDigit(c) || c == '.' || c == '%' || c == 'A'; }

char symbolFor(CalculatorEngine::Operator op)
{
    switch (op) {
    case CalculatorEngine::Operator::Add:      return '+';
    case CalculatorEngine::Operator::Subtract: return '-';
    case CalculatorEngine::Operator::Multiply: return '*';
    case CalculatorEngine::Operator::Divide:   return '/';
    }
    return '+';
}

// Start of the run of digits and '.' that ends just before `end`.
std::size_t numberStart(const std::string& s, std::size_t end)
{
    while (end > 0 && (isDigit(s[end - 1]) || s[end - 1] == '.'))
        --end;
    return end;
}

// Start of the operand at the end of `s` (a number or 'A', ignoring a '%'
// suffix), or npos when `s` does not end with an operand.
std::size_t trailingOperandStart(const std::string& s)
{
    std::size_t end = s.size();
    while (end > 0 && s[end - 1] == '%')
        --end;
    if (end > 0 && s[end - 1] == 'A')
        return end - 1;
    const std::size_t start = numberStart(s, end);
    return start == end ? std::string::npos : start;
}

// True if the '-' at index i is a sign (start of text, or right after an operator).
bool isSignAt(const std::string& s, std::size_t i)
{
    return s[i] == '-' && (i == 0 || isOperatorChar(s[i - 1]));
}

// True if the operand starting at `start` has a sign directly in front of it.
bool signPrecedes(const std::string& s, std::size_t start)
{
    return start > 0 && isSignAt(s, start - 1);
}

}  // namespace

// ---------------------------------------------------------------- keys

void CalculatorEngine::inputDigit(int digit)
{
    if (digit < 0 || digit > 9)
        return;
    if (phase_ != Phase::Editing)
        startNewExpression();
    if (!expression_.empty() && expression_.back() == 'A')
        expression_.pop_back();  // typing replaces the previous answer
    if (!expression_.empty() && expression_.back() == '%')
        return;

    const std::string number = expression_.substr(numberStart(expression_, expression_.size()));
    const char c = static_cast<char>('0' + digit);
    if (number == "0") {
        if (digit != 0)
            expression_.back() = c;  // a lone zero is replaced, so no "007"
        return;
    }
    if (std::count_if(number.begin(), number.end(), isDigit) >= kMaxDigits)
        return;
    expression_.push_back(c);
}

void CalculatorEngine::inputDecimalPoint()
{
    if (phase_ != Phase::Editing)
        startNewExpression();
    if (!expression_.empty() && expression_.back() == 'A')
        expression_.pop_back();
    if (!expression_.empty() && expression_.back() == '%')
        return;

    const std::string number = expression_.substr(numberStart(expression_, expression_.size()));
    if (number.find('.') != std::string::npos)
        return;
    expression_ += number.empty() ? "0." : ".";
}

void CalculatorEngine::inputOperator(Operator op)
{
    if (phase_ == Phase::Error)
        return;
    if (phase_ == Phase::Result)
        continueFromAnswer();

    const char symbol = symbolFor(op);

    if (expression_.empty()) {
        // '-' starts a negative number; the others start from zero.
        expression_ = (symbol == '-') ? "-" : std::string("0") + symbol;
        return;
    }
    if (expression_ == "-")
        return;  // a lone minus sign is waiting for its number

    const char last = expression_.back();
    if (isOperandChar(last)) {
        if (last == '.')
            expression_.pop_back();  // "5." + operator -> "5+"
        expression_.push_back(symbol);
        return;
    }

    // The expression ends with an operator.
    if (symbol == '-') {
        if (last == '*' || last == '/') {
            expression_.push_back('-');  // "5 x -" starts a negative number
            return;
        }
        if (last == '-')
            return;
    }
    // Otherwise the new operator replaces the pending one(s).
    while (!expression_.empty() && isOperatorChar(expression_.back()))
        expression_.pop_back();
    expression_.push_back(symbol);
}

void CalculatorEngine::inputPercent()
{
    if (phase_ == Phase::Error)
        return;
    if (phase_ == Phase::Result)
        continueFromAnswer();
    if (expression_.empty() || !isOperandChar(expression_.back()) || expression_.back() == '%')
        return;
    if (expression_.back() == '.')
        expression_.pop_back();
    expression_.push_back('%');
}

void CalculatorEngine::toggleSign()
{
    if (phase_ == Phase::Error)
        return;
    if (phase_ == Phase::Result) {
        answer_ = -answer_;  // negate the shown result in place
        history_.clear();
        return;
    }

    const std::size_t start = trailingOperandStart(expression_);
    if (start == std::string::npos) {
        // No number yet: cancel a pending sign, or start a negative number.
        if (!expression_.empty() && isSignAt(expression_, expression_.size() - 1))
            expression_.pop_back();
        else
            expression_ += "-0";
        return;
    }
    if (signPrecedes(expression_, start))
        expression_.erase(start - 1, 1);
    else
        expression_.insert(start, 1, '-');
}

void CalculatorEngine::backspace()
{
    if (phase_ == Phase::Error) {
        clearAll();
        return;
    }
    if (phase_ == Phase::Result) {
        // Make the result editable and delete its last character.
        const std::string text = formatNumber(answer_);
        if (text.find('e') != std::string::npos) {
            clearAll();  // scientific notation cannot be edited digit by digit
            return;
        }
        expression_ = text;
        expression_.pop_back();
        history_.clear();
        phase_ = Phase::Editing;
        return;
    }

    if (expression_.empty())
        return;
    expression_.pop_back();
    if (expression_ == "A") {  // the operator that continued from a result was removed
        expression_.clear();
        phase_ = Phase::Result;
    }
}

void CalculatorEngine::clearEntry()
{
    if (!canClearEntry()) {
        clearAll();
        return;
    }
    std::size_t start = trailingOperandStart(expression_);
    if (signPrecedes(expression_, start))
        --start;
    expression_.erase(start);
    if (expression_.empty())
        clearAll();
}

void CalculatorEngine::clearAll()
{
    startNewExpression();
    answer_ = 0.0;
}

void CalculatorEngine::equals()
{
    if (phase_ != Phase::Editing)
        return;

    std::string text = expression_;
    while (!text.empty() && isOperatorChar(text.back()))
        text.pop_back();  // "5 +" evaluates as "5"
    if (text.empty())
        return;

    const EvalResult result = ExpressionEvaluator::evaluate(text, answer_);
    history_ = displayText(text) + " =";  // built before the answer changes
    expression_.clear();
    if (result.ok()) {
        answer_ = result.value;
        phase_ = Phase::Result;
    } else {
        error_ = ExpressionEvaluator::describe(result.status);
        phase_ = Phase::Error;
    }
}

// ------------------------------------------------------------- display

bool CalculatorEngine::canClearEntry() const
{
    return phase_ == Phase::Editing && trailingOperandStart(expression_) != std::string::npos;
}

std::string CalculatorEngine::mainLine() const
{
    switch (phase_) {
    case Phase::Result: return formatNumber(answer_);
    case Phase::Error:  return error_;
    case Phase::Editing: break;
    }
    return expression_.empty() ? std::string("0") : displayText(expression_);
}

std::string CalculatorEngine::secondaryLine() const
{
    return phase_ == Phase::Editing ? preview() : history_;
}

std::string CalculatorEngine::copyText() const
{
    return phase_ == Phase::Error ? std::string() : mainLine();
}

std::string CalculatorEngine::formatNumber(double value)
{
    if (value == 0.0)
        value = 0.0;  // turns -0 into 0

    std::ostringstream out;
    out.imbue(std::locale::classic());  // never a decimal comma
    out << std::setprecision(15) << value;
    std::string text = out.str();

    // Tidy the exponent: "1e-05" -> "1e-5"
    const std::size_t e = text.find('e');
    if (e != std::string::npos) {
        const std::size_t digits = e + 2;  // skip 'e' and its sign
        while (digits + 1 < text.size() && text[digits] == '0')
            text.erase(digits, 1);
    }
    return text;
}

// ------------------------------------------------------------- helpers

void CalculatorEngine::startNewExpression()
{
    expression_.clear();
    history_.clear();
    error_.clear();
    phase_ = Phase::Editing;
}

void CalculatorEngine::continueFromAnswer()
{
    expression_ = "A";
    phase_ = Phase::Editing;
}

// Spaces around binary operators, signs attached to their number, and the
// previous answer written out. Operators stay ASCII; the GUI swaps in symbols.
std::string CalculatorEngine::displayText(const std::string& expression) const
{
    std::string out;
    for (std::size_t i = 0; i < expression.size(); ++i) {
        const char c = expression[i];
        if (c == 'A') {
            out += formatNumber(answer_);
        } else if (isOperatorChar(c) && i > 0 && isOperandChar(expression[i - 1])) {
            out += ' ';
            out += c;
            out += ' ';
        } else {
            out += c;
        }
    }
    if (!out.empty() && out.back() == ' ')
        out.pop_back();
    return out;
}

// Live result of what has been typed so far, or "" if there is nothing
// useful to show (a lone number, an incomplete or invalid expression).
std::string CalculatorEngine::preview() const
{
    std::string text = expression_;
    while (!text.empty() && isOperatorChar(text.back()))
        text.pop_back();
    if (text.find_first_of("+-*/%", 1) == std::string::npos)
        return std::string();

    const EvalResult result = ExpressionEvaluator::evaluate(text, answer_);
    return result.ok() ? "= " + formatNumber(result.value) : std::string();
}

}  // namespace calc