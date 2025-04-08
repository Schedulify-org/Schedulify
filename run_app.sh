#!/bin/bash

# Build the Docker image
docker build -f Dockerfile_main -t schedulify-main .

# Run the built Docker container
docker run -it --rm -v "$(pwd):/usr/src/app" -w /usr/src/app/build schedulify-main
