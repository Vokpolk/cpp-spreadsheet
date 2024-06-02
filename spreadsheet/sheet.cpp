#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {

    if (pos.row < 0 || pos.col < 0 || 
        pos.row >= pos.MAX_ROWS || pos.col >= pos.MAX_COLS) {
        throw InvalidPositionException("SetCell. Invalid position");
    }

    auto cell = std::make_unique<Cell>(*this);
    if (pos.row >= size_.rows) {
        size_.rows = static_cast<size_t>(pos.row + 1);
    }
    if (pos.col >= size_.cols) {
        size_.cols = static_cast<size_t>(pos.col + 1);
    }
    if (!cells_.count(pos)) {
        cells_[pos] = std::make_unique<Cell>(*this);
    }
    cells_[pos]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.row < 0 || pos.col < 0 ||
        pos.row >= pos.MAX_ROWS || pos.col >= pos.MAX_COLS) {
        throw InvalidPositionException("GetCell. Invalid position");
    }
    if (!cells_.count(pos)) {
        return nullptr;
    }
    return cells_.at(pos).get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if (pos.row < 0 || pos.col < 0 ||
        pos.row >= pos.MAX_ROWS || pos.col >= pos.MAX_COLS) {
        throw InvalidPositionException("GetCell. Invalid position");
    }
    if (!cells_.count(pos)) {
        return nullptr;
    }
    return cells_.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    if (pos.row < 0 || pos.col < 0 ||
        pos.row >= pos.MAX_ROWS || pos.col >= pos.MAX_COLS) {
        throw InvalidPositionException("ClearCell. Invalid position");
    }
    if (GetCell(pos) == nullptr) {
        return;
    }
    cells_.at(pos).reset();

    Size new_size{ -1, -1 };
    for (const auto& [pos, cell] : cells_) {
        if (new_size.rows < pos.row && cell.get() != nullptr) {
            new_size.rows = pos.row;
        }
        if (new_size.cols < pos.col && cell.get() != nullptr) {
            new_size.cols = pos.col;
        }
    }
    size_ = { new_size.rows + 1, new_size.cols + 1 };
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; i++) {
        bool first = false;
        for (int j = 0; j < size_.cols; j++) {
            if (first) {
                output << '\t';
            }
            if (cells_.count({ i,j }) != 0 && cells_.at({ i,j }).get() != nullptr) {
				auto cell = cells_.at({ i,j }).get();
                if (std::holds_alternative<double>(cell->GetValue())) {
                    output << std::get<double>(cell->GetValue());
                }
                else if (std::holds_alternative<std::string>(cell->GetValue())) {
                    output << std::get<std::string>(cell->GetValue());
                }
                else {
                    output << std::get<FormulaError>(cell->GetValue());
                }
            }
            first = true;
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; i++) {
        bool first = false;
        for (int j = 0; j < size_.cols; j++) {
            if (first) {
                output << '\t';
            }
            if (cells_.count({ i,j }) != 0 && cells_.at({ i,j }).get() != nullptr) {
                output << cells_.at({ i,j }).get()->GetText();
            }
            first = true;
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}