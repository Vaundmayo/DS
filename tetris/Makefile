# Makefile for cross-platform Tetris

# 기본 컴파일러 (윈도우에서는 MinGW 기준)
CC = gcc

# 컴파일 대상 소스
SRC = tetris.c

# 최종 실행 파일 이름 (OS에 따라 .exe 추가)
ifeq ($(OS),Windows_NT)
    EXE = tetris.exe
    CFLAGS = -Wall -Wextra -std=c99
else
    EXE = tetris
    CFLAGS = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE
endif

# 빌드 대상
all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# 실행
run: $(EXE)
	./$(EXE)

# 삭제
clean:
ifeq ($(OS),Windows_NT)
	del /F tetris.exe 2>NUL
else
	rm -f tetris tetris.exe
endif

# 디버그 빌드
debug: CFLAGS += -g -DDEBUG
debug: clean $(EXE)

# 최적화 빌드
release: CFLAGS += -O2
release: clean $(EXE)

# 도움말
help:
	@echo "사용 가능한 명령어:"
	@echo "  make        - 프로그램 빌드"
	@echo "  make run    - 프로그램 실행"
	@echo "  make clean  - 빌드 파일 삭제"
	@echo "  make debug  - 디버그 모드로 빌드"
	@echo "  make release - 최적화 빌드"
	@echo "  make help   - 이 도움말 표시"

.PHONY: all run clean debug release help