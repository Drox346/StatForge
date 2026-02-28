#include "api/c.h"

#include <stdio.h>

int main(void) {
    SF_Engine* engine = sf_create_engine();
    if (engine == NULL) {
        printf("Error: could not create engine\n");
        return 1;
    }

    const char* life_cell_name = "life";
    sf_create_value_cell(engine, life_cell_name, 105.0);

    double cell_value = 0.0;
    SF_ErrorCode error = sf_get_cell_value(engine, life_cell_name, &cell_value);
    if (error != SF_OK) {
        printf("Error: %s\n", sf_last_error());
    } else {
        printf("Val: %.0f\n", cell_value);
    }

    sf_create_value_cell(engine, life_cell_name, 100.0);
    printf("Error: %s\n", sf_last_error());

    sf_destroy_engine(engine);
    return 0;
}
