//
// Created by 杜科 on 2020/10/8.
//


void low_print(char *str);

void aos_main(void) {
    low_print("aos_main\n");
    low_print("Hello World!\n");
    while (1) {}
}
