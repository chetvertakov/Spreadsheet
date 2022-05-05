#pragma once

#include "cell.h"
#include "common.h"

#include <set>
#include <unordered_map>

class Row {
public:
    void SetCell(size_t col, std::string text, Sheet &sheet);

    CellInterface* operator[](size_t col);
    const CellInterface* operator[](size_t col) const;

    void Clear(size_t col);

    void PrintValues(std::ostream& output, size_t total_cells) const;
    void PrintTexts(std::ostream& output, size_t total_cells) const;

    static void PrintEmptyRow(std::ostream& output, size_t total_cells);

    size_t Size() const;
private:
    std::unordered_map<size_t, std::unique_ptr<Cell>> data_;
    std::set<size_t> non_empty_cells_;
};

class Sheet : public SheetInterface {
public:
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // возвращает указатель на ячейку. Если такой ячейки нет - создает пустую и возвращает указатель
    Cell* GetCellOrCreate(Position pos);

private:
    std::unordered_map<size_t, Row> rows_;
    std::set<size_t> non_empty_rows_;

    static void TestPosition(Position pos);
};
