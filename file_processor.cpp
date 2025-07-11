#include <cstddef>
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "file_info.h"
#include "config.h"
#include "search_utils.h"
#include "file_processor.h"

AdaptiveFileProcessor::AdaptiveFileProcessor() : 
    total_lines(0), 
    matched_lines(0), 
    total_files_processed(0) 
{}

bool AdaptiveFileProcessor::load_patterns(const std::string& rules_file) {
    // Check if rules file exists
    if (!std::filesystem::exists(rules_file)) {
        std::cerr << "Rules file '" << rules_file << "' not found. Creating default file...\n";
        create_default_rules_file(rules_file);
        std::cerr << "Created '" << rules_file << "' with placeholder rule.\n";
        std::cerr << "Change the rules to what you want, then run the program again.\n";
        return false;
    }
    
    std::ifstream file(rules_file);
    if (!file) {
        std::cerr << "Error: Cannot open rules file '" << rules_file << "'\n";
        return false;
    }
    
    searcher.clear_patterns();
    std::string line;
    int rule_count = 0;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        searcher.add_pattern(line);
        rule_count++;
    }
    
    if (rule_count == 0) {
        std::cerr << "No valid rules found in '" << rules_file << "'.\n";
        std::cerr << "Please add search patterns (one per line) and run the program again.\n";
        return false;
    }
    
    std::cerr << "Loaded " << rule_count << " patterns from " << rules_file << "\n";
    return true;
}

void AdaptiveFileProcessor::process_directory(const std::string& directory) {
    auto start_time = std::chrono::steady_clock::now();
    
    // Open single output file
    std::ofstream result_file(std::string(NAME) + ".result", std::ios::binary);
    if (!result_file) {
        std::cerr << "Error: Cannot create " << NAME << ".result\n";
        return;
    }
    
    // Find all .txt files
    std::vector<std::filesystem::path> txt_files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && 
                entry.path().extension() == ".txt") {
                txt_files.push_back(entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error reading current directory: " << e.what() << "\n";
        return;
    }
    
    if (txt_files.empty()) {
        std::cerr << "No .txt files found in current directory\n";
        return;
    }
    
    std::cerr << "Found " << txt_files.size() << " .txt files to process\n";
    std::cerr << "Processing with " << searcher.pattern_count() << " patterns\n\n";
    
    // Process each file directly to result file
    size_t total_size = 0;
    for (const auto& file_path : txt_files) {
        std::string input_file = file_path.string();
        
        // Get file size before processing
        std::error_code ec;
        auto file_size = std::filesystem::file_size(file_path, ec);
        std::string size_str = ec ? "unknown size" : formatNumber(file_size);
        std::cerr << "Processing: " << file_path.filename().string() << " (" << size_str << ") ... ";
        
        size_t file_matched_lines = process_file_to_stream(input_file, result_file);
        total_files_processed++;
        total_size += file_size;
        
        if (file_matched_lines > 0) {
            std::cerr << "✓ " << formatNumber(file_matched_lines) << " matches\n";
        } else {
            std::cerr << "✗ no matches\n";
        }
    }
    
    result_file.close();
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    print_batch_stats(duration, total_size);
}

