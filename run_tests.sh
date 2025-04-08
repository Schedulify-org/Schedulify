#!/bin/bash

# Build the Docker image
docker build -f Dockerfile_tests -t schedulify-test .

# Run the built Docker container
docker run -it --rm -v "$(pwd):/usr/src/app" -w /usr/src/app/tests/build schedulify-test