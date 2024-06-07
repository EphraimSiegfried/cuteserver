#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DATA_FILE "data.json"
#define BUFFER_SIZE 8000
#define SEPARATOR "*"

void handle_get();
void handle_post();
int file_exists(const char *filename);
void read_data_file(int start_line);
void write_data_file(struct json_object *json_obj);

int main() {
    char *request_method = getenv("REQUEST_METHOD");

    printf("Content-Type: application/json\n\n");

    if (request_method == NULL) {
        printf("{\"error\": \"Server Error\", \"message\": \"No request method "
               "provided\"}");
        return 1;
    }

    if (strcmp(request_method, "GET") == 0) {
        handle_get();
    } else if (strcmp(request_method, "POST") == 0) {
        handle_post();
    } else {
        printf("{\"error\": \"Bad Request\", \"message\": \"Unsupported request "
               "method\"}");
        return 1;
    }

    return 0;
}

void handle_get() {
    if (!file_exists(DATA_FILE)) {
        printf("[]");
        return;
    }

    char *query = getenv("QUERY_STRING");
    char *parameter = strtok(query, "=");
    char *value = strtok(NULL, "=");

    // TODO: check if valid query

    read_data_file(atoi(value));
}

void handle_post() {
    char buffer[BUFFER_SIZE];
    struct json_object *json_obj = NULL;
    struct json_object *new_entry = NULL;

    fgets(buffer, BUFFER_SIZE, stdin);

    new_entry = json_tokener_parse(buffer);
    write_data_file(new_entry);
    json_object_put(new_entry);
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void strip_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}
void read_data_file(int start_line) {
    FILE *file = fopen(DATA_FILE, "r");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    printf("[");
    int lines_read = 0;
    int start = 1;
    while ((linelen = getline(&line, &linecap, file)) != -1) {
        lines_read++;
        if (start_line > lines_read) continue;
        // https://stackoverflow.com/questions/201782/can-you-use-a-trailing-comma-in-a-json-object
        if (!start) printf(",");
        start = 0;

        strip_newline(line);
        char *name = strtok(line, SEPARATOR);
        char *message = strtok(NULL, SEPARATOR);
        printf("{\"name\":\"%s\",\"message\":\"%s\"}", name, message);
    }
    printf("]");
    fclose(file);
}

void write_data_file(struct json_object *new_entry) {
    struct json_object *name = NULL;
    struct json_object *message = NULL;
    if (new_entry == NULL ||
        !json_object_object_get_ex(new_entry, "name", &name) ||
        !json_object_object_get_ex(new_entry, "message", &message)) {
        printf("{\"error\": \"Bad Request\", \"message\": \"Please provide both "
               "name and message\"}");
        return;
    }
    const char *str_name = json_object_get_string(name);
    const char *str_message = json_object_get_string(message);

    FILE *file = fopen(DATA_FILE, "a");
    fprintf(file, "%s%s%s\n", str_name, SEPARATOR, str_message);
    fclose(file);
    printf("%s", json_object_to_json_string(new_entry));
}
