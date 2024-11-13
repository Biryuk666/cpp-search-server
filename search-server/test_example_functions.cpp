#include "test_example_functions.h"

#include <iostream>

using namespace std;
 
void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status, const vector<int>& ratings){
    try{
        search_server.AddDocument(document_id, document, status, ratings);
    }catch(const std::invalid_argument& e){
        cout << "The document could not be added"s << document_id << ": "s << e.what() << endl;
    }
}