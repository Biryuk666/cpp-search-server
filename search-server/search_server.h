#pragma once

#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

using std::literals::string_literals::operator""s;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);
    explicit SearchServer(const std::string& stop_words_text);
    explicit SearchServer(std::string_view stop_words_text);

    using Iterator = std::vector<int>::iterator;
    using ConstIterator = std::vector<int>::const_iterator;
    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;
    ConstIterator cbegin() const;
    ConstIterator cend() const;
    
    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const ;
    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    int GetDocumentCount() const;
    int GetDocumentId(int index) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;

    template <typename ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const ExecutionPolicy& policy, std::string_view raw_query, int document_id) const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    template <typename ExecutionPolicy>
    void RemoveDocument(const ExecutionPolicy& policy, int document_id);

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };        

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };
    
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> id_to_word_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;    

    bool IsStopWord(const std::string& word) const;
    static bool IsValidWord(const std::string& word);
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);
    QueryWord ParseQueryWord(const std::string& text) const;
    Query ParseQuery(const std::string& text) const;
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
{
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentPredicate document_predicate) const {
    const auto query = ParseQuery(raw_query.data());

    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(std::execution::par, matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                return lhs.rating > rhs.rating;
            }
            return lhs.relevance > rhs.relevance;
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto &[document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto &[document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

template <typename ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const ExecutionPolicy& policy, std::string_view raw_query, int document_id) const {
    if (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        return MatchDocument(raw_query, document_id);
    }

    if (id_to_word_freqs_.find(document_id) == id_to_word_freqs_.end()) {
        throw std::out_of_range("Invalid document id"s);
    }
    const auto query = ParseQuery(raw_query.data());

    std::vector<std::string_view> matched_words;
    for_each (policy, query.plus_words.begin(), query.plus_words.end(), [&](const auto& word) {
        auto it = word_to_document_freqs_.find(word);
        if (it != word_to_document_freqs_.end() && it->second.find(document_id) != it->second.end()) {
            matched_words.push_back(word);
        }
    });

    for_each (policy, query.minus_words.begin(), query.minus_words.end(), [&](const auto& word) {
        auto it = word_to_document_freqs_.find(word);
        if (it != word_to_document_freqs_.end() && it->second.find(document_id) != it->second.end()) {
            matched_words.clear();
        }
    });

    return {matched_words, documents_.at(document_id).status};
}

template <typename ExecutionPolicy>
void SearchServer::RemoveDocument(const ExecutionPolicy& policy, int document_id) {
    if (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        RemoveDocument(document_id);
    } else {
        auto it = id_to_word_freqs_.find(document_id);
        if (it == id_to_word_freqs_.end()) return;
        
        for_each (policy, it->second.begin(), it->second.end(), [&](const auto& word_to_id) {
            word_to_document_freqs_.at(word_to_id.first.data()).erase(document_id);
        });

        id_to_word_freqs_.erase(document_id);
        documents_.erase(document_id);

        auto iterator = find(policy, begin(), end(), document_id);
        document_ids_.erase(iterator);
    }
}