# 编译器
CC = gcc
# 编译选项：开启警告 + 头文件路径
CFLAGS = -Wall -I./include
# 链接选项
LDFLAGS = 

# 目标可执行文件
TARGET = bin/demo

# 源文件列表：src下所有.c
SRCS = $(wildcard src/*.c)
# 把 src/*.c 替换成 obj/*.o
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

# 默认目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)	
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 编译.c生成.o文件
obj/%.o: src/%.c	
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译产物
clean:
	rm -rf obj bin

# 伪目标，防止和文件重名
.PHONY: all clean
