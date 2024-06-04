#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DATA_FILE "data.json"
#define BUFFER_SIZE 1024

void handle_get();
void handle_post();
int file_exists(const char *filename);
void read_data_file(struct json_object **json_obj);
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
  struct json_object *json_obj = NULL;

  if (!file_exists(DATA_FILE)) {
    printf("[]");
    return;
  }

  read_data_file(&json_obj);
  const char *json_str = json_object_to_json_string(json_obj);
  printf("%s", json_str);

  json_object_put(json_obj);
}

void handle_post() {
  char buffer[BUFFER_SIZE];
  struct json_object *json_obj = NULL;
  struct json_object *new_entry = NULL;
  struct json_object *name = NULL;
  struct json_object *message = NULL;

  fgets(buffer, BUFFER_SIZE, stdin);

  new_entry = json_tokener_parse(buffer);

  if (new_entry == NULL ||
      !json_object_object_get_ex(new_entry, "name", &name) ||
      !json_object_object_get_ex(new_entry, "message", &message)) {
    printf("{\"error\": \"Bad Request\", \"message\": \"Please provide both "
           "name and message\"}");
    return;
  }

  if (file_exists(DATA_FILE)) {
    read_data_file(&json_obj);
    json_object_array_add(json_obj, new_entry);
  } else {
    json_obj = json_object_new_array();
    json_object_array_add(json_obj, new_entry);
  }

  write_data_file(json_obj);

  printf("%s", json_object_to_json_string(new_entry));

  json_object_put(json_obj);
}

int file_exists(const char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

void read_data_file(struct json_object **json_obj) {
  FILE *file = fopen(DATA_FILE, "r");
  char buffer[BUFFER_SIZE];

  fread(buffer, sizeof(char), BUFFER_SIZE, file);
  fclose(file);

  *json_obj = json_tokener_parse(buffer);
}

void write_data_file(struct json_object *json_obj) {
  FILE *file = fopen(DATA_FILE, "w");
  const char *json_str = json_object_to_json_string(json_obj);

  fprintf(file, "%s", json_str);
  fclose(file);
}