size_t AdaptiveFileProcessor::process_file_to_stream(const std::string& input_filename, std::ostream& output) {
    size_t prev_matched = matched_lines;
    
    try {
        FileInfo file_info = analyze_input(input_filename);
        ProcessingMethod method = select_method(file_info);
        
        std::ifstream input(input_filename, std::ios::binary);
        if (!input) {
            std::cerr << "Error: Cannot open " << input_filename << "\n";
            return 0;
        }
        
        // Process using selected method
        switch (method) {
            case ProcessingMethod::SMALL_FILE_OPTIMIZED:
                process_small_file(input, output);
                break;
            case ProcessingMethod::LARGE_FILE_OPTIMIZED:
                process_large_file(input, output);
                break;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing " << input_filename << ": " << e.what() << "\n";
    }
    
    return matched_lines - prev_matched;
}

void AdaptiveFileProcessor::print_batch_stats(const std::chrono::milliseconds& duration, size_t total_size) const {
    std::cerr << "\n=== BATCH PROCESSING COMPLETE ===\n";
    std::cerr << "Files processed: " << formatNumber(total_files_processed) << '\n';
    std::cerr << "Total size: " << formatNumber(total_size) << '\n';
    std::cerr << "Total lines scanned: " << formatNumber(total_lines) << '\n';
    std::cerr << "Total matched lines: " << formatNumber(matched_lines) << '\n';
    std::cerr << "Processing time: " << duration.count() << " ms\n";
    
    if (duration.count() > 0) {
        std::cerr << "Lines per second: " 
                 << std::fixed << std::setprecision(0)
                 << formatNumber((total_lines * 1000.0) / duration.count()) << '\n';
    }
}

FileInfo AdaptiveFileProcessor::analyze_input(const std::string& filename) {
    struct stat file_stat;
    if (stat(filename.c_str(), &file_stat) != 0) {
        throw std::runtime_error("Cannot stat input file: " + filename);
    }
    
    return FileInfo(file_stat.st_size, S_ISREG(file_stat.st_mode));
}

ProcessingMethod AdaptiveFileProcessor::select_method(const FileInfo& info) {
    if (info.size < SMALL_FILE_THRESHOLD) {
        return ProcessingMethod::SMALL_FILE_OPTIMIZED;
    } else {
        return ProcessingMethod::LARGE_FILE_OPTIMIZED;
    }
}

void AdaptiveFileProcessor::process_small_file(std::istream& input, std::ostream& output) {
    // Optimized for small files - use getline with string operations
    std::string line;
    line.reserve(1024);
    
    // Use larger buffer for better performance
    std::string output_buffer;
    output_buffer.reserve(256 * 1024);  // 256KB
    constexpr size_t FLUSH_THRESHOLD = 128 * 1024;
    
    while (std::getline(input, line)) {
        total_lines++;
        
        if (searcher.search_any(line)) {
            matched_lines++;
            output_buffer += line;
            output_buffer += '\n';
            
            if (output_buffer.size() >= FLUSH_THRESHOLD) {
                output << output_buffer;
                output_buffer.clear();
            }
        }
    }
    
    if (!output_buffer.empty()) {
        output << output_buffer;
    }
}

void AdaptiveFileProcessor::process_large_file(std::istream& input, std::ostream& output) {
    // Optimized approach for medium files
    constexpr size_t BUFFER_SIZE = 512 * 1024;  // 512KB read buffer
    std::vector<char> read_buffer(BUFFER_SIZE);
    input.rdbuf()->pubsetbuf(read_buffer.data(), BUFFER_SIZE);
    
    std::string line;
    line.reserve(8192);  // Larger line buffer
    
    std::string output_buffer;
    output_buffer.reserve(512 * 1024);  // 512KB output buffer
    constexpr size_t FLUSH_THRESHOLD = 256 * 1024;
    
    while (std::getline(input, line)) {
        total_lines++;
        
        if (searcher.search_any(line)) {
            matched_lines++;
            output_buffer += line;
            output_buffer += '\n';
            
            if (output_buffer.size() >= FLUSH_THRESHOLD) {
                output << output_buffer;
                output_buffer.clear();
            }
        }
    }
    
    if (!output_buffer.empty()) {
        output << output_buffer;
    }
}

void AdaptiveFileProcessor::create_default_rules_file(const std::string& rules_file) {
    std::ofstream file(rules_file);
    if (!file) {
        throw std::runtime_error("Cannot create rules file: " + rules_file);
    }
    
    file << "# " << NAME << " Rules File\n";
    file << "# Add one search pattern per line\n";
    file << "# Lines starting with # are comments\n";
    file << "\n";
    file << "steampowered\n";
    
    file.close();
}

std::string AdaptiveFileProcessor::formatNumber(double number) const {
    std::ostringstream oss;
    
    if (number >= 1000000) {
        oss << std::fixed << std::setprecision(1) << (number / 1000000.0) << 'M';
    }
    else if (number >= 1000) {
        oss << std::fixed << std::setprecision(1) << (number / 1000.0) << 'K';
    }
    else {
        oss << std::to_string(static_cast<int>(number));
    }
    
    // Remove trailing .0 if present
    std::string result = oss.str();
    size_t dot_pos = result.find(".0");
    if (dot_pos != std::string::npos && dot_pos == result.length() - 2) {
        result.erase(dot_pos, 2);
    }
    
    return result;
}