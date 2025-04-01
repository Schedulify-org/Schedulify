#include "main/main.h"

int main() {
    string inputPath = "../data/V1.0CourseDB.txt";
    string originOutputPath = "../data/V1.originOutputSched.txt";
    string modifiedOutputPath = "../data/V1.modifiedOutputPath.txt";


    main_app(inputPath, originOutputPath, modifiedOutputPath);

    return 1;
}