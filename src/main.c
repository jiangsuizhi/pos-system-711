#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 商品结构体：基础商品信息
typedef struct {
    char name[20];
    char code[10];
    double price;
} Product;

// 购物车条目：商品+购买数量
typedef struct {
    int product_idx;
    int quantity;
} CartItem;

// 商品库
Product products[] = {
    {"Cola", "001", 3.50},
    {"Lollipop", "002", 0.50},
    {"Noodles", "003", 6.00}
};
const int product_cnt = sizeof(products) / sizeof(Product);

// 购物车，最多放20种商品
#define MAX_CART 20
CartItem cart[MAX_CART];
int cart_size = 0;

// 根据条码查找商品，返回下标，找不到返回-1
int find_product(const char *code)
{
    for (int i = 0; i < product_cnt; i++) {
        if (strcmp(products[i].code, code) == 0) {
            return i;
        }
    }
    return -1;
}

// 在购物车中查找商品，返回cart下标，找不到返回-1
int find_in_cart(int prod_idx) {
    for (int i = 0; i < cart_size; i++) {
        if (cart[i].product_idx == prod_idx) {
            return i;
        }
    }
    return -1;
}

// 增减购物车商品
void cart_modify(int prod_idx, int delta)
{
    int cart_pos = find_in_cart(prod_idx);
    if (cart_pos >= 0) {
        cart[cart_pos].quantity += delta;
        if (cart[cart_pos].quantity <= 0) {
            // 数量<=0，直接从购物车移除
            for (int i = cart_pos; i < cart_size - 1; i++) {
                cart[i] = cart[i+1];
            }
            cart_size--;
        }
    } else {
        // 购物车没有这个商品，只有delta>0才新增
        if (delta > 0 && cart_size < MAX_CART) {
            cart[cart_size].product_idx = prod_idx;
            cart[cart_size].quantity = delta;
            cart_size++;
        }
    }
}

// print / checkout 打印小票
void print_receipt(void)
 {
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

// prices 打印全部商品清单
void print_all_goods(void) 
{
    printf("Item      No. Pri.\n");
    printf("----------------\n");
    for (int i = 0; i < product_cnt; i++) {
        printf("%-9s %s %.2f\n", products[i].name, products[i].code, products[i].price);
    }
}

// 处理单个token，例如 "001" 或者 "-001"
void handle_token(char *token) 
{
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
    // 打印本条更新后的该行商品
    int c_pos = find_in_cart(prod_idx);
    if(c_pos >= 0){
        Product *p = &products[prod_idx];
        double amt = p->price * cart[c_pos].quantity;
        printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[c_pos].quantity, amt);
    }
}

int main(void)
{
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
        if (strcmp(input_buf, "checkout") == 0) {
            printf("Receipt\n");
            printf("Item       Pri. Qty Amount\n");
            printf("--------------------------\n");
            print_receipt();
            cart_size = 0; //结账清空购物车
            continue;
        }

        // 分割一行内多个条码指令
        char *token = strtok(input_buf, " ");
        while (token != NULL) {
            handle_token(token);
            token = strtok(NULL, " ");
        }
    }
    return 0;
}
