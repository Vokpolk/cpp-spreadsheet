#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet)
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>())
{}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    referenced_cells_.clear();

    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        //impl_ = std::make_unique<FormulaImpl>(std::string(text.begin() + 1, text.end()), sheet_);

        std::unique_ptr<Impl> temp = std::make_unique<FormulaImpl>(std::string(text.begin() + 1, text.end()), sheet_);
        std::vector<Position> temp_ref_cells = temp->GetReferencedCells();
        CyclicDependency(temp_ref_cells);
        UpdateDependencies(temp_ref_cells);
        impl_ = std::move(temp);
    }
    else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
    InvalidateCache();
}

void Cell::Clear() {
    impl_.release();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}


void Cell::CyclicDependency(const std::vector<Position>& ref_cells) const {
    std::unordered_set<CellInterface*> visited_cells;
    CyclicDependencyRecur(ref_cells, visited_cells);
}

void Cell::CyclicDependencyRecur(const std::vector<Position>& ref_cells, std::unordered_set<CellInterface*>& visited_cells) const {
    for (const auto& pos : ref_cells) {
        CellInterface* ref_cell = sheet_.GetCell(pos);
        if (ref_cell == this) {
            throw CircularDependencyException("Circular dependency!!!");
        }
        if (ref_cell && visited_cells.count(ref_cell) == 0) {
            const auto& another_ref_cells = ref_cell->GetReferencedCells();
            if (!another_ref_cells.empty()) {
                CyclicDependencyRecur(another_ref_cells, visited_cells);
            }
            visited_cells.insert(ref_cell);
        }
    }
}

void Cell::UpdateDependencies(const std::vector<Position>& new_ref_cells) {
    dependent_cells_.erase(this);

    for (const auto& pos : new_ref_cells) {
        if (!sheet_.GetCell(pos)) {
            sheet_.SetCell(pos, "");
        }
        Cell* new_ref_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));

        referenced_cells_.insert(new_ref_cell);
    }
}

void Cell::InvalidateCache() {
    impl_->ResetCache();

    std::unordered_set<Cell*> visited_cells;
    InvalidateCacheRecur(visited_cells);
}

void Cell::InvalidateCacheRecur(std::unordered_set<Cell*>& visited_cells) {
    for (const auto& dependent_cell : dependent_cells_) {
        dependent_cell->impl_->ResetCache();
        if (visited_cells.count(dependent_cell) == 0) {
            if (!dependent_cell->dependent_cells_.empty()) {
                dependent_cell->InvalidateCacheRecur(visited_cells);
            }
            visited_cells.insert(dependent_cell);
        }
    }
}


std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

std::optional<FormulaInterface::Value> Cell::Impl::GetCache() const {
    return std::nullopt;
}

void Cell::Impl::ResetCache()
{}


Cell::Value Cell::EmptyImpl::GetValue() {
    return "";
}
std::string Cell::EmptyImpl::GetText() const {
    return "";
}


Cell::TextImpl::TextImpl(std::string expr)
    : value_(expr)
{}

Cell::Value Cell::TextImpl::GetValue() {
    if (value_[0] == ESCAPE_SIGN) {
        return std::string(value_.begin() + 1, value_.end());
    }
    return value_;
}

std::string Cell::TextImpl::GetText() const {
    return value_;
}


Cell::FormulaImpl::FormulaImpl(std::string expr, const SheetInterface& sheet)
    : formula_(ParseFormula(expr))
    , sheet_(sheet)
{}

Cell::Value Cell::FormulaImpl::GetValue() {
    /*auto formula = ParseFormula(std::string(std::get<std::string>(value).begin() + 1, std::get<std::string>(value).end())).get()->Evaluate();
    if (std::holds_alternative<double>(formula)) {
        return std::get<double>(formula);
    }
    else {
        return std::get<FormulaError>(formula);
    }*/

    if (!cache_.has_value()) {
        cache_ = formula_->Evaluate(sheet_);
    }

    if (std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
    }
    else {
        return std::get<FormulaError>(cache_.value());
    }
}
std::string Cell::FormulaImpl::GetText() const {
    return '=' + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::ResetCache() {
    cache_.reset();
}

std::optional<FormulaInterface::Value> Cell::FormulaImpl::GetCache() const {
    return cache_;
}