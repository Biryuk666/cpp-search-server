#pragma once
#include <iostream>
#include <string>

using std::literals::string_literals::operator""s;

struct Document {
    Document();
    Document(int id, double relevance, int rating);

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

std::ostream& operator << (std::ostream& out, Document document);