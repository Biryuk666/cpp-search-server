#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>



using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;

    for (const char& c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }

    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> document_words = SplitIntoWordsNoStop(document);

        for (const string& word : document_words) {
            double tf = static_cast<double>(count(document_words.begin(), document_words.end(), word)) / document_words.size();
            word_to_document_freqs_[word].insert({document_id, tf});
        }

        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query& query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

private:
   
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
                
        for (string word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                query_words.minus_words.insert(word.substr(1));
            } else {
                query_words.plus_words.insert(word);
            }
        }

        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        ector<Document> matched_documents;
        map<int, double> document_to_relevance;
        
        if (!query_words.plus_words.empty() || !document_to_relevance.empty()) {
            for (const string& query_word : query_words.plus_words) {
                if (word_to_document_freqs_.count(query_word) != 0 && query_words.minus_words.count(query_word) == 0) {
                    double idf = log(document_count_ / static_cast<double>(word_to_document_freqs_.at(query_word).size()));
                    for (const auto& [document_id, tf] : word_to_document_freqs_.at(query_word)) {
                        document_to_relevance[document_id] += tf * idf;
                    }
                }
            }
        } else {
            return {};
        }
 
        for (const auto& [id, relevance] : document_to_relevance) {
            matched_documents.push_back({id, relevance});
        }

        return matched_documents;
    }

};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}


int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }

}