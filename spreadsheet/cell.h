#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

private:
    //можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual ~Impl() = default;

        virtual Value GetValue() = 0;
        virtual std::string GetText() const = 0;

        virtual std::vector<Position> GetReferencedCells() const;

        virtual std::optional<FormulaInterface::Value> GetCache() const;
        virtual void ResetCache();

    };
    class EmptyImpl : public Impl {
    public:
        Value GetValue() override;
        std::string GetText() const override;
    private:
    };
    class TextImpl : public Impl {
    public:
        TextImpl(std::string expression);

        Value GetValue() override;
        std::string GetText() const override;
    private:
        std::string value_;
    };
    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string exprtession, const SheetInterface& sheet);
        Value GetValue() override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

        std::optional<FormulaInterface::Value> GetCache() const override;
        void ResetCache() override;
    private:
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };
    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;

    void CyclicDependency(const std::vector<Position>& ref_cells) const;
    void CyclicDependencyRecur(const std::vector<Position>& ref_cells, std::unordered_set<CellInterface*>& visited_cells) const;

    void InvalidateCache();
    void InvalidateCacheRecur(std::unordered_set<Cell*>& visited_cells);

    void UpdateDependencies(const std::vector<Position>& new_cells);

    std::unordered_set<Cell*> dependent_cells_; //ячейки которые ссылаются на текущую ячейку
    std::unordered_set<Cell*> referenced_cells_; //зависимые ячейки от текущей ячейки
};