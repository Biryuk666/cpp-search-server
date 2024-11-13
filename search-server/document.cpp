#include "document.h"

using namespace std;

Document::Document() = default;

Document::Document(int id_, double relevance_, int rating_)
    : id(id_), relevance(relevance_), rating(rating_)
{
}

ostream& operator << (ostream& out, Document document) {
    return out << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s
    << document.rating << " }"s;
}