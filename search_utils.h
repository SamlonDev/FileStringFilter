#ifndef SEARCH_UTILS_H
#define SEARCH_UTILS_H

#include <vector>
#include <string>

class MultiPatternSearch {
private:
    std::vector<std::string> patterns;
    std::vector<std::vector<int>> skip_tables;
    
    void build_skip_table(const std::string& pattern, std::vector<int>& skip_table);
    bool search_pattern(const char* text, size_t text_len, 
                       const std::string& pattern, 
                       const std::vector<int>& skip_table) const;

public:
    MultiPatternSearch() = default;
    
    void add_pattern(const std::string& pattern);
    void clear_patterns();
    size_t pattern_count() const;
    
    bool search_any(const char* text, size_t text_len) const;
    bool search_any(const std::string& text) const;
};

#endif // SEARCH_UTILS_H