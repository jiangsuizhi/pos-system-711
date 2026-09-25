#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// 商品结构体
typedef struct {
    char name[20];
    char code[10];
    double price;
} Product;

// 购物车条目
typedef struct {
    int product_idx;
    int quantity;
} CartItem;

// 单条交易里的商品明细（用于写入销售文件）
typedef struct {
    char item_name[20];
    int qty;
} SaleItem;

// 一笔完整销售订单
typedef struct {
    int day;                // 交易所属日期号
    int serial_no;          // 流水号
    char time_str[20];      // HH:MM:SS
    SaleItem items[20];
    int item_count;
    double total;
} SaleRecord;

// ========== 商品基础数据 ==========
Product products[] = {
    {"Cola", "001", 3.50},
    {"Lollipop", "002", 0.50},
    {"Noodles", "003", 6.00}
};
const int product_cnt = sizeof(products) / sizeof(Product);

// ========== 购物车全局变量 ==========
#define MAX_CART 20
CartItem cart[MAX_CART];
int cart_size = 0;

// ========== 销售记录全局变量 ==========
int today_day = 1;          // 当前是第几天，newday会+1
int current_serial = 0;    // 当日流水号，每checkout+1

// 根据条码查找商品，返回下标，找不到返回-1
int find_product(const char *code) {
    for (int i = 0; i < product_cnt; i++) {
        if (strcmp(products[i].code, code) == 0) {
            return i;
        }
    }
    return -1;
}

// 在购物车中查找商品
int find_in_cart(int prod_idx) {
    for (int i = 0; i < cart_size; i++) {
        if (cart[i].product_idx == prod_idx) {
            return i;
        }
    }
    return -1;
}

// 修改购物车商品数量 delta=+1 / -1
void cart_modify(int prod_idx, int delta) {
    int cart_pos = find_in_cart(prod_idx);
    if (cart_pos >= 0) {
        cart[cart_pos].quantity += delta;
        if (cart[cart_pos].quantity <= 0) {
            // 数量归零，移除购物车
            for (int i = cart_pos; i < cart_size - 1; i++) {
                cart[i] = cart[i+1];
            }
            cart_size--;
        }
    } else {
        if (delta > 0 && cart_size < MAX_CART) {
            cart[cart_size].product_idx = prod_idx;
            cart[cart_size].quantity = delta;
            cart_size++;
        }
    }
}

// 打印当前购物小票（print命令）
void print_receipt(void) {
    double total = 0.0;
    for (int i = 0; i < cart_size; i++) {
        Product *p = &products[cart[i].product_idx];
        double amount = p->price * cart[i].quantity;
        printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[i].quantity, amount);
        total += amount;
    }
    printf("------------------------\n");
    printf("Total                  =%.2f\n", total);
}

// prices：打印全部商品清单
void print_all_goods(void) {
    printf("Item      No. Pri.\n");
    printf("----------------\n");
    for (int i = 0; i < product_cnt; i++) {
        printf("%-9s %s %.2f\n", products[i].name, products[i].code, products[i].price);
    }
}

// 处理单个token: "001" "-001"
void handle_token(char *token) {
    int delta = 1;
    char code[10];
    if (token[0] == '-') {
        delta = -1;
        strcpy(code, token + 1);
    } else {
        strcpy(code, token);
    }
    int prod_idx = find_product(code);
    if (prod_idx < 0) {
        printf("ERROR: code not found\n");
        return;
    }
    cart_modify(prod_idx, delta);
    int c_pos = find_in_cart(prod_idx);
    if(c_pos >= 0){
        Product *p = &products[prod_idx];
        double amt = p->price * cart[c_pos].quantity;
        printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[c_pos].quantity, amt);
    }
}

