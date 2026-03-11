#include "api/c.h"

#include <stdio.h>

int main(void) {
    SF_Engine* engine = sf_create_engine();
    if (engine == NULL) {
        printf("Error: could not create engine\n");
        return 1;
    }

    const char* life_node_name = "life";
    sf_create_value_node(engine, life_node_name, 105.0);

    double node_value = 0.0;
    SF_ErrorCode error = sf_get_node_value(engine, life_node_name, &node_value);
    if (error != SF_OK) {
        printf("Error: %s\n", sf_last_error());
    } else {
        printf("Val: %.0f\n", node_value);
    }

    sf_create_value_node(engine, life_node_name, 100.0);
    printf("Error: %s\n", sf_last_error());

    sf_destroy_engine(engine);
    return 0;
}
