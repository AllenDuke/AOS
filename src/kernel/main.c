

int DISPLAY_POSITION = (80 * 6 + 0) * 2;

void low_print(char *str);

void aos_main(void) {
    low_print("aos_main\n");
    low_print("Hello World!\n");
    while (1) {}
}

