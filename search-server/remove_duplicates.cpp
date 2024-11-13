#include "remove_duplicates.h"

#include <iostream>
#include <set>
#include <string_view>

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
    set<int> id_to_remove;
    set<set<string_view>> unique_words_sets;
    for (const int document_id : search_server) {
        set<string_view> unique_words;
        for (const auto& [word, id] : search_server.GetWordFrequencies(document_id)) {
            unique_words.insert(word);
        }
        if (unique_words_sets.count(unique_words)) {
            id_to_remove.insert(document_id);
        } else {
            unique_words_sets.insert(unique_words);
        }
    }

    for (int document_id : id_to_remove) {        
        cout << "Found duplicate document id " << document_id << endl;
        search_server.RemoveDocument(document_id);
    }
}