#include "common.h"

#include <cctype>
#include <cmath>
#include <sstream>

using namespace std::literals;

namespace  {

std::string NumberToLetters(int number);
int LettersToNumber(std::string_view letters);
bool IsCorrect(std::string_view letters, size_t &first_digit_pos);

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const int MAX_LETTERS_COUNT = MAX_POS_LETTER_COUNT;
const int MAX_DIGITS_COUNT = 5;

} // namespace

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return (col >= 0 && col < Position::MAX_COLS
            && row >= 0 && row < Position::MAX_ROWS);
}

std::string Position::ToString() const {
    if (IsValid()) {
        return NumberToLetters(col) + std::to_string(row + 1);
    }
    return {};
}

Position Position::FromString(std::string_view str) {
    size_t num_start;
    if (IsCorrect(str, num_start)) {
        int col = LettersToNumber(str.substr(0, num_start));
        int row = std::stoi(std::string(str.substr(num_start))) - 1;
        Position result{row, col};
        if (result.IsValid()) {
            return result;
        }
    }
    return Position::NONE;
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}

namespace {

std::string NumberToLetters(int number) {
    std::string result;
    if (number >= 0) {
        int alpha = number / 26;
        int rem = number % 26;
        result = ('A' + rem);
        if (alpha > 0) {
            result = NumberToLetters(alpha - 1) + result;
        }
    }
    return result;
}

int LettersToNumber(std::string_view letters) {
    int result = 0;
    auto len = letters.length();
    for (size_t i = 0; i < len; ++i) {
        auto c = letters.at(len - i  - 1);
        result += static_cast<int>(c - 'A' + 1) * std::pow(LETTERS, i);
    }
    return result - 1;
}

bool IsCorrect(std::string_view str, size_t &first_digit_pos) {
    if (str.size() > MAX_POSITION_LENGTH) {
        return false;
    }
    size_t num_letters = 0;
    size_t num_digits = 0;
    for (auto c : str) {
        // сначали идут только заглавные буквы
        if (num_digits == 0) {
            if (std::isupper(c)) {
                ++num_letters;
            } else if (std::isdigit(c)) {
                ++num_digits;
            } else {
                return false;
            }
        // потом только цифры
        } else {
            if (!std::isdigit(c)) {
                return false;
            }
            ++num_digits;
        }
    }
    first_digit_pos = num_letters;
    return num_digits > 0 && num_digits <= MAX_DIGITS_COUNT
            && num_letters > 0 && num_letters <= MAX_LETTERS_COUNT;
}

} // namespace
