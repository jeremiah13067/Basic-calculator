// Console tests for the calculator logic. No GUI and no Qt needed.
#include "CalculatorEngine.h"
#include "ExpressionEvaluator.h"

#include <cmath>
#include <iostream>
#include <string>

using calc::CalculatorEngine;
using calc::EvalStatus;
using calc::ExpressionEvaluator;

namespace {

int checks = 0;
int failures = 0;

void check(bool ok, const std::string& what)
{
    ++checks;
    if (!ok) {
        ++failures;
        std::cout << "FAIL: " << what << '\n';
    }
}

// Types a key sequence into the engine:
//   0-9 . + - * / % =  as on the keypad
//   N = +/-   B = backspace   C = clear entry   A = all clear
void press(CalculatorEngine& engine, const std::string& keys)
{
    using Op = CalculatorEngine::Operator;
    for (const char key : keys) {
        if (key >= '0' && key <= '9') {
            engine.inputDigit(key - '0');
            continue;
        }
        switch (key) {
        case '.': engine.inputDecimalPoint(); break;
        case '+': engine.inputOperator(Op::Add); break;
        case '-': engine.inputOperator(Op::Subtract); break;
        case '*': engine.inputOperator(Op::Multiply); break;
        case '/': engine.inputOperator(Op::Divide); break;
        case '%': engine.inputPercent(); break;
        case '=': engine.equals(); break;
        case 'N': engine.toggleSign(); break;
        case 'B': engine.backspace(); break;
        case 'C': engine.clearEntry(); break;
        case 'A': engine.clearAll(); break;
        default: break;
        }
    }
}

std::string mainAfter(const std::string& keys)
{
    CalculatorEngine engine;
    press(engine, keys);
    return engine.mainLine();
}

std::string secondaryAfter(const std::string& keys)
{
    CalculatorEngine engine;
    press(engine, keys);
    return engine.secondaryLine();
}

void expectValue(const std::string& text, double expected, double answer = 0.0)
{
    const auto r = ExpressionEvaluator::evaluate(text, answer);
    check(r.ok() && std::fabs(r.value - expected) < 1e-9, "evaluate \"" + text + "\"");
}

void expectStatus(const std::string& text, EvalStatus expected)
{
    check(ExpressionEvaluator::evaluate(text).status == expected, "status of \"" + text + "\"");
}

}  // namespace

