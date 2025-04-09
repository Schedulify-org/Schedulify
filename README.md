# 📅 Schedulify
Schedulify is a containerized tool for generating valid course schedules based on user-selected course IDs and timing constraints.

### Prerequisites
- Docker installed → [Get Docker](https://www.docker.com/get-started)
- WSL(for Windows) or any terminal (for macOS/Linux)

 ### Run the project container:

1. Clone the repo from github 
  
   ``` git clone https://github.com/Schedulify-org/Schedulify.git  ```

2. Navigate to project's path in the terminal
 
   ``` cd Schedulify  ```

3. Grant execute permission by running (for Mac/Linux only):
  
     ``` sudo chmod +x run_app.sh  ```

4. Run the app container:  🐳 Make Sure Docker Daemon Is Active !

     ./run_app.sh 

          
 ### Run tests:
1. Grant execute permission by running (Mac/Linux only):
 
      ``` sudo chmod +x run_tests.sh  ```
  
2. Run the test container 🐳 Make Sure Docker Daemon Is Active !

      ./run_tests.sh

## Output

- Results will be saved to a file named:  
  #### data/V1.schedOutput.txt #
- This file will be created in the /data directory of the project after running the app.
- Test results are shown directly in the terminal during execution.
****
