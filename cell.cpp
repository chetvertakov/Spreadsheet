#include "cell.h"
#include "formula.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;

class Cell::Impl {
public:
    virtual CellInterface::Value GetValue(const SheetInterface &sheet) {
        return ""s;
    }

    virtual std::string GetText() {
        return ""s;
    }

    virtual std::vector<Position> GetReferencedCells() const {
        return {};
    }

    virtual void ClearCahce() {
    }

    virtual ~Impl() = default;
protected:
    Impl() = default;
};

class Cell::EmptyImpl final : public Impl {
public:
    EmptyImpl() = default;
};

class Cell::TextImpl final : public Impl {
public:
    TextImpl(const std::string &text) : text_{text} {}

    CellInterface::Value GetValue(const SheetInterface &sheet) override {
        if (text_.at(0) == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }

    std::string GetText() override {
        return text_;
    }
private:
    std::string text_;
};

class Cell::FormulaImpl final : public Impl {
public:
    FormulaImpl(const std::string &text)
        : formula_{ParseFormula(text)} {}

    CellInterface::Value GetValue(const SheetInterface &sheet) override {
        if (!cache_) {
            cache_ = formula_->Evaluate(sheet);
        }
        return std::visit([](auto val) {
            return CellInterface::Value(val);
        }, cache_.value());
    }

    std::string GetText() override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

    void ClearCahce() override {
        cache_.reset();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    std::optional<FormulaInterface::Value> cache_;
};

Cell::Cell(Sheet &sheet)
    : sheet_{sheet}
    , impl_{std::make_unique<EmptyImpl>()} {
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text == impl_->GetText()) {
        return;
    }
    if (text.size() > 1 && text.at(0) == FORMULA_SIGN) {
        auto temp_impl = std::make_unique<FormulaImpl>(text.substr(1));
        TestCycles(temp_impl->GetReferencedCells(), this);
        impl_ = std::move(temp_impl);
    } else if (text.size () > 0){
        impl_ = std::make_unique<TextImpl>(text);
    } else {
        impl_ = std::make_unique<EmptyImpl>();
    }
    DeleteParents();
    SetParents();
    ClearChildrenCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
    DeleteParents();
    ClearChildrenCache();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue(sheet_);
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !children_.empty();
}

void Cell::SetParents() {
    for (const auto &parent_pos : GetReferencedCells()) {
        auto parent = sheet_.GetCellOrCreate(parent_pos);
        // указываем в каждом родителе, что данная ячейка является их ребенком
        parent->children_.insert(this);
        // добавляем ячейку в список родителей текущей ячейки
        parents_.insert(parent);
    }
}

void Cell::DeleteParents() {
    for (auto parent : parents_) {
        // удаляем у каждого родителя информацию о том, что данная ячейка является для него ребенком
        parent->children_.erase(this);
    }
    // очищаем список родителей ячейки
    parents_.clear();
}

void Cell::ClearCache() {
    impl_->ClearCahce();
}

void Cell::ClearChildrenCache() {
    // удаляем кеш у ячейки
    ClearCache();
    for (auto child : children_) {
        // вызываем рекурсивно для каждого ребенка
        child->ClearChildrenCache();
    }
}

void Cell::TestCycles(const std::vector<Position> &parents, const Cell *root) {
    for (const auto &parent_pos : parents) {
        if (auto parent = dynamic_cast<Cell*>(sheet_.GetCell(parent_pos))) {
            if (parent == root) {
                throw CircularDependencyException("Cycle found!"s);
            }
            parent->TestCycles(parent->GetReferencedCells(), root);
        }
    }
}




