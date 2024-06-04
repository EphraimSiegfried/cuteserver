# cuteserver

A simple and cute web server written in C

## Try out the web server

Start cuteserver to serve the example application located in this repository.

```bash
# First create the image for the web server.
# Make sure you are in the project root directory
docker build -t cutie .
# Then create the image for the example app
docker build -t kitty_chat example_app/
# Run the example app in a container
docker run -p 8888:80 kitty_chat
```

Or just run

```bash
./run.sh
```
