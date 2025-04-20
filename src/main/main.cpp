#include "main/main.h"

int main() {
    string inputPath = COURSEDBINPUT;
    string modifiedOutputPath = OUTPUTPATH;
    string courseOutput = OUTPUTCOURSEPATH;
    string userInput = USERINPUT;

    main_app(inputPath, modifiedOutputPath, courseOutput, userInput);

    return 1;
}