int main()
{
    struct Case {
        const char* keys;
        const char* expected;
    };
    static const Case cases[] = {
        // precedence and left-to-right evaluation
        {"2+3*4=", "14"}, {"2*3+4=", "10"}, {"10-2-3=", "5"}, {"100/10/5=", "2"},
        {"2+6/3*2=", "6"}, {"9-3*2+1=", "4"}, {"7+8=", "15"}, {"6*7=", "42"},
        {"9/3=", "3"}, {"5-9=", "-4"},
        // decimals
        {"0.1+0.2=", "0.3"}, {".5+.25=", "0.75"}, {"1.5*4=", "6"}, {"10/4=", "2.5"},
        {"1/3*3=", "1"}, {"1.5.5=", "1.55"}, {"0.1*3=", "0.3"}, {"10-0.5=", "9.5"},
        {"2.50+2.50=", "5"},
        // negative numbers and the +/- key
        {"-5+3=", "-2"}, {"5*-3=", "-15"}, {"-2*-3=", "6"}, {"3-5=", "-2"},
        {"5-3N=", "8"}, {"5N*3=", "-15"}, {"N5=", "-5"}, {"5NN=", "5"},
        // percent
        {"50%=", "0.5"}, {"200+10%=", "220"}, {"200-10%=", "180"}, {"200*10%=", "20"},
        {"50/50%=", "100"}, {"%", "0"}, {"5%", "5%"},
        // errors
        {"5/0=", "Cannot divide by zero"}, {"0/0=", "Result is undefined"},
        {"2*3/0+1=", "Cannot divide by zero"}, {"1/0.0=", "Cannot divide by zero"},
        {"5/0=7", "7"}, {"5/0=+", "Cannot divide by zero"}, {"5/0=B", "0"},
        // chained calculations
        {"2+3=*4=", "20"}, {"10/4=+1=", "3.5"}, {"1/3=*3=", "1"}, {"5+3=7", "7"},
        {"9*9=-1=", "80"}, {"2+3=+4=+5=", "14"}, {"2+3=%=", "0.05"}, {"6*7=N", "-42"},
        {"2+3=N+1=", "-4"},
        // backspace and clear
        {"123B", "12"}, {"5+3B", "5 +"}, {"5+3BB", "5"}, {"BBB", "0"}, {"12+3=B", "1"},
        {"5B", "0"}, {"1+1=+B", "2"},
        {"12+34C", "12 +"}, {"12+34CC", "0"}, {"12+34A", "0"}, {"1+1=A", "0"}, {"1+1=C", "0"},
        // operator replacement and unusual sequences
        {"5+*3=", "15"}, {"5*+3=", "8"}, {"5*-+3=", "8"}, {"5--3=", "2"}, {"+5=", "5"},
        {"*5=", "0"}, {"5+=", "5"}, {"5*=", "5"},
        // digit entry
        {"007", "7"}, {"000", "0"}, {"0.05", "0.05"}, {"1234567890123456", "123456789012345"},
        {"5+007", "5 + 7"}, {"5%3", "5%"},
        // what the display shows while typing
        {"5*-3", "5 * -3"}, {"2+3*4", "2 + 3 * 4"},
    };
    for (const Case& c : cases) {
        const std::string actual = mainAfter(c.keys);
        check(actual == c.expected, std::string("keys ") + c.keys + ": expected \"" + c.expected +
                                        "\", got \"" + actual + "\"");
    }

    // A product far beyond the range of a double
    std::string huge;
    for (int i = 0; i < 21; ++i)
        huge += "999999999999999*";
    huge += "9=";
    check(mainAfter(huge) == "Overflow", "overflow is reported");

    // Small line: live preview, finished expression
    check(secondaryAfter("2+3*4") == "= 14", "live preview");
    check(secondaryAfter("2+3*4=") == "2 + 3 * 4 =", "expression after equals");
    check(secondaryAfter("5") == "", "no preview for a lone number");
    check(secondaryAfter("5/0") == "", "no preview for an invalid result");
    check(secondaryAfter("1+1=+") == "", "no preview right after continuing");
    check(secondaryAfter("1+1=+B") == "1 + 1 =", "backspace returns to the result");

    // C / AC and error state
    {
        CalculatorEngine e;
        check(!e.canClearEntry(), "fresh engine shows AC");
        press(e, "5");
        check(e.canClearEntry(), "typing a number shows C");
        press(e, "+");
        check(!e.canClearEntry(), "after an operator shows AC");
        press(e, "3=");
        check(!e.canClearEntry(), "after = shows AC");
    }
    {
        CalculatorEngine e;
        press(e, "5/0=");
        check(e.hasError(), "5/0 sets the error state");
        check(e.secondaryLine() == "5 / 0 =", "failed expression is shown");
        press(e, "A");
        check(!e.hasError() && e.mainLine() == "0", "AC clears the error");
    }

    // Number formatting
    check(CalculatorEngine::formatNumber(0.1 + 0.2) == "0.3", "format 0.1+0.2");
    check(CalculatorEngine::formatNumber(-0.0) == "0", "format negative zero");
    check(CalculatorEngine::formatNumber(-2.5) == "-2.5", "format -2.5");
    check(CalculatorEngine::formatNumber(1.0 / 3.0) == "0.333333333333333", "format 1/3");
    check(CalculatorEngine::formatNumber(123456789012345.0) == "123456789012345", "format 15 digits");
    check(CalculatorEngine::formatNumber(1e20) == "1e+20", "format 1e20");
    check(CalculatorEngine::formatNumber(0.00001) == "1e-5", "format 1e-5");

    // The evaluator on its own
    expectValue("2+3*4", 14);
    expectValue("2 + 3", 5);
    expectValue("-2*-3", 6);
    expectValue("10/4", 2.5);
    expectValue("200+10%", 220);
    expectValue("200-10%", 180);
    expectValue("200*10%", 20);
    expectValue("50%", 0.5);
    expectValue("A*2", 42, 21);
    expectValue("1/3*3", 1);
    expectStatus("1/0", EvalStatus::DivisionByZero);
    expectStatus("0/0", EvalStatus::Undefined);
    expectStatus("", EvalStatus::SyntaxError);
    expectStatus("5+", EvalStatus::SyntaxError);
    expectStatus("*5", EvalStatus::SyntaxError);
    expectStatus("1.2.3", EvalStatus::SyntaxError);
    expectStatus("2 3", EvalStatus::SyntaxError);

    std::cout << checks << " checks, " << failures << " failed\n";
    return failures == 0 ? 0 : 1;
}