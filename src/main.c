#include "dataframe.h"

int main(void) {
    const char *filename = "report.csv";

    DataFrame *df = read_csv(filename);

    if (!df) return 1;

    print_head_dataframe(df, 5);
    free_dataframe(df);
    return 0;
}