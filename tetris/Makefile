
CC = gcc
SRC = tetris.c

ifeq ($(OS),Windows_NT)
    EXE = tetris.exe
    CFLAGS = -Wall -Wextra -std=c99 -finput-charset=UTF-8 -fexec-charset=UTF-8
else
    EXE = tetris
    CFLAGS = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE
endif

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

run: $(EXE)
	./$(EXE)

clean:
ifeq ($(OS),Windows_NT)
	del /F tetris.exe 2>NUL
else
	rm -f tetris tetris.exe
endif

debug: CFLAGS += -g -DDEBUG
debug: clean $(EXE)

release: CFLAGS += -O2
release: clean $(EXE)

help:
	@echo "사용 가능한 명령어:"
	@echo "  make        - 프로그램 빌드"
	@echo "  make run    - 프로그램 실행"
	@echo "  make clean  - 빌드 파일 삭제"
	@echo "  make debug  - 디버그 모드로 빌드"
	@echo "  make release - 최적화 빌드"
	@echo "  make help   - 이 도움말 표시"

.PHONY: all run clean debug release help