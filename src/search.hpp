#ifndef PASSDOQ_SEARCH_HPP
#define PASSDOQ_SEARCH_HPP

#include <string>
#include <vector>
#include <memory>
#include <xapian.h>

namespace passdoq {
namespace search {

// Search result
struct SearchResult {
    std::string entry_id;
    double relevance;
    std::string snippet;
};

// Search query builder
class QueryBuilder {
public:
    QueryBuilder();
    
    // Add search terms
    QueryBuilder& add_term(const std::string& term);
    QueryBuilder& add_phrase(const std::string& phrase);
    
    // Filter by tag
    QueryBuilder& filter_tag(const std::string& tag);
    
    // Filter by metadata
    QueryBuilder& filter_metadata(const std::string& key, const std::string& value);
    
    // Build the query
    Xapian::Query build() const;
    
private:
    std::vector<Xapian::Query> queries_;
    std::vector<Xapian::Query> filters_;
};

// Search engine
class SearchEngine {
public:
    SearchEngine(const std::string& index_path);
    ~SearchEngine();
    
    // Open index
    bool open();
    void close();
    
    // Search operations
    std::vector<SearchResult> search(const std::string& query_string, 
                                      size_t max_results = 10);
    
    std::vector<SearchResult> search(const Xapian::Query& query,
                                      size_t max_results = 10);
    
    // Advanced search
    std::vector<SearchResult> search_by_tag(const std::string& tag,
                                             size_t max_results = 10);
    
    std::vector<SearchResult> search_metadata(const std::string& key,
                                               const std::string& value,
                                               size_t max_results = 10);
    
    // Get suggestions
    std::vector<std::string> suggest(const std::string& prefix, size_t max_suggestions = 5);
    
private:
    std::string index_path_;
    std::unique_ptr<Xapian::Database> db_;
    
    // Parse query string
    Xapian::Query parse_query(const std::string& query_string);
    
    // Convert match set to results
    std::vector<SearchResult> matches_to_results(const Xapian::MSet& matches);
};

} // namespace search
} // namespace passdoq

#endif // PASSDOQ_SEARCH_HPP
