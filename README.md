# cuteserver

A simple and cute web server written in C

## Try out the web server

**Requirements**: Docker

Start cuteserver to serve the example application located in this repository.

1. First start the docker daemon. On most Linux systems this can be done with ` sudo systemctl start docker`. On MacOS start the docker desktop app.

2. Run the following commands

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
bash run.sh
```

3. Open your preferred browser and visit **localhost:8888**

## Manual: Host your own web application

### Setting up with docker

**Requirements**: Docker

For web applications that use static files, you can use this docker-compose file.
```yaml
services:
  cutie:
    build: https://github.com/EphraimSiegfried/cuteserver.git
    ports:
      - 8888:80
    volumes:
      - /path/to/dir/with/static/files/:/content/
      - /path/to/config.toml:/config.toml
```

Now set your preferred configuration in the configuration file for the web server. 

```TOML
[server]
    address = "0.0.0.0" 
    port = 80 
    workers = 4
    log_file = "./logs"

[resource.cute]
    domain = "your-domain.com"
    root = "/content"
    cgi_bin_dir = "/webs/content/cgi-bin"
    [resource.cute.remaps]
        "/api/messages" = "/cgi-bin/messages.cgi"
        "/verycool" = "/cgi-bin/cool.cgi"
```


For applications that need CGI scripts that rely on external libraries please refer to the example configuration in the example_app folder. Note that CGI scripts need the .cgi file extension.

### Building Cuteserver from scratch

**Requirements**: cmake, make, git, gcc

Run the following commands

```
cmake .
make
```

Now it should be possible to run `cuteserver` in the command 

Note: The default path for the configuration file is "./config.toml", so when you build from scratch, it uses the config.toml file at root level. You can provide another configuration path using the -p argument. You might need to adjust the paths in your configuration file and compile the executables (cgi-scripts) before running. 
