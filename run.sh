#!/bin/bash
set -e
# Build the image for the web server
docker build -t cutie .

# Build the image for the example app
docker build -t kitty_chat example_app/

clear
# Run the example app in a container
docker run --hostname cutecontainer -p 8888:80 kitty_chat
