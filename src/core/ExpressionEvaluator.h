#pragma once

#include <string>

namespace calc {

enum class EvalStatus {
    Ok,
    SyntaxError,     // not a valid expression
    DivisionByZero,  // x / 0
    Undefined,       // 0 / 0
    Overflow         // result too large to represent
};

struct EvalResult {
    EvalStatus status = EvalStatus::Ok;
    double value = 0.0;

    bool ok() const { return status == EvalStatus::Ok; }
};

// Evaluates text such as "2+3*4" or "200-10%".
//
//  * numbers with an optional decimal point, binary + - * /, unary + and -
//  * precedence: unary sign, then * and /, then + and - (left to right)
//  * postfix %: "50%" is 0.5; after + or - a bare percentage is taken of the
//    left-hand value, so "200+10%" is 220 and "200-10%" is 180
//  * 'A' stands for `answer`, the previous result. Using the exact value keeps
//    full precision when a calculation is continued from a result.
class ExpressionEvaluator {
public:
    static EvalResult evaluate(const std::string& expression, double answer = 0.0);
    static const char* describe(EvalStatus status);
};

}  // namespace calc