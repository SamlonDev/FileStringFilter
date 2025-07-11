#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <string>
#include <chrono>
#include "file_info.h"
#include "salmonFiltuh.h"
#include "search_utils.h"

class AdaptiveFileProcessor {
private:
    static constexpr size_t SMALL_FILE_THRESHOLD = 50 * 1024 * 1024;   // 50MB
    static constexpr size_t LARGE_FILE_THRESHOLD = 500 * 1024 * 1024;  // 500MB
    
    MultiPatternSearch searcher;
    size_t total_lines;
    size_t matched_lines;
    size_t total_files_processed;
    
    FileInfo analyze_input(const std::string& filename);
    ProcessingMethod select_method(const FileInfo& info);
    void process_small_file(std::istream& input, std::ostream& output);
    void process_large_file(std::istream& input, std::ostream& output);
    void create_default_rules_file(const std::string& rules_file);
    std::string formatNumber(double number) const;
    
public:
    AdaptiveFileProcessor();
    ~AdaptiveFileProcessor() = default;
    
    bool load_patterns(const std::string& rules_file = std::string(NAME) + ".rules");
    void process_directory(const std::string& directory = ".");
    size_t process_file_to_stream(const std::string& input_filename, std::ostream& output);
    void print_batch_stats(const std::chrono::milliseconds& duration, size_t total_size) const;
};

#endif // FILE_PROCESSOR_H