#include <stdio.h>
#include <stdlib.h>
#include "dataframe.h"
#include "csv_reader.h"

int main(void) {
    const char *filepath = "market_data_test.csv";

    FILE *file = fopen(filepath, "w");
    if (file) {
        fprintf(file, "Date,Ticker,ClosePrice,Volume\n");
        fprintf(file, "2023-10-01,MSFT,315.75,210000\n");
        fprintf(file, "2023-10-02,MSFT,317.20,250000\n");
        fprintf(file, "2023-10-03,MSFT,312.90,300000\n");
        fprintf(file, "2023-10-04,MSFT,318.40,190000\n");
        fprintf(file, "2023-10-05,MSFT,,185000\n");
        fclose(file);
        printf("Test file created: %s\n", filepath);
    } else {
        printf("Test file creation failed.\n");
        return 1;
    }

    DataFrame *df = NULL;
    const int status = read_csv(filepath, true, ",", &df);

    if (status != DATAFRAME_SUCCESS) {
        printf("CSV reading error. Error code: %d\n", status);
        return 1;
    }

    print_head_dataframe(df, 10);

    free_dataframe(df);

    return 0;
}