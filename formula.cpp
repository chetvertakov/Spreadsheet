#include "common.h"
#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) : category_{category} {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    const static std::string REF = "#REF!"s;
    const static std::string VALUE = "#VALUE!"s;
    const static std::string DIV0 = "#DIV/0!"s;
    const static std::string EMPTY = ""s;
    switch (category_) {
        case Category::Ref :
            return REF;
        case Category::Value :
            return VALUE;
        case Category::Div0 :
            return DIV0;
        default :
            assert(false);
            return EMPTY;
    }
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {

class Formula final : public FormulaInterface {
public:

    explicit Formula(std::string expression) try : ast_{ParseFormulaAST(expression)} {
        Position prev = Position::NONE;
        for (auto &cell : ast_.GetCells()) {
            if (cell.IsValid() && !(cell == prev)) {
                referenced_cells_.push_back(std::move(cell));
                prev = cell;
            }
        }
    } catch (std::exception &exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        }  catch (FormulaError &err) {
            return err;
        }
    }
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return referenced_cells_;
    }

private:
    FormulaAST ast_;
    std::vector<Position> referenced_cells_;
};

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
