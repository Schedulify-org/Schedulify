#include "../include/main.h"
#include "../include/main_app.h"

int main() {
    string inputPath = "src/V1.0CourseDB.txt";
    string outputPath = "src/V1.outputSched.txt";

    main_app(inputPath, outputPath);

    return 1;
}