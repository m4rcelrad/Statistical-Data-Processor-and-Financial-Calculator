#include <stdio.h>
#include <stdlib.h>
#include "dataframe.h"
#include "csv_reader.h"

// #include "loan_simulation.h"
// #include "report.h"

int main(void) {

    printf("Data Manager Module\n");

    const char *target_file = "report.csv";
    printf("Attempting to load: %s\n", target_file);

    DataFrame *df = NULL;
    DataframeErrorCode status = read_csv(target_file, true, ";", &df);

    if (status == DATAFRAME_SUCCESS) {
        printf("SUCCESS: File loaded perfectly.\n");

        print_head_dataframe(df, 10);

        free_dataframe(df);
    } else {
        printf("ERROR: Could not load file. Code: %d\n", status);
    }

    return 0;
}