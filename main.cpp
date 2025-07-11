#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include "file_processor.h"



int main() {
    try {
        AdaptiveFileProcessor processor;
        
        // Load patterns from rules file
        if (!processor.load_patterns()) {
            std::cout<<"Press any key to exit...";
            getchar();
            return 1; // Exit if no valid patterns loaded
        }
        
        // Process all .txt files in current directory
        processor.process_directory();
        std::cout<<"Press any key to exit...";
        getchar();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}
