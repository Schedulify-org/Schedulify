#include "main/main.h"

int main() {
    string inputPath = "../data/validDB.txt";
    string modifiedOutputPath = "../data/V1.schedOutput.txt";
    string userInput = "../data/userInput.txt";

    main_app(inputPath, modifiedOutputPath, userInput);

    return 1;
}