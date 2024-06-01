#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) 
        : ast_(ParseFormulaAST(expression))
    {}

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            Value value;
            value = ast_.Execute(sheet);
            return value;
        }
        catch (const FormulaError& fe) {
            return fe;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream os(expression_);
        ast_.PrintFormula(os);

        return os.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        const auto& pos_list = ast_.GetCells();
        //std::vector<Position> s(pos_list.begin(), pos_list.end());
        std::set<Position> s(pos_list.begin(), pos_list.end());
        return /*s;*/ { s.begin(), s.end() };
    }

private:
    FormulaAST ast_;
    std::string expression_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("ParseFormula error");
    }
}