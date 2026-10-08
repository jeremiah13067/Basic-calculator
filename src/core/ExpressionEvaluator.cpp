#include "ExpressionEvaluator.h"

#include <cmath>
#include <cstddef>
#include <locale>
#include <sstream>

namespace calc {
namespace {

struct EvalFailure {
    EvalStatus status;
};

struct Value {
    double number = 0.0;
    bool isPercent = false;  // true for a bare percentage such as "10%"
};

// Recursive-descent parser that evaluates as it parses.
//   expression := term   { ('+' | '-') term }
//   term       := factor { ('*' | '/') factor }
//   factor     := ('+' | '-') factor | number { '%' }
class Parser {
public:
    Parser(const std::string& text, double answer) : text_(text), answer_(answer) {}

    double parse()
    {
        const double result = parseExpression();
        skipSpaces();
        if (pos_ != text_.size())
            fail(EvalStatus::SyntaxError);
        return result;
    }

private:
    double parseExpression()
    {
        double total = parseTerm().number;
        for (;;) {
            skipSpaces();
            const char op = peek();
            if (op != '+' && op != '-')
                return total;
            ++pos_;
            const Value rhs = parseTerm();
            // "200 + 10%" means 10% of 200.
            const double amount = rhs.isPercent ? total * rhs.number : rhs.number;
            total = checked(op == '+' ? total + amount : total - amount);
        }
    }

    Value parseTerm()
    {
        Value left = parseFactor();
        for (;;) {
            skipSpaces();
            const char op = peek();
            if (op != '*' && op != '/')
                return left;
            ++pos_;
            const Value right = parseFactor();
            if (op == '*') {
                left.number = checked(left.number * right.number);
            } else {
                if (right.number == 0.0)
                    fail(left.number == 0.0 ? EvalStatus::Undefined : EvalStatus::DivisionByZero);
                left.number = checked(left.number / right.number);
            }
            left.isPercent = false;
        }
    }

    Value parseFactor()
    {
        skipSpaces();
        const char c = peek();
        if (c == '-' || c == '+') {
            ++pos_;
            Value v = parseFactor();
            if (c == '-')
                v.number = -v.number;
            return v;
        }

        Value v;
        v.number = parseNumber();
        skipSpaces();
        while (peek() == '%') {
            ++pos_;
            v.number /= 100.0;
            v.isPercent = true;
            skipSpaces();
        }
        return v;
    }

    double parseNumber()
    {
        if (peek() == 'A') {
            ++pos_;
            return answer_;
        }

        const std::size_t start = pos_;
        bool sawDigit = false;
        bool sawPoint = false;
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if (c >= '0' && c <= '9')
                sawDigit = true;
            else if (c == '.' && !sawPoint)
                sawPoint = true;
            else
                break;
            ++pos_;
        }
        if (!sawDigit)
            fail(EvalStatus::SyntaxError);

        // A stream with the classic locale never expects a decimal comma,
        // whatever the user's system locale is.
        std::istringstream stream(text_.substr(start, pos_ - start));
        stream.imbue(std::locale::classic());
        double value = 0.0;
        stream >> value;
        if (stream.fail())
            fail(EvalStatus::SyntaxError);
        return checked(value);
    }

    double checked(double value) const
    {
        if (!std::isfinite(value))
            fail(EvalStatus::Overflow);
        return value;
    }

    char peek() const { return pos_ < text_.size() ? text_[pos_] : '\0'; }
    void skipSpaces() { while (peek() == ' ') ++pos_; }
    [[noreturn]] static void fail(EvalStatus status) { throw EvalFailure{status}; }

    std::string text_;
    double answer_;
    std::size_t pos_ = 0;
};

}  // namespace

EvalResult ExpressionEvaluator::evaluate(const std::string& expression, double answer)
{
    EvalResult result;
    try {
        result.value = Parser(expression, answer).parse();
    } catch (const EvalFailure& failure) {
        result.status = failure.status;
        result.value = 0.0;
    }
    return result;
}

const char* ExpressionEvaluator::describe(EvalStatus status)
{
    switch (status) {
    case EvalStatus::Ok:             return "";
    case EvalStatus::SyntaxError:    return "Invalid expression";
    case EvalStatus::DivisionByZero: return "Cannot divide by zero";
    case EvalStatus::Undefined:      return "Result is undefined";
    case EvalStatus::Overflow:       return "Overflow";
    }
    return "";
}

}  // namespace calc