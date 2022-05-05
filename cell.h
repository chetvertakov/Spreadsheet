#pragma once

#include "common.h"

#include <set>

class Sheet;

class Cell final : public CellInterface {
public:
    Cell(Sheet &sheet);
    ~Cell() override;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    Sheet &sheet_;
    std::unique_ptr<Impl> impl_;

    // список ячеек, от которых зависит данная
    std::set<Cell*> parents_;
    // список ячеек, которые зависят от данной
    std::set<Cell*> children_;

    // устанавливает родителей ячейки
    void SetParents();
    // удаляет родителей ячейки
    void DeleteParents();

    void ClearCache();
    // сбрасывает кэш у ячейки и всех её детей
    void ClearChildrenCache();

    // проверяет новую формулу на циклы в таблице
    void TestCycles(const std::vector<Position> &parents, const Cell *root);

};


