#include "main/main.h"

int main() {
    namespace fs = std::filesystem;

    fs::path main_path = fs::current_path().parent_path();

    string inputPath = main_path.string() + COURSEDBINPUT;
    string modifiedOutputPath = main_path.string() + UOTPUTPATH;
    string userInput = main_path.string() + USERINPUT;

    main_app(inputPath, modifiedOutputPath, userInput);

    return 1;
}