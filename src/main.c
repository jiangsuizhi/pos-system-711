#include <stdio.h>
#include <string.h>

// 定义结构体
typedef struct {
    char name[10];   // 商品名称
    char code[10];   // 条码
    double price;    // 价格
} Product;

int main() {
    // 初始化商品数组
    Product goods[] = {
        {"Cola",    "001", 3.50},
        {"Lollipop","002", 0.50},
        {"Noodles", "003", 6.00}
    };
    int goods_count=sizeof(goods) / sizeof(goods[0]);  // 商品数量
     
    char input[100]="/0";  // 存放用户输入的整行字符串

    printf("===== 711possystem =====\n");
    while (1) {
        printf("#");
        // 读取输入
        fgets(input, sizeof(input), stdin);
        // 去掉fgets读到的换行符 \n
        input[strcspn(input, "\n")] = '\0';

        // 是否退出
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        }

        // 所有商品
        if (strcmp(input, "prices") == 0) {
            printf("Item      No. Pri.\n");
            printf("-------------------\n");
            for (int i = 0; i < goods_count; i++) {
                printf("%-10s %s %.2f\n", goods[i].name, goods[i].code, goods[i].price);
            }
            continue;
        }

        // 分割
        char *token = strtok(input, " ");
        while (token != NULL) {
            int found = 0;
            for (int i = 0; i < goods_count; i++) {
                if (strcmp(token, goods[i].code) == 0) {
                    printf("%s\t%.2f\n", goods[i].name, goods[i].price);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("ERROR: code not found\n");
            }
            token = strtok(NULL, " ");
        }
    }
    printf("程序退出\n");
    return 0;
}


