# 編譯器和標誌
CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

# 輸出檔案名稱
TARGET = main

# 源碼檔案
SRCS = main.c

# 編譯規則
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS) $(LDFLAGS)

# 清理編譯文件
clean:
	rm -f $(TARGET)
