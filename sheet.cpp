#include "sheet.h"

#include <algorithm>
#include <iostream>

using namespace std::literals;

namespace {

std::ostream& operator<<(std::ostream& output, const CellInterface::Value &val) {
    std::visit([&output](auto &v) {
        output << v;
    }, val);
    return output;
}

} // namespace

void Sheet::SetCell(Position pos, std::string text) {
    TestPosition(pos);
    rows_[pos.row].SetCell(pos.col, text, *this);
    if (rows_[pos.row].Size() > 0 && !text.empty()) {
        non_empty_rows_.insert(pos.row);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    TestPosition(pos);
    auto row = rows_.find(pos.row);
    if (row == rows_.end()) {
        return nullptr;
    }
    return row->second[pos.col];
}

CellInterface* Sheet::GetCell(Position pos) {
    TestPosition(pos);
    auto row = rows_.find(pos.row);
    if (row == rows_.end()) {
        return nullptr;
    }
    return row->second[pos.col];
}

void Sheet::ClearCell(Position pos) {
    TestPosition(pos);
    auto row = rows_.find(pos.row);
    if (row != rows_.end()) {
        row->second.Clear(pos.col);
        if (row->second.Size() == 0) {
            non_empty_rows_.erase(pos.row);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (non_empty_rows_.empty()) {
        return {0, 0};
    }
    Size result;
    result.rows = *non_empty_rows_.rbegin() + 1;
    for (auto &row : rows_) {
        result.cols = std::max(result.cols, static_cast<int>(row.second.Size()));
    }
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    auto end = rows_.end();
    for (auto i = 0; i < size.rows; ++i) {
        auto row = rows_.find(i);
        if (row != end) {
            row->second.PrintValues(output, size.cols);
        } else {
            Row::PrintEmptyRow(output, size.cols);
        }
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    auto end = rows_.end();
    for (auto i = 0; i < size.rows; ++i) {
        auto row = rows_.find(i);
        if (row != end) {
            row->second.PrintTexts(output, size.cols);
        } else {
            Row::PrintEmptyRow(output, size.cols);
        }
    }
}

Cell* Sheet::GetCellOrCreate(Position pos) {
    TestPosition(pos);
    if (GetCell(pos) == nullptr) {
        SetCell(pos, ""s);
    }
    return dynamic_cast<Cell*>(GetCell(pos));
}

void Sheet::TestPosition(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException{"Invalid position"s};
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Row::SetCell(size_t col, std::string text, Sheet &sheet) {
    // если в текущей позиции еще не было ячейки - создаём
    if (auto cell = data_.find(col); cell == data_.end()) {
        data_[col] = std::make_unique<Cell>(sheet);
    }
    data_[col]->Set(text);
    if (!text.empty()) {
        non_empty_cells_.insert(col);
    }
}

CellInterface* Row::operator[](size_t col) {
    auto cell = data_.find(col);
    if (cell == data_.end()) {
        return nullptr;
    }
    return cell->second.get();
}

const CellInterface* Row::operator[](size_t col) const {
    auto cell = data_.find(col);
    if (cell == data_.end()) {
        return nullptr;
    }
    return cell->second.get();
}

void Row::Clear(size_t col) {
    auto cell = data_.find(col);
    if (cell != data_.end()) {
        cell->second->Clear();
        non_empty_cells_.erase(col);
        // если у ячейки нет детей - удаляем её полностью
        // иначе нет, чтобы сохранить список детей
        if (!cell->second->IsReferenced()) {
            data_.erase(cell);
        }
    }
}

void Row::PrintValues(std::ostream &output, size_t total_cells) const {
    for (size_t i = 0; i < total_cells; ++i) {
        auto cell = data_.find(i);
        if (cell != data_.end()) {
            output << cell->second->GetValue();
        }
        if (i < total_cells - 1) {
            output << '\t';
        }
    }
    output << '\n';
}

void Row::PrintTexts(std::ostream &output, size_t total_cells) const {
    for (size_t i = 0; i < total_cells; ++i) {
        auto cell = data_.find(i);
        if (cell != data_.end()) {
            output << cell->second->GetText();
        }
        if (i < total_cells - 1) {
            output << '\t';
        }
    }
    output << '\n';
}

size_t Row::Size() const {
    if (non_empty_cells_.empty()) {
        return 0;
    } else {
        return *non_empty_cells_.rbegin() + 1;
    }
}

void Row::PrintEmptyRow(std::ostream &output, size_t total_cells) {
    for (size_t i = 0; i < total_cells - 1; ++i) {
        output << '\t';
    }
    output << '\n';
}


