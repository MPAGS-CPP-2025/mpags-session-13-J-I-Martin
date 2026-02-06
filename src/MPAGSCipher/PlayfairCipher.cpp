#include "PlayfairCipher.hpp"

#include <algorithm>
#include <iostream>
#include <string>

/**
 * \file PlayfairCipher.cpp
 * \brief Contains the implementation of the PlayfairCipher class
 */

PlayfairCipher::PlayfairCipher(const std::string& key)
{
    this->setKey(key);
}

void PlayfairCipher::setKey(const std::string& key)
{
    // Store the original key
    key_ = key;

    // Append the alphabet to the key
    key_ += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Make sure the key is upper case
    std::transform(std::begin(key_), std::end(key_), std::begin(key_),
                   ::toupper);

    // Remove non-alphabet characters
    key_.erase(std::remove_if(std::begin(key_), std::end(key_),
                              [](char c) { return !std::isalpha(c); }),
               std::end(key_));

    // Change J -> I
    std::transform(std::begin(key_), std::end(key_), std::begin(key_),
                   [](char c) { return (c == 'J') ? 'I' : c; });

    // Remove duplicated letters
    std::string lettersFound{""};
    auto detectDuplicates = [&](char c) {
        if (lettersFound.find(c) == std::string::npos) {
            lettersFound += c;
            return false;
        } else {
            return true;
        }
    };
    key_.erase(
        std::remove_if(std::begin(key_), std::end(key_), detectDuplicates),
        std::end(key_));

    // Store the coordinates of each letter
    // (at this point the key length must be equal to the square of the grid dimension)
    for (std::size_t i{0}; i < keyLength_; ++i) {
        std::size_t row{i / gridSize_};
        std::size_t column{i % gridSize_};

        auto coords = std::make_pair(row, column);

        charLookup_[key_[i]] = coords;
        coordLookup_[coords] = key_[i];
    }
}

std::string PlayfairCipher::applyCipher(const std::string& inputText,
                                        const CipherMode cipherMode) const
{
    // Create the output string, initially a copy of the input text
    std::string outputText{inputText};

    // Change J -> I
    std::transform(std::begin(outputText), std::end(outputText),
                   std::begin(outputText),
                   [](char c) { return (c == 'J') ? 'I' : c; });

    // Find repeated characters and add an X (or a Q for repeated X's)
    std::string processedText{""};
    for (std::size_t i{0}, inc{2}; i < outputText.size(); i += inc) {
        // prevent moving out of loop
        if (i + 1 >= outputText.size()) {
            // means the last character is ith position
            processedText += outputText[i];
            break;
        }
        // normal processing of bigrams
        char c1{outputText[i]};
        char c2{outputText[i + 1]};
        if (c1 == c2) {
            processedText += c1;
            processedText += (c1 == 'X') ? 'Q' : 'X';
            inc = 1;
        } else {
            processedText += c1;
            processedText += c2;
            inc = 2;
        }
    }

    // If the size of the input is odd, add a trailing Z
    if (processedText.size() % 2 != 0) {
        (processedText[-1] != 'Z') ? processedText += 'X'
                                   : processedText += 'Z';
    }
    // Loop over the input bigrams
    for (std::size_t i{0}; i < processedText.size();
         i += 2) {    // here i+=2 as static inc
        // Get the characters
        char c1{processedText[i]};
        char c2{processedText[i + 1]};

        // - Find the coordinates in the grid for each bigram
        using MapCoord = std::pair<std::size_t, std::size_t>;
        MapCoord coords1{charLookup_.at(c1)};
        auto& [row1, col1] = coords1;
        MapCoord coords2{charLookup_.at(c2)};
        auto& [row2, col2] = coords2;

        // - Apply the rules to these coords to get new coords
        size_t shift{1};
        if (cipherMode == CipherMode::Decrypt) {
            shift *= -1;
        }
        if (row1 == row2) {
            // same row, shift columns
            col1 = (col1 + shift) % gridSize_;
            col2 = (col2 + shift) % gridSize_;
        } else if (col1 == col2) {
            // same column, shift rows
            row1 = (row1 + shift) % gridSize_;
            row2 = (row2 + shift) % gridSize_;
        } else {
            // rectangle, swap columns
            std::swap(col1, col2);
        }

        // - Find the letters associated with the new coords
        char newC1{coordLookup_.at(coords1)};
        char newC2{coordLookup_.at(coords2)};

        // - Make the replacements
        processedText[i] = newC1;
        processedText[i + 1] = newC2;
    }
    // Return the output text
    return processedText;
}
