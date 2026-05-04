#include "runcom.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <tao/pegtl.hpp>

namespace passdoq {
namespace runcom {

namespace peg = tao::pegtl;

// PRL Grammar
namespace prl_grammar {
    struct ws : peg::star<peg::space> {};
    struct comment : peg::seq<peg::one<'#'>, peg::until<peg::eolf>> {};
    struct ignored : peg::sor<ws, comment> {};
    
    struct identifier : peg::identifier {};
    struct quoted_string : peg::seq<peg::one<'"'>, peg::until<peg::one<'"'>>> {};
    struct unquoted_string : peg::plus<peg::not_one<' ', '\t', '\n', '\r', '#'>> {};
    struct string_value : peg::sor<quoted_string, unquoted_string> {};
    
    struct integer : peg::seq<peg::opt<peg::one<'-'>>, peg::plus<peg::digit>> {};
    struct boolean : peg::sor<peg::string<'t','r','u','e'>, 
                               peg::string<'f','a','l','s','e'>,
                               peg::string<'y','e','s'>,
                               peg::string<'n','o'>,
                               peg::one<'1'>,
                               peg::one<'0'>> {};
    
    struct list_item : peg::seq<ws, string_value, ws> {};
    struct list_value : peg::seq<peg::one<'['>, 
                                  peg::list<list_item, peg::one<','>, ws>,
                                  peg::one<']'>> {};
    
    struct value : peg::sor<list_value, boolean, integer, string_value> {};
    
    struct assignment : peg::seq<identifier, ws, peg::one<'='>, ws, value> {};
    struct statement : peg::seq<ws, peg::opt<assignment>, ws, peg::opt<comment>, peg::eol> {};
    
    struct grammar : peg::until<peg::eof, statement> {};
}

// PRL Parser Actions
template<typename Rule>
struct prl_action : peg::nothing<Rule> {};

struct ParseState {
    std::map<std::string, Value>* values;
    std::string current_key;
    std::string current_string;
    std::vector<std::string> current_list;
    bool in_list = false;
};

template<>
struct prl_action<prl_grammar::identifier> {
    template<typename Input>
    static void apply(const Input& in, ParseState& state) {
        state.current_key = in.string();
    }
};

template<>
struct prl_action<prl_grammar::string_value> {
    template<typename Input>
    static void apply(const Input& in, ParseState& state) {
        std::string val = in.string();
        if (val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.length() - 2);
        }
        
        if (state.in_list) {
            state.current_list.push_back(val);
        } else {
            state.current_string = val;
        }
    }
};

template<>
struct prl_action<prl_grammar::integer> {
    template<typename Input>
    static void apply(const Input& in, ParseState& state) {
        if (!state.current_key.empty()) {
            (*state.values)[state.current_key] = Value::from_int(std::stoll(in.string()));
        }
    }
};

template<>
struct prl_action<prl_grammar::boolean> {
    template<typename Input>
    static void apply(const Input& in, ParseState& state) {
        if (!state.current_key.empty()) {
            std::string val = in.string();
            bool b = (val == "true" || val == "yes" || val == "1");
            (*state.values)[state.current_key] = Value::from_bool(b);
        }
    }
};

template<>
struct prl_action<prl_grammar::list_value> {
    template<typename Input>
    static void apply(const Input& in, ParseState& state) {
        if (!state.current_key.empty()) {
            (*state.values)[state.current_key] = Value::from_list(state.current_list);
            state.current_list.clear();
        }
    }
};

template<>
struct prl_action<prl_grammar::assignment> {
    template<typename Input>
    static void apply(const Input& in, ParseState& state) {
        if (!state.current_key.empty() && !state.current_string.empty()) {
            (*state.values)[state.current_key] = Value::from_string(state.current_string);
            state.current_string.clear();
        }
        state.current_key.clear();
    }
};

// Value implementations
Value Value::from_string(const std::string& s) {
    Value v;
    v.type = ValueType::STRING;
    v.string_value = s;
    return v;
}

Value Value::from_int(int64_t i) {
    Value v;
    v.type = ValueType::INTEGER;
    v.int_value = i;
    return v;
}

Value Value::from_bool(bool b) {
    Value v;
    v.type = ValueType::BOOLEAN;
    v.bool_value = b;
    return v;
}

Value Value::from_list(const std::vector<std::string>& l) {
    Value v;
    v.type = ValueType::LIST;
    v.list_value = l;
    return v;
}

// Config implementation
Config::Config() {}

bool Config::load(const std::string& path) {
    if (is_circular_include(path)) {
        return false;
    }
    
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    loaded_files_.push_back(path);
    
    // Preprocess
    std::string preprocessed = preprocess(content, 
                                          std::filesystem::path(path).parent_path().string());
    
    // Parse
    return parse_prl(preprocessed, std::filesystem::path(path).parent_path().string());
}

bool Config::load_defaults() {
    auto paths = get_default_paths();
    
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            if (load(path)) {
                return true;
            }
        }
    }
    
    return false;
}

