#ifndef PASSDOQ_INDEXING_HPP
#define PASSDOQ_INDEXING_HPP

#include "vault.hpp"
#include <string>
#include <memory>
#include <xapian.h>

namespace passdoq {
namespace indexing {

// Index manager for vault entries
class IndexManager {
public:
    IndexManager(const std::string& index_path);
    ~IndexManager();
    
    // Index operations
    bool open();
    bool create();
    void close();
    
    // Add/update/remove entries
    bool index_entry(const vault::Entry& entry);
    bool update_entry(const vault::Entry& entry);
    bool remove_entry(const std::string& entry_id);
    
    // Rebuild entire index
    bool rebuild_index(const std::vector<vault::Entry>& entries);
    
    // Clear index
    bool clear();
    
    // Check if index exists
    bool exists() const;
    
private:
    std::string index_path_;
    std::unique_ptr<Xapian::WritableDatabase> db_;
    
    // Convert entry to Xapian document
    Xapian::Document entry_to_document(const vault::Entry& entry);
    
    // Generate document ID from entry ID
    std::string get_doc_id(const std::string& entry_id) const;
};

} // namespace indexing
} // namespace passdoq

#endif // PASSDOQ_INDEXING_HPP
