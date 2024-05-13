#include "document.h"

using namespace std;


    Document::Document() = default;

    Document::Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating)
    {
    }

    ostream& operator << (ostream& out, Document document) {
    return out << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s
            << document.rating << " }"s;
}

    




