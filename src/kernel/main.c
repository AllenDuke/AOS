

int display_position = (80 * 6 + 0) * 2;

void low_print(char *str);

void aos_main(void) {
    low_print("#{aos_main}-->Hello OS!!!\n");
    while (1) {}
}

