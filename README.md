# Schedulify
 Schedulify is a containerized tool for generating legal course schedules based on a user's selected course IDs and timming constraints.
### Prerequisites
- Docker installed → [Get Docker](https://www.docker.com/get-started)
- Git Bash (for Windows) or any terminal (for macOS/Linux)

 ### Run the project container:
1. Clone the repo from github 
  
   ``` git clone https://github.com/Schedulify-org/Schedulify.git  ```

2. Navigate to project's path in the terminal
 
   ``` cd Schedulify  ```

3. Grant execute permission by running (for Mac/Linux only):
  
     ``` sudo chmod +x run_app.sh  ```

4. Run the app: 
### 🐳 Make Sure Docker Daemon Is Active
     ./run_app.sh 
- 🪟 Windows users, No need to set permissions. Use Git Bash and run:
``` bash run_app.sh ```

          
 ### Run tests:
1. Grant execute permission by running (Mac/Linux only):
 
      ``` sudo chmod +x run_tests.sh  ```
  
2. Run the test container  
### 🐳 Make Sure Docker Daemon Is Active
      ./run_tests.sh

- 🪟 Windows: ``` bash run_tests.sh  ```
