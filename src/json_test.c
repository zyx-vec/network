#include <stdio.h>
#include <string.h>
#include "json.h"


int main() {
    char* json = "[1, {\"hello\": \"world\"}]";

    json_value value;
    json_init(&value);
    parse(&value, json);

    printf("%lf\n", json_get_number(json_get_array_element(&value, 0)));
    printf("%s: %s\n", json_get_object_key(json_get_array_element(&value, 1), 0),\
            json_get_string(json_get_object_value(json_get_array_element(&value, 1), 0)));

    json_free(&value);

    return 0;
}

