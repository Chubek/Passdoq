#include "indexing.hpp"
#include <filesystem>
#include <sstream>

namespace passdoq {
namespace indexing {

IndexManager::IndexManager(const std::string& index_path)
    : index_path_(index_path) {
}

IndexManager::~IndexManager() {
    close();
}

bool IndexManager::open() {
    try {
        if (!exists()) {
            return create();
        }
        
        db_ = std::make_unique<Xapian::WritableDatabase>(
            index_path_, Xapian::DB_OPEN);
        return true;
        
    } catch (const Xapian::Error& e) {
        return false;
    }
}

bool IndexManager::create() {
    try {
        // Create directory if needed
        std::filesystem::create_directories(
            std::filesystem::path(index_path_).parent_path());
        
        db_ = std::make_unique<Xapian::WritableDatabase>(
            index_path_, Xapian::DB_CREATE_OR_OPEN);
        return true;
        
    } catch (const Xapian::Error& e) {
        return false;
    }
}

void IndexManager::close() {
    if (db_) {
        try {
            db_->commit();
        } catch (const Xapian::Error&) {
            // Ignore errors on close
        }
        db_.reset();
    }
}

bool IndexManager::index_entry(const vault::Entry& entry) {
    if (!db_) {
        return false;
    }
    
    try {
        auto doc = entry_to_document(entry);
        db_->add_document(doc);
        db_->commit();
        return true;
        
    } catch (const Xapian::Error& e) {
        return false;
    }
}

bool IndexManager::update_entry(const vault::Entry& entry) {
    if (!db_) {
        return false;
    }
    
    try {
        auto doc = entry_to_document(entry);
        std::string doc_id = get_doc_id(entry.id);
        db_->replace_document(doc_id, doc);
        db_->commit();
        return true;
        
    } catch (const Xapian::Error& e) {
        return false;
    }
}

bool IndexManager::remove_entry(const std::string& entry_id) {
    if (!db_) {
        return false;
    }
    
    try {
        std::string doc_id = get_doc_id(entry_id);
        db_->delete_document(doc_id);
        db_->commit();
        return true;
        
    } catch (const Xapian::Error& e) {
        return false;
    }
}

bool IndexManager::rebuild_index(const std::vector<vault::Entry>& entries) {
    if (!clear()) {
        return false;
    }
    
    if (!db_) {
        if (!create()) {
            return false;
        }
    }
    
    try {
        for (const auto& entry : entries) {
            auto doc = entry_to_document(entry);
            db_->add_document(doc);
        }
        db_->commit();
        return true;
        
    } catch (const Xapian::Error& e) {
        return false;
    }
}

bool IndexManager::clear() {
    close();
    
    try {
        if (exists()) {
            std::filesystem::remove_all(index_path_);
        }
        return create();
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool IndexManager::exists() const {
    return std::filesystem::exists(index_path_);
}

Xapian::Document IndexManager::entry_to_document(const vault::Entry& entry) {
    Xapian::Document doc;
    Xapian::TermGenerator term_gen;
    
    term_gen.set_document(doc);
    
    // Index name (with higher weight)
    term_gen.index_text(entry.name, 2);
    
    // Index username
    term_gen.index_text(entry.username, 1);
    
    // Index tags
    for (const auto& tag : entry.tags) {
        term_gen.index_text(tag, 1);
        doc.add_boolean_term("TAG:" + tag);
    }
    
    // Index metadata
    for (const auto& pair : entry.metadata) {
        term_gen.index_text(pair.first + " " + pair.second, 1);
        doc.add_value(0, pair.first + ":" + pair.second);
    }
    
    // Store entry ID as data
    doc.set_data(entry.id);
    
    // Add unique term for document ID
    doc.add_boolean_term(get_doc_id(entry.id));
    
    return doc;
}

std::string IndexManager::get_doc_id(const std::string& entry_id) const {
    return "Q" + entry_id;
}

} // namespace indexing
} // namespace passdoq
