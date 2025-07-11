
#ifndef FILEINFO_H
#define FILEINFO_H
#include <cstddef>
struct FileInfo {
    size_t size;
    bool is_regular_file;
    
    FileInfo(size_t s, bool regular) : size(s), is_regular_file(regular) {}
};

enum class ProcessingMethod {
    SMALL_FILE_OPTIMIZED,    // < 50MB: getline() with string operations
    LARGE_FILE_OPTIMIZED     // > 500MB: character-by-character with fixed buffers
};
#endif