#include "search_utils.h"
#include <algorithm>

void MultiPatternSearch::build_skip_table(const std::string& pattern, std::vector<int>& skip_table) {
    skip_table.assign(256, static_cast<int>(pattern.length()));
    for (int i = 0; i < static_cast<int>(pattern.length()) - 1; ++i) {
        skip_table[static_cast<unsigned char>(std::tolower(pattern[i]))] = 
            static_cast<int>(pattern.length()) - 1 - i;
    }
}

void MultiPatternSearch::add_pattern(const std::string& pattern) {
    if (pattern.empty()) return;
    
    std::string lower_pattern = pattern;
    std::transform(lower_pattern.begin(), lower_pattern.end(), 
                  lower_pattern.begin(), ::tolower);
    patterns.push_back(lower_pattern);
    
    skip_tables.emplace_back();
    build_skip_table(lower_pattern, skip_tables.back());
}

void MultiPatternSearch::clear_patterns() {
    patterns.clear();
    skip_tables.clear();
}

size_t MultiPatternSearch::pattern_count() const {
    return patterns.size();
}

bool MultiPatternSearch::search_any(const char* text, size_t text_len) const {
    for (size_t p = 0; p < patterns.size(); ++p) {
        if (search_pattern(text, text_len, patterns[p], skip_tables[p])) {
            return true;
        }
    }
    return false;
}

bool MultiPatternSearch::search_any(const std::string& text) const {
    return search_any(text.c_str(), text.length());
}

bool MultiPatternSearch::search_pattern(const char* text, size_t text_len, 
                                       const std::string& pattern, 
                                       const std::vector<int>& skip_table) const {
    if (text_len < pattern.length()) return false;
    
    int pattern_len = static_cast<int>(pattern.length());
    
    for (int i = pattern_len - 1; i < static_cast<int>(text_len);) {
        int j = pattern_len - 1;
        int k = i;
        
        while (j >= 0 && std::tolower(text[k]) == pattern[j]) {
            --j;
            --k;
        }
        
        if (j < 0) return true;
        
        i += skip_table[static_cast<unsigned char>(std::tolower(text[i]))];
    }
    
    return false;
}