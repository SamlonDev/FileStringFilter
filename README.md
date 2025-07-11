This program filters files in a directory based on a set of patterns in the FileFilter.rules file.

use build.sh to build the program. Windows users are lost idk.

use ./FileFilter to run the program. It will tell you what to do.

how to compile for windows on linux:
cmake .. -DCMAKE_SYSTEM_NAME=Windows \
         -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
         -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
         -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres

make

