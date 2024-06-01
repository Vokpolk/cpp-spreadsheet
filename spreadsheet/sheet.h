#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами

private:
	// Можете дополнить ваш класс нужными полями и методами
    //std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    struct Hash {
        size_t operator()(const Position& pos) const {
            return 11 * pos.row + 42 * pos.col;
        }
    };
    std::unordered_map<Position, std::unique_ptr<Cell>, Hash> cells_;
    Size size_{ 0,0 };
};