// 将本次结账订单追加写入 sales.csv
void save_sale_to_file(int day, int serial, const char *timestr, double total) {
    FILE *fp = fopen("sales.csv", "a");
    if (!fp) {
        printf("ERROR: cannot open sales.csv\n");
        return;
    }
    // 一行格式：day,serial,time,total,item1:qty;item2:qty
    fprintf(fp, "%d,%d,%s,%.2f,", day, serial, timestr, total);
    for(int i=0; i<cart_size; i++){
        Product *p = &products[cart[i].product_idx];
        fprintf(fp, "%s:%d;", p->name, cart[i].quantity);
    }
    fprintf(fp,"\n");
    fclose(fp);
}

// sales 命令：读取csv，打印指定day的销售记录
void show_sales(int target_day) {
    FILE *fp = fopen("sales.csv", "r");
    if(!fp){
        printf("Date: %d\n", target_day);
        printf("No sales records.\nDaily: 0.00\n");
        return;
    }
    char buf[512];
    double daily_total = 0.0;
    int has_record = 0;

    printf("Date: %d\n", target_day);
    printf("No.     Time        Items                 Amount\n");
    printf("------------------------------------------------\n");

    while(fgets(buf,512,fp)){
        int day, serial;
        char time_str[20];
        double total;
        char items_buf[256];
        // csv解析 day,serial,time,total,items
        sscanf(buf, "%d,%d,%[^,],%lf,%s", &day, &serial, time_str, &total, items_buf);
        if(day == target_day){
            has_record = 1;
            daily_total += total;
            // 打印流水号、时间
            printf("%d    %s    ", serial, time_str);
            // 分割商品  Cola:1;Lollipop:1;
            char *item_token = strtok(items_buf, ";");
            int first_item = 1;
            while(item_token != NULL && strlen(item_token)>0){
                if(!first_item) printf("                ");
                printf("%s\n", item_token);
                first_item = 0;
                item_token = strtok(NULL, ";");
            }
            printf("                                     %.2f\n", total);
        }
    }
    fclose(fp);
    if(!has_record){
        printf("No records\n");
    }
    printf("Daily: %.2f\n", daily_total);
}

int main(void) {
    char input_buf[256];
    while (1) {
        printf("> ");
        fgets(input_buf, sizeof(input_buf), stdin);
        input_buf[strcspn(input_buf, "\n")] = '\0';

        if (strcmp(input_buf, "exit") == 0 || strcmp(input_buf, "quit") == 0) {
            break;
        }
        if (strcmp(input_buf, "prices") == 0) {
            print_all_goods();
            continue;
        }
        if (strcmp(input_buf, "drop") == 0) {
            cart_size = 0;
            continue;
        }
        if (strcmp(input_buf, "print") == 0) {
            print_receipt();
            continue;
        }
        if (strcmp(input_buf, "newday") == 0) {
            today_day += 1;
            current_serial = 0;
            cart_size = 0;
            printf("New day started. Today's sales records cleared.\n");
            continue;
        }
        // ===== sales [day] 命令 =====
        char cmd[32];
        int arg_day;
        if(sscanf(input_buf, "%s %d", cmd, &arg_day) == 2){
            if(strcmp(cmd,"sales") == 0){
                show_sales(arg_day);
                continue;
            }
        }
        if(strcmp(input_buf, "sales") == 0){
            show_sales(today_day);
            continue;
        }

        // ===== checkout 结账 =====
        if(strcmp(input_buf, "checkout") == 0){
            printf("Receipt\n");
            printf("Item       Pri. Qty Amount\n");
            printf("--------------------------\n");
            double total = 0.0;
            for (int i = 0; i < cart_size; i++) {
                Product *p = &products[cart[i].product_idx];
                double amount = p->price * cart[i].quantity;
                printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[i].quantity, amount);
                total += amount;
            }
            printf("------------------------\n");
            printf("Total                  =%.2f\n", total);

            // 获取当前系统时间
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char timestr[20];
            sprintf(timestr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
            current_serial++;
            save_sale_to_file(today_day, current_serial, timestr, total);
            cart_size = 0; // 清空购物车
            continue;
        }

        // 解析条码指令 001 -001
        char *token = strtok(input_buf, " ");
        while (token != NULL) {
            handle_token(token);
            token = strtok(NULL, " ");
        }
    }
    return 0;
}