std::string Config::get_string(const std::string& key, const std::string& default_val) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return default_val;
    }
    
    if (it->second.type == ValueType::STRING) {
        return it->second.string_value;
    }
    
    return default_val;
}

int64_t Config::get_int(const std::string& key, int64_t default_val) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return default_val;
    }
    
    if (it->second.type == ValueType::INTEGER) {
        return it->second.int_value;
    }
    
    return default_val;
}

bool Config::get_bool(const std::string& key, bool default_val) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return default_val;
    }
    
    if (it->second.type == ValueType::BOOLEAN) {
        return it->second.bool_value;
    }
    
    return default_val;
}

std::vector<std::string> Config::get_list(const std::string& key) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return {};
    }
    
    if (it->second.type == ValueType::LIST) {
        return it->second.list_value;
    }
    
    return {};
}

void Config::set(const std::string& key, const Value& value) {
    values_[key] = value;
}

void Config::set_string(const std::string& key, const std::string& value) {
    values_[key] = Value::from_string(value);
}

void Config::set_int(const std::string& key, int64_t value) {
    values_[key] = Value::from_int(value);
}

void Config::set_bool(const std::string& key, bool value) {
    values_[key] = Value::from_bool(value);
}

void Config::set_list(const std::string& key, const std::vector<std::string>& value) {
    values_[key] = Value::from_list(value);
}

bool Config::has(const std::string& key) const {
    return values_.find(key) != values_.end();
}

std::vector<std::string> Config::keys() const {
    std::vector<std::string> result;
    for (const auto& pair : values_) {
        result.push_back(pair.first);
    }
    return result;
}

bool Config::parse_prl(const std::string& content, const std::string& base_path) {
    try {
        ParseState state;
        state.values = &values_;
        
        peg::memory_input input(content, "runcom");
        return peg::parse<prl_grammar::grammar, prl_action>(input, state);
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string Config::preprocess(const std::string& content, const std::string& base_path) {
    // Simple include handling (ucpp would be used in production)
    std::istringstream iss(content);
    std::ostringstream oss;
    std::string line;
    
    while (std::getline(iss, line)) {
        // Check for #include directive
        if (line.find("#include") == 0) {
            size_t start = line.find('"');
            size_t end = line.rfind('"');
            
            if (start != std::string::npos && end != std::string::npos && start < end) {
                std::string include_file = line.substr(start + 1, end - start - 1);
                std::filesystem::path include_path = std::filesystem::path(base_path) / include_file;
                
                if (!is_circular_include(include_path.string())) {
                    std::ifstream inc_file(include_path);
                    if (inc_file) {
                        std::string inc_content((std::istreambuf_iterator<char>(inc_file)),
                                                std::istreambuf_iterator<char>());
                        loaded_files_.push_back(include_path.string());
                        oss << preprocess(inc_content, include_path.parent_path().string());
                    }
                }
            }
        } else {
            oss << line << '\n';
        }
    }
    
    return oss.str();
}

bool Config::is_circular_include(const std::string& path) const {
    return std::find(loaded_files_.begin(), loaded_files_.end(), path) != loaded_files_.end();
}

std::vector<std::string> get_default_paths() {
    std::vector<std::string> paths;
    
    // $PASSDOQ_RUNCOM_FILE
    const char* env_file = std::getenv("PASSDOQ_RUNCOM_FILE");
    if (env_file) {
        paths.push_back(env_file);
    }
    
    // $XDG_CONFIG_HOME/passdoq/Passdoq.cnf
    const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config) {
        paths.push_back(std::string(xdg_config) + "/passdoq/Passdoq.cnf");
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            paths.push_back(std::string(home) + "/.config/passdoq/Passdoq.cnf");
        }
    }
    
    // ~/.passdoqrc
    const char* home = std::getenv("HOME");
    if (home) {
        paths.push_back(std::string(home) + "/.passdoqrc");
    }
    
    return paths;
}

} // namespace runcom
} // namespace passdoq
