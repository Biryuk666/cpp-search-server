#include "search_server.h"

#include <stdexcept>

using namespace std;

SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)) {}

SearchServer::SearchServer(string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text.data())) {}

SearchServer::Iterator SearchServer::begin() {
    return document_ids_.begin();
}

SearchServer::Iterator SearchServer::end() {
    return document_ids_.end();
}

SearchServer::ConstIterator SearchServer::begin() const {
    return cbegin();
}

SearchServer::ConstIterator SearchServer::end() const {
    return cend();
}

SearchServer::ConstIterator SearchServer::cbegin() const {
    return document_ids_.begin();
}

SearchServer::ConstIterator SearchServer::cend() const {
    return document_ids_.end();
}

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document.data());

    const double inv_word_count = 1.0 / static_cast<double>(words.size());
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        id_to_word_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
    return SearchServer::FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
        return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

    int SearchServer::GetDocumentCount() const {
        return static_cast<int>(documents_.size());
}

    int SearchServer::GetDocumentId(int index) const {
        return document_ids_.at(static_cast<size_t>(index));
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
    if (id_to_word_freqs_.find(document_id) == id_to_word_freqs_.end()) {
        throw out_of_range("Invalid document id"s);
    }
    const auto query = ParseQuery(raw_query.data());

    vector<string_view> matched_words;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return {matched_words, documents_.at(document_id).status};
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static map<string_view, double> empty_map;    
    auto it = id_to_word_freqs_.find(document_id);
    if (it != id_to_word_freqs_.end()) {
        return it->second;
    }

    return empty_map;
}

void SearchServer::RemoveDocument(int document_id) {
    auto it = id_to_word_freqs_.find(document_id);
    if (it == id_to_word_freqs_.end()) return;
    
    for (auto [word, frequency] : it->second) {
        word_to_document_freqs_.at(word.data()).erase(document_id);
    }

    id_to_word_freqs_.erase(document_id);
    documents_.erase(document_id);

    auto iterator = find(begin(), end(), document_id);
    document_ids_.erase(iterator);
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + text + " is invalid");
    }

    return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query result;
    for (const std::string& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / static_cast<double>(word_to_document_freqs_.at(word).size()));
}