#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROD 30
#define MAX_CART 20
#define MAX_NAME_LEN 20
#define MAX_CODE_LEN 10

// 商品结构体
typedef struct {
    char name[MAX_NAME_LEN];
    char code[MAX_CODE_LEN];
    double price;
} Product;

// 购物车条目
typedef struct {
    int product_idx;
    int quantity;
} CartItem;

// 销售订单明细
typedef struct {
    char item_name[MAX_NAME_LEN];
    int qty;
} SaleItem;

// 一笔销售记录
typedef struct {
    int day;
    int serial_no;
    char time_str[20];
    SaleItem items[20];
    int item_count;
    double total;
} SaleRecord;

// ========== 全局变量 ==========
Product products[MAX_PROD];
int product_cnt = 0;

CartItem cart[MAX_CART];
int cart_size = 0;

int today_day = 1;
int current_serial = 0;

int is_admin_mode = 0;
char admin_pwd[20] = "admin123"; //初始管理员密码

// 初始化默认商品
void init_default_products()
{
    product_cnt = 0;
    strcpy(products[product_cnt].name, "Cola");
    strcpy(products[product_cnt].code, "001");
    products[product_cnt].price = 3.50;
    product_cnt++;

    strcpy(products[product_cnt].name, "Lollipop");
    strcpy(products[product_cnt].code, "002");
    products[product_cnt].price = 0.50;
    product_cnt++;

    strcpy(products[product_cnt].name, "Noodles");
    strcpy(products[product_cnt].code, "003");
    products[product_cnt].price = 6.00;
    product_cnt++;
}

// 根据条码查找商品下标，找不到返回-1
int find_product(const char *code)
{
    for(int i=0; i<product_cnt; i++){
        if(strcmp(products[i].code, code)==0){
            return i;
        }
    }
    return -1;
}

// 购物车查找商品
int find_in_cart(int prod_idx)
{
    for(int i=0; i<cart_size; i++){
        if(cart[i].product_idx == prod_idx){
            return i;
        }
    }
    return -1;
}

// 修改购物车数量
void cart_modify(int prod_idx, int delta)
{
    int cart_pos = find_in_cart(prod_idx);
    if (cart_pos >= 0)
    {
        cart[cart_pos].quantity += delta;
        if (cart[cart_pos].quantity <= 0)
        {
            for(int i=cart_pos; i<cart_size-1; i++){
                cart[i] = cart[i+1];
            }
            cart_size--;
        }
    }
    else
    {
        if(delta>0 && cart_size < MAX_CART){
            cart[cart_size].product_idx = prod_idx;
            cart[cart_size].quantity = delta;
            cart_size++;
        }
    }
}

// 打印购物小票 print命令
void print_receipt(void)
{
    double total = 0.0;
    for(int i=0; i<cart_size; i++){
        Product *p = &products[cart[i].product_idx];
        double amount = p->price * cart[i].quantity;
        printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[i].quantity, amount);
        total += amount;
    }
    printf("------------------------\n");
    printf("Total                  =%.2f\n", total);
}

// prices：列出全部商品信息
void print_all_goods(void)
{
    printf("Item      No.   Pri.\n");
    printf("--------------------\n");
    for(int i=0; i<product_cnt; i++){
        printf("%-9s %s   %.2f\n", products[i].name, products[i].code, products[i].price);
    }
}

// 处理条码token：001 / -001
void handle_token(char *token)
{
    int delta = 1;
    char code[MAX_CODE_LEN];
    if(token[0] == '-'){
        delta = -1;
        strcpy(code, token+1);
    }else{
        strcpy(code, token);
    }
    int prod_idx = find_product(code);
    if(prod_idx <0){
        printf("ERROR: code not found\n");
        return;
    }
    cart_modify(prod_idx, delta);
    int c_pos = find_in_cart(prod_idx);
    if(c_pos >=0){
        Product *p = &products[prod_idx];
        double amt = p->price * cart[c_pos].quantity;
        printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[c_pos].quantity, amt);
    }
}

// 保存订单到sales.csv
void save_sale_to_file(int day, int serial, const char *timestr, double total)
{
    FILE *fp = fopen("sales.csv", "a");
    if (!fp)
    {
        printf("ERROR: cannot open sales.csv\n");
        return;
    }
    fprintf(fp, "%d,%d,%s,%.2f,", day, serial, timestr, total);
    for(int i=0; i<cart_size; i++){
        Product *p = &products[cart[i].product_idx];
        fprintf(fp, "%s:%d;", p->name, cart[i].quantity);
    }
    fprintf(fp,"\n");
    fclose(fp);
}

// sales命令：读取并展示指定日期销售记录
void show_sales(int target_day)
{
    FILE *fp = fopen("sales.csv", "r");
    if(!fp){
        printf("Date: %d\nNo sales records.\nDaily: 0.00\n", target_day);
        return;
    }
    char buf[512];
    double daily_total =0.0;
    int has_record = 0;
    printf("Date: %d\n", target_day);
    printf("No.     Time        Items                 Amount\n");
    printf("------------------------------------------------\n");
    while(fgets(buf,512,fp)){
        int day, serial;
        char time_str[20];
        double total;
        char items_buf[256];
        sscanf(buf, "%d,%d,%[^,],%lf,%s", &day, &serial, time_str, &total, items_buf);
        if(day == target_day){
            has_record =1;
            daily_total += total;
            printf("%d    %s    ", serial, time_str);
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
    if(!has_record) printf("No records\n");
    printf("Daily: %.2f\n", daily_total);
}

// ===================== 管理员模式命令 =====================
// setprice <code> <newprice>
void cmd_setprice(char *code, double new_price)
{
    int idx = find_product(code);
    if(idx == -1){
        printf("ERROR: Product not found.\n");
        return;
    }
    products[idx].price = new_price;
    printf("Price updated.\n");
}

// itemadd <code> <name> <price>
void cmd_itemadd(char *code, char *name, double price)
{
    if(find_product(code) != -1){
        printf("ERROR: Code already exists.\n");
        return;
    }
    if(product_cnt >= MAX_PROD){
        printf("ERROR: Product list full.\n");
        return;
    }
    strcpy(products[product_cnt].code, code);
    strcpy(products[product_cnt].name, name);
    products[product_cnt].price = price;
    printf("%s(%s) added.\n", products[product_cnt].name, products[product_cnt].code);
    product_cnt++;
}

// itemdel <code>
void cmd_itemdel(char *code)
{
    int idx = find_product(code);
    if(idx == -1){
        printf("ERROR: Product not found.\n");
        return;
    }
    printf("%s(%s) removed.\n", products[idx].name, products[idx].code);
    // 删除，后续商品前移覆盖
    for(int i=idx; i < product_cnt-1; i++){
        products[i] = products[i+1];
    }
    product_cnt--;
}

int main(void)
{
    init_default_products();
    char input_buf[256];
    while (1)
    {
        // 根据模式选择提示符
        if(is_admin_mode){
            printf("admin> ");
        }else{
            printf("> ");
        }
        fgets(input_buf, sizeof(input_buf), stdin);
        input_buf[strcspn(input_buf, "\n")] = '\0';

        // ========= 管理员模式下命令 =========
        if(is_admin_mode)
        {
            char cmd[32], code[10], name[20];
            double price;
            // back退出管理员
            if(strcmp(input_buf, "back") == 0){
                is_admin_mode = 0;
                printf("Bye.\n");
                continue;
            }
            if(strcmp(input_buf, "prices") == 0){
                print_all_goods();
                continue;
            }
            // setprice 条码 价格
            if(sscanf(input_buf, "%s %s %lf", cmd, code, &price) == 3){
                if(strcmp(cmd,"setprice")==0){
                    cmd_setprice(code, price);
                    continue;
                }
            }
            // itemadd 条码 名称 价格
            if(sscanf(input_buf, "%s %s %s %lf", cmd, code, name, &price) ==4){
                if(strcmp(cmd,"itemadd")==0){
                    cmd_itemadd(code,name,price);
                    continue;
                }
            }
            // itemdel 条码
            if(sscanf(input_buf, "%s %s", cmd, code)==2){
                if(strcmp(cmd,"itemdel")==0){
                    cmd_itemdel(code);
                    continue;
                }
            }
            printf("Unknown admin command.\n");
            continue;
        }

        // ========== 普通收银模式 ==========
        if(strcmp(input_buf, "exit") ==0 || strcmp(input_buf, "quit") ==0){
            break;
        }
        if(strcmp(input_buf, "admin") == 0){
            char pwd_in[20];
            printf("Password: ");
            scanf("%s", pwd_in);
            getchar(); //吸收换行
            if(strcmp(pwd_in, admin_pwd)==0){
                is_admin_mode = 1;
                printf("Admin mode.\n");
            }else{
                printf("Wrong password!\n");
            }
            continue;
        }
        if(strcmp(input_buf, "prices") == 0){
            print_all_goods();
            continue;
        }
        if(strcmp(input_buf, "drop") ==0){
            cart_size = 0;
            continue;
        }
        if(strcmp(input_buf, "print") ==0){
            print_receipt();
            continue;
        }
        if(strcmp(input_buf, "newday") ==0){
            today_day +=1;
            current_serial = 0;
            cart_size =0;
            printf("New day started. Today's sales records cleared.\n");
            continue;
        }
        // sales [day]
        char cmd[32];
        int arg_day;
        if(sscanf(input_buf, "%s %d", cmd, &arg_day)==2){            
            if(strcmp(cmd,"sales")==0){
                show_sales(arg_day);
                continue;
            }
        }
        if(strcmp(input_buf, "sales") == 0){
            show_sales(today_day);
            continue;
        }
        // checkout结账
        if(strcmp(input_buf, "checkout") == 0){
            printf("Receipt\n");
            printf("Item       Pri. Qty Amount\n");
            printf("--------------------------\n");
            double total = 0.0;
            for(int i=0; i<cart_size; i++){
                Product *p = &products[cart[i].product_idx];
                double amount = p->price * cart[i].quantity;
                printf("%-10s %.2f x%d =%.2f\n", p->name, p->price, cart[i].quantity, amount);                
                total += amount;                
            }
            printf("------------------------\n");
            printf("Total                  =%.2f\n", total);
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char timestr[20];
            sprintf(timestr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
            current_serial++;
            save_sale_to_file(today_day, current_serial, timestr, total);
            cart_size =0;
            continue;
        }
        // 条码输入
        char *token = strtok(input_buf, " ");
        while(token != NULL){
            handle_token(token);
            token = strtok(NULL, " ");
        }
        
    }
    return 0;
}
