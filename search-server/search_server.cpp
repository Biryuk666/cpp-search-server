#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        const auto words = SplitIntoWordsNoStop(document);

        const double inv_word_count = 1.0 / static_cast<double>(words.size());
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        return SearchServer::FindTopDocuments(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
        return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int SearchServer::GetDocumentCount() const {
        return static_cast<int>(documents_.size());
    }

    int SearchServer::GetDocumentId(int index) const {
        return document_ids_.at(static_cast<size_t>(index));
}