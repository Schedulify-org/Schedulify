#include "main/main.h"

int main() {
    char cwdBuf[MAX_PATH];
    if (!getcwd(cwdBuf, sizeof(cwdBuf))) {
        perror("getcwd failed");
        return 1;
    }

    std::string cwd(cwdBuf);
    auto pos = cwd.find_last_of("/\\");
    if (pos != std::string::npos) {
        cwd.erase(pos);
    }

    string inputPath = cwd + COURSEDBINPUT;
    string modifiedOutputPath = cwd + UOTPUTPATH;
    string userInput = cwd + USERINPUT;

    main_app(inputPath, modifiedOutputPath, userInput);

    return 1;
}