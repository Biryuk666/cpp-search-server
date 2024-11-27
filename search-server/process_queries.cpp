#include "process_queries.h"

#include <algorithm>
#include <execution>
#include <numeric>

using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries) {
    vector<vector<Document>> result(queries.size());
    transform(execution::par, queries.begin(), queries.end(), result.begin(), [&] (const string& query) {
        return search_server.FindTopDocuments(query);
    });

    return result;
}

vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const vector<string>& queries) {
    auto documents = ProcessQueries(search_server, queries);
    
    return reduce(execution::par, documents.begin(), documents.end(), vector<Document>{}, [](vector<Document> lhs, const vector<Document>& rhs){
        lhs.insert(lhs.end(), rhs.begin(), rhs.end());
        return lhs;
    });
}