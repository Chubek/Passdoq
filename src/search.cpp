#include "search.hpp"
#include <algorithm>

namespace passdoq {
namespace search {

// QueryBuilder implementation
QueryBuilder::QueryBuilder() {}

QueryBuilder& QueryBuilder::add_term(const std::string& term) {
    queries_.push_back(Xapian::Query(term));
    return *this;
}

QueryBuilder& QueryBuilder::add_phrase(const std::string& phrase) {
    queries_.push_back(Xapian::Query(phrase, 1, 0));
    return *this;
}

QueryBuilder& QueryBuilder::filter_tag(const std::string& tag) {
    filters_.push_back(Xapian::Query("TAG:" + tag));
    return *this;
}

QueryBuilder& QueryBuilder::filter_metadata(const std::string& key, const std::string& value) {
    filters_.push_back(Xapian::Query(key + ":" + value));
    return *this;
}

Xapian::Query QueryBuilder::build() const {
    if (queries_.empty() && filters_.empty()) {
        return Xapian::Query::MatchAll;
    }
    
    Xapian::Query main_query;
    
    if (!queries_.empty()) {
        main_query = Xapian::Query(Xapian::Query::OP_OR, 
                                    queries_.begin(), queries_.end());
    } else {
        main_query = Xapian::Query::MatchAll;
    }
    
    if (!filters_.empty()) {
        Xapian::Query filter_query = Xapian::Query(Xapian::Query::OP_AND,
                                                     filters_.begin(), filters_.end());
        main_query = Xapian::Query(Xapian::Query::OP_FILTER,
                                    main_query, filter_query);
    }
    
    return main_query;
}

// SearchEngine implementation
SearchEngine::SearchEngine(const std::string& index_path)
    : index_path_(index_path) {
}

SearchEngine::~SearchEngine() {
    close();
}

bool SearchEngine::open() {
    try {
        db_ = std::make_unique<Xapian::Database>(index_path_);
        return true;
    } catch (const Xapian::Error& e) {
        return false;
    }
}

void SearchEngine::close() {
    db_.reset();
}

std::vector<SearchResult> SearchEngine::search(const std::string& query_string,
                                                size_t max_results) {
    if (!db_) {
        return {};
    }
    
    try {
        auto query = parse_query(query_string);
        return search(query, max_results);
    } catch (const Xapian::Error& e) {
        return {};
    }
}

std::vector<SearchResult> SearchEngine::search(const Xapian::Query& query,
                                                size_t max_results) {
    if (!db_) {
        return {};
    }
    
    try {
        Xapian::Enquire enquire(*db_);
        enquire.set_query(query);
        
        Xapian::MSet matches = enquire.get_mset(0, max_results);
        
        return matches_to_results(matches);
        
    } catch (const Xapian::Error& e) {
        return {};
    }
}

std::vector<SearchResult> SearchEngine::search_by_tag(const std::string& tag,
                                                       size_t max_results) {
    QueryBuilder builder;
    builder.filter_tag(tag);
    return search(builder.build(), max_results);
}

std::vector<SearchResult> SearchEngine::search_metadata(const std::string& key,
                                                         const std::string& value,
                                                         size_t max_results) {
    QueryBuilder builder;
    builder.filter_metadata(key, value);
    return search(builder.build(), max_results);
}

std::vector<std::string> SearchEngine::suggest(const std::string& prefix,
                                                size_t max_suggestions) {
    if (!db_) {
        return {};
    }
    
    std::vector<std::string> suggestions;
    
    try {
        // Get all terms starting with prefix
        for (auto it = db_->allterms_begin(prefix);
             it != db_->allterms_end(prefix) && suggestions.size() < max_suggestions;
             ++it) {
            suggestions.push_back(*it);
        }
        
    } catch (const Xapian::Error& e) {
        return {};
    }
    
    return suggestions;
}

Xapian::Query SearchEngine::parse_query(const std::string& query_string) {
    Xapian::QueryParser parser;
    parser.set_database(*db_);
    parser.set_stemmer(Xapian::Stem("english"));
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    
    return parser.parse_query(query_string,
                               Xapian::QueryParser::FLAG_DEFAULT |
                               Xapian::QueryParser::FLAG_WILDCARD);
}

std::vector<SearchResult> SearchEngine::matches_to_results(const Xapian::MSet& matches) {
    std::vector<SearchResult> results;
    
    for (auto it = matches.begin(); it != matches.end(); ++it) {
        SearchResult result;
        result.entry_id = it.get_document().get_data();
        result.relevance = it.get_percent() / 100.0;
        
        // Generate snippet (first 100 chars of document data)
        std::string data = it.get_document().get_data();
        if (data.length() > 100) {
            result.snippet = data.substr(0, 97) + "...";
        } else {
            result.snippet = data;
        }
        
        results.push_back(result);
    }
    
    return results;
}

} // namespace search
} // namespace passdoq
