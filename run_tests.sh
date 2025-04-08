#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Convert line endings (do this once manually from Windows editor or with dos2unix/sed)

# Build the Docker image
docker build -f Dockerfile_tests -t schedulify-test .

# Run the container and execute tests inside the build directory
docker run -it --rm -v "$(pwd):/usr/src/app" -w /usr/src/app/tests/build schedulify-test
