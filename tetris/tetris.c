#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

/* OS별 헤더 파일 */
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CURSOR_SHOW() { CONSOLE_CURSOR_INFO ci = {1, TRUE};  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci); }
    #define CURSOR_HIDE() { CONSOLE_CURSOR_INFO ci = {1, FALSE}; SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci); }
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #define CURSOR_SHOW() printf("\033[?25h");
    #define CURSOR_HIDE() printf("\033[?25l");
#endif

/* 타이머 */
#define CCHAR 0
#ifdef CTIME
#undef CTIME
#endif
#define CTIME 1

/* 왼쪽, 오른쪽, 아래, 회전 */
#define LEFT1 106 // 'j'
#define LEFT2 74 //	 'J'
#define RIGHT1 108 // 'l'
#define RIGHT2 76 // 'L'
#define DOWN1 107	// 'k'
#define DOWN2 75	// 'K'
#define ROTATE1 105	// 'i'
#define ROTATE2 73	// 'I'

/* 블록 고정, 블록 삭제, 블록 생성, 게임 종료 */
#define DROP1 97  // 'a'
#define DROP2 65  // 'A'
#define QUIT1 112 // 'p'
#define QUIT2 80  // 'P'

/* 블록 모양 */
#define I_BLOCK 0
#define	T_BLOCK 1
#define S_BLOCK 2
#define Z_BLOCK 3
#define L_BLOCK 4
#define J_BLOCK 5
#define O_BLOCK 6

/* 게임 시작, 게임 종료 */
#define GAME_START 0
#define GAME_END 1

#define Board_Height 19
#define Board_Width 10
#define BoardX 4
#define BoardY 3
int board[Board_Height][Board_Width] = {0}; // 테트리스 판을 2차원 배열로 표현

/* 색상 코드 */
#define RESET_COLOR  "\033[0m"
#define RED_COLOR    "\033[31m"
#define GREEN_COLOR  "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR   "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR   "\033[36m"
#define WHITE_COLOR  "\033[37m"
#define GRAY_COLOR   "\033[90m"

/* Windows ANSI 적용 */
#ifdef _WIN32
void enableWindowsAnsi() {
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hout, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hout, dwMode);
    
    // Windows UTF-8
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
}
#endif


const char* get_Color(int blockNum) {
    switch (blockNum) {
        case I_BLOCK: return CYAN_COLOR;
        case T_BLOCK: return MAGENTA_COLOR;
        case S_BLOCK: return GREEN_COLOR;
        case Z_BLOCK: return RED_COLOR;
        case L_BLOCK: return YELLOW_COLOR;
        case J_BLOCK: return BLUE_COLOR;
        case O_BLOCK: return WHITE_COLOR;
        default: return RESET_COLOR;
    }
}

/*

 * 블록 모양(I, T, S, Z, L, J, O) 
 * 4*4 배열의 2차원 배열
 * 모든 블록의 모양을 표시
 *
 * 블록의 모양을 표시
 * 왼쪽, 오른쪽, 아래, 회전 
 * 4*4 배열의 2차원 배열
 * 모든 블록의 모양을 표시
 *
 * 4*4*4 배열의 3차원 배열
 */

char i_block[4][4][4] = {
    {  // 첫 번째 회전 상태
        {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },
    {  // 두 번째 회전 상태
        {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1} },
    {  // 세 번째 회전 상태
        {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1} },
    {  // 네 번째 회전 상태
        {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0} }
};

char t_block[4][4][4] =
	{
		{{1, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
		{{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
	};

char s_block[4][4][4] =
	{
		{{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
		{{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
		{{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
	};

char z_block[4][4][4] =
	{
		{{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
	};

char l_block[4][4][4] =
	{
		{{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
		{{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
	};

char j_block[4][4][4] =
	{
		{{0, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
		{{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
	};

char o_block[4][4][4] =
	{
		{{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
	};

/* 테트리스 판을 2차원 배열로 표현
 * 2차원 배열의 2차원 배열
 * 모든 블록의 모양을 표시
 *
 * 19*10 배열
 * 모든 블록의 모양을 표시
 * 모든 블록의 모양을 표시*/

char tetris_table[19][10];

/* 게임 종료 때때마다
 * 이름과 득점수와 
 * 날짜와 시간과 순위를 저장
 * */

typedef struct result
{
	char name[30];
	long point;
	int year;
	int month;
	int day;
	int hour;
	int min;
	int rank;
} ScoreRecord;

int block_number = 0;  /* 블록 번호 */
int next_block_number = 0; /* 다음 블록 번호 */
int block_state = 0; /* 블록 상태, 왼쪽, 오른쪽, 아래, 회전 */

int x = 3, y = 0; /* 블록의 위치 */

int game = GAME_END; /* 게임 시작, 게임 종료 */
int best_point = 0; /* 최고 점수 */

long point = 0; /* 현재 점수 */

int quit_by_user = 0; /* 사용자가 게임을 종료했는지 여부 */

// 함수 선언
void GotoXY(int x, int y);
void set_unblocking(int flag);
int kbhit();
void create_Board();
void print_Board();
void display_Block(int blockNum, int rot, int posX, int posY);
void erase_Block(int blockNum, int rot, int posX, int posY);
int check_bottom(int blockNum, int rot, int posX, int posY);
void fix_Block(int blockNum, int rot, int posX, int posY);
void spawn_NewBlock();
void handle_Input();
int remove_Lines();
void save_score(const char* name, long point);
void search_score();
void print_score_sorted();
void display_NextBlock(int blockNum);
void print_Score();
int display_menu(void);
void clear_Screen();
void msleep(int msec);
void exit_handle(int sig);

// OS별 화면 지우기
void clear_Screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// OS별 밀리초 대기
void msleep(int msec) {
#ifdef _WIN32
    Sleep(msec);
#else
    usleep(msec * 1000);
#endif
}

// 커서 이동 함수
void GotoXY(int x, int y) {
#ifdef _WIN32
    COORD coord;
    coord.X = x - 1;
    coord.Y = y - 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    printf("\033[%d;%dH", y, x);
    fflush(stdout);
#endif
}

// 터미널 설정 변경
void set_unblocking(int flag) {
    (void)flag;
#ifdef _WIN32
#else
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    if (flag) {
        term.c_lflag &= ~(ICANON | ECHO); // 비차단 모드
    } else {
        term.c_lflag |= (ICANON | ECHO); // 차단 모드
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

// 키 입력 확인
int kbhit() {
#ifdef _WIN32
    return _kbhit();
#else
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
#endif
}

// 키 입력 받기
int getch() {
#ifdef _WIN32
    return _getch();
#else
    return getchar();
#endif
}

// 종료 핸들러
void exit_handle(int sig) {
    (void)sig;
    CURSOR_SHOW();
    set_unblocking(0); // 터미널 모드 복구
    exit(0);
}

void create_Board() {
    for (int y = 0; y < Board_Height; y++) {
        for (int x = 0; x < Board_Width; x++) {
            if (y == Board_Height - 1 || x == 0 || x == Board_Width - 1) {
                board[y][x] = 1; // 바닥, 좌우 벽 = 1
            } else {
                board[y][x] = 0; // 빈 공간 = 0
            }
        }
    }
}

void print_Board() {
    for (int y = 0; y < Board_Height; y++) {
        for (int x = 0; x < Board_Width; x++) {
            GotoXY(BoardX + x * 2, BoardY + y);

            if (board[y][x] == 1) { // 벽과 바닥 출력
                if (y == 0 || y == Board_Height - 1 || x == 0 || x == Board_Width - 1) {
                #ifdef _WIN32
                    printf("\033[37m##\033[0m");
                #else
                    printf("\033[37m▩\033[0m");
                #endif
                } else {
                #ifdef _WIN32
                    printf("\033[90m[]\033[0m");
                #else
                    printf("\033[90m■\033[0m");
                #endif
                }
            } else {
                printf("  ");
            }
        }
    }
    fflush(stdout);
}

int game_start(){
    clear_Screen();
    
#ifdef _WIN32
    // 커서 숨기기
    CURSOR_HIDE();
#else
    CURSOR_HIDE();
    fflush(stdout);
#endif
    srand((unsigned int)time(NULL));
    next_block_number = rand() % 7;
    
    create_Board();  // 테트리스 판 생성, 출력
    print_Board();

    set_unblocking(1); // 터미널 비차단 모드 설정
    quit_by_user = 0;
    point = 0;
    
    spawn_NewBlock(); // 첫 블럭 생성

    int tick = 0;

    while (1) {
        handle_Input();  // 실시간 키 입력
        if (game != GAME_START) break; // 게임 종료시 루프 탈출
        
        // 매 0.4초마다 자동 낙하
        if (tick % 4 == 0) {
            int newY = y + 1;

            if (!check_bottom(block_number, block_state, x, newY)) {
                erase_Block(block_number, block_state, x, y);
                y = newY;
                display_Block(block_number, block_state, x, y);
            } else {
                fix_Block(block_number, block_state, x, y); // 블럭 고정
                int lines = remove_Lines(); // 라인 제거
                if (lines > 0) {
                    print_Board();
                    point += lines * 100; // 라인 제거 시 점수 추가
                    print_Score(); // 점수 출력
                }
                if (game == GAME_END) {
                    break;
                }
                if (game == GAME_START) {
                    spawn_NewBlock();
                }  
            }
        }

        msleep(100); // 루프 틱 간격: 0.1초
        tick++;
    }
    
    if(quit_by_user == 0) {
        set_unblocking(0);
        CURSOR_SHOW();

        char name[50];
        GotoXY(BoardX, BoardY + Board_Height + 3);
        printf("\nGAME OVER! 점수: %ld\n", point);
        printf("이름을 입력하세요: ");
        scanf("%s", name);
        save_score(name, point);
        getchar();
    }
    return 1;
}
/* 블록 그리기 */
void display_Block(int blockNum, int rot, int posX, int posY) {
    char (*block)[4][4] = NULL;

    switch (blockNum) {
        case I_BLOCK: block = i_block; break;
        case T_BLOCK: block = t_block; break;
        case S_BLOCK: block = s_block; break;
        case Z_BLOCK: block = z_block; break;
        case L_BLOCK: block = l_block; break;
        case J_BLOCK: block = j_block; break;
        case O_BLOCK: block = o_block; break;
        default: return;
    }

    const char* color = get_Color(blockNum);

    for (int i = 0; i < 4; i++) {  // y축
        for (int j = 0; j < 4; j++) {  // x축
            if (block[rot][i][j]) {
                GotoXY(BoardX + (posX + j) * 2, BoardY + posY + i);
            #ifdef _WIN32
                printf("%s[]\033[0m", color);
            #else
                printf("%s■%s", color, RESET_COLOR);
            #endif
            }
        }
    }
    GotoXY(0, BoardY + Board_Height + 1);
    fflush(stdout);
}
/* 블록 지우기 */
void erase_Block(int blockNum, int rot, int posX, int posY) {
    char (*block)[4][4] = NULL;
    switch (blockNum) {
        case I_BLOCK: block = i_block; break;
        case T_BLOCK: block = t_block; break;
        case S_BLOCK: block = s_block; break;
        case Z_BLOCK: block = z_block; break;
        case L_BLOCK: block = l_block; break;
        case J_BLOCK: block = j_block; break;
        case O_BLOCK: block = o_block; break;
        default: return;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[rot][i][j]) {
                GotoXY(BoardX + (posX + j) * 2, BoardY + posY + i);
                printf("  ");  // 지우기: 공백 2칸
            }
        }
    }
    GotoXY(0, BoardY + Board_Height + 1); // 커서를 보드 아래로 내림
    fflush(stdout);
}
/* 충돌 검사 */
int check_bottom(int blockNum, int rot, int posX, int posY) {
    char (*block)[4][4] = NULL;

    switch (blockNum) {
        case I_BLOCK: block = i_block; break;
        case T_BLOCK: block = t_block; break;
        case S_BLOCK: block = s_block; break;
        case Z_BLOCK: block = z_block; break;
        case L_BLOCK: block = l_block; break;
        case J_BLOCK: block = j_block; break;
        case O_BLOCK: block = o_block; break;
        default: return 1;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[rot][i][j]) {
                int bx = posX + j;
                int by = posY + i;
                if (bx < 0 || bx >= Board_Width || by < 0 || by >= Board_Height)
                    return 1;
                if (board[by][bx] == 1)
                    return 1;
            }
        }
    }
    return 0;
}
/* 블록 고정 */
void fix_Block(int blockNum, int rot, int posX, int posY) {
    char (*block)[4][4] = NULL;
    switch (blockNum) {
        case I_BLOCK: block = i_block; break;
        case T_BLOCK: block = t_block; break;
        case S_BLOCK: block = s_block; break;
        case Z_BLOCK: block = z_block; break;
        case L_BLOCK: block = l_block; break;
        case J_BLOCK: block = j_block; break;
        case O_BLOCK: block = o_block; break;
        default: return;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[rot][i][j]) {
                int bx = posX + j;
                int by = posY + i;
                if (bx >= 1 && bx < Board_Width - 1 && by >= 0 && by < Board_Height - 1) {
                    board[by][bx] = 1;
                    GotoXY(BoardX + bx * 2, BoardY + by);
                #ifdef _WIN32
                    printf("%s[]\033[0m", GRAY_COLOR);
                #else
                    printf("%s■%s", GRAY_COLOR, RESET_COLOR);
                #endif
                }
            }
        }
    }
    fflush(stdout);
}
/* 키 입력 */
void handle_Input() {
    if (!kbhit()) return;

    int ch = getch();
    int newX = x;
    int newY = y;
    int newRot = block_state;

    switch (ch) {
        case LEFT1: case LEFT2: // 'j'
            newX = x - 1;
            break;
        case RIGHT1: case RIGHT2:   // 'l'
            newX = x + 1;
            break;
        case DOWN1: case DOWN2: // 'k'
            newY = y + 1;
            break;
        case ROTATE1: case ROTATE2: // 'i'
            newRot = (block_state + 1) % 4;
            break;
        case DROP1: case DROP2:  // 'a'
            while (!check_bottom(block_number, block_state, x, y + 1)) {
                erase_Block(block_number, block_state, x, y);
                y++;
                display_Block(block_number, block_state, x, y);
                msleep(10);  // 아주 짧은 시간 지연
            }
            return;
        case QUIT1: case QUIT2: // 'p'
            game = GAME_END;
            erase_Block(block_number, block_state, x, y);
            
            set_unblocking(0);      // 터미널 모드 복구
            CURSOR_SHOW();
            fflush(stdout);

            GotoXY(BoardX, BoardY + Board_Height + 3);
            printf("\n게임을 종료합니다.\n");

            printf("이름을 입력하세요: ");
            char name[50];
            scanf("%s", name);
            save_score(name, point);         // 점수 저장
            getchar();

            printf("메뉴로 돌아갑니다...\n");
            msleep(1000);                    // 1초 대기
            clear_Screen();                   // 화면 지우기
            quit_by_user = 1;           // 사용자가 종료했음을 표시
            break; 
        default:
            return;
    }

    if (!check_bottom(block_number, newRot, newX, newY)) {
        erase_Block(block_number, block_state, x, y);  // 이전 위치 지움
        x = newX;
        y = newY;
        block_state = newRot;
        display_Block(block_number, block_state, x, y);   // 새 위치 그림
    }
}
/* 새 블록 스폰 */
void spawn_NewBlock() {
    if (game != GAME_START) return;
    x = 4;
    y = 0;

    block_number = next_block_number;
    next_block_number = rand() % 7;

    display_NextBlock(next_block_number);  // 다음 블럭 ui 표시

    block_state = 0;

    if (check_bottom(block_number, block_state, x, y)) {
        game = GAME_END;
        return;
    }

    display_Block(block_number, block_state, x, y);
    print_Score();
}
/* 라인 제거 */
int remove_Lines() {
    int linesRemoved = 0;

    for (int y = Board_Height - 2; y >= 0; y--) { // 바닥 제외
        int isFull = 1;
        for (int x = 1; x < Board_Width - 1; x++) {
            if (board[y][x] == 0) {
                isFull = 0;
                break;
            }
        }

        if (isFull) {
            linesRemoved++;

            // 윗줄들을 아래로 복사
            for (int row = y; row > 1; row--) {
                for (int col = 1; col < Board_Width - 1; col++) {
                    board[row][col] = board[row - 1][col];
                }
            }

            // 가장 윗 줄을 0으로 초기화
            for (int col = 1; col < Board_Width - 1; col++) {
                board[1][col] = 0;
            }
            
            y++; // 같은 줄 다시 검사
        }
    }

    return linesRemoved;
}
/* 점수 저장 */
void save_score(const char* name, long point) {
    FILE* fp = fopen("score.txt", "r");
    struct result records[20];
    int count = 0;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    // 기존 기록 불러오기
    if (fp) {
        while (fscanf(fp, "%s %ld %d-%d-%d %d:%d",
                      records[count].name, &records[count].point,
                      &records[count].year, &records[count].month, &records[count].day,
                      &records[count].hour, &records[count].min) == 7) {
            count++;
            if (count >= 20) break;
        }
        fclose(fp);
    }

    // 최소 점수 인덱스 찾기
    if (count == 20) {
        int minIdx = 0;
        for (int i = 1; i < 20; i++) {
            if (records[i].point < records[minIdx].point)
                minIdx = i;
        }

        if (point > records[minIdx].point) {
            // 교체
            strcpy(records[minIdx].name, name);
            records[minIdx].point = point;
            records[minIdx].year = t->tm_year + 1900;
            records[minIdx].month = t->tm_mon + 1;
            records[minIdx].day = t->tm_mday;
            records[minIdx].hour = t->tm_hour;
            records[minIdx].min = t->tm_min;
        } else {
            return; // 기록되지 않음
        }
    } else {
        // 20개 미만이면 새 기록 추가
        strcpy(records[count].name, name);
        records[count].point = point;
        records[count].year = t->tm_year + 1900;
        records[count].month = t->tm_mon + 1;
        records[count].day = t->tm_mday;
        records[count].hour = t->tm_hour;
        records[count].min = t->tm_min;
        count++;
    }

    // 점수 내림차순 정렬
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (records[i].point < records[j].point) {
                struct result temp = records[i];
                records[i] = records[j];
                records[j] = temp;
            }
        }
    }

    // 파일에 저장
    fp = fopen("score.txt", "w");
    if (fp) {
        for (int i = 0; i < count; i++) {
            fprintf(fp, "%s %ld %04d-%02d-%02d %02d:%02d\n",
                    records[i].name, records[i].point,
                    records[i].year, records[i].month, records[i].day,
                    records[i].hour, records[i].min);
        }
        fclose(fp);
    }
}
// 점수 검색 //
void search_score() {
    char search_name[50];
    int found = 0;
    FILE* fp = fopen("score.txt", "r");
    if (!fp) {
        printf("기록 파일이 없습니다.\n");
        return;
    }

    printf("검색할 이름을 입력하세요: ");
    scanf("%s", search_name);

    char name[50];
    long score;
    int year, month, day, hour, min;

    printf("\n----- 검색 결과 -----\n");
    while (fscanf(fp, "%s %ld %d-%d-%d %d:%d",
                  name, &score, &year, &month, &day, &hour, &min) != EOF) {
        if (strcmp(name, search_name) == 0) {
            printf("이름: %s | 점수: %ld | 날짜: %04d-%02d-%02d %02d:%02d\n",
                   name, score, year, month, day, hour, min);
            found = 1;
        }
    }
    if (!found) {
        printf("해당 이름의 기록이 없습니다.\n");
    }

    fclose(fp);
}
// 점수 비교 //
int compare_scores(const void* a, const void* b) {
    return (int)(((ScoreRecord*)b)->point - ((ScoreRecord*)a)->point);
}
// 점수 정렬 및 출력 //
void print_score_sorted() {
    FILE* fp = fopen("score.txt", "r");
    if (!fp) {
        printf("기록 파일이 없습니다.\n");
        return;
    }

    ScoreRecord records[100];
    int count = 0;

    while (fscanf(fp, "%s %ld %d-%d-%d %d:%d",
                  records[count].name,
                  &records[count].point,
                  &records[count].year,
                  &records[count].month,
                  &records[count].day,
                  &records[count].hour,
                  &records[count].min) == 7) {
        count++;
        if (count >= 20) break;  // 최대 20개 제한
    }

    fclose(fp);

    // 정렬
    qsort(records, count, sizeof(ScoreRecord), compare_scores);

    // 출력
    printf("\n===== 점수 랭킹 =====\n");
    for (int i = 0; i < count; i++) {
        printf("%2d위 | 이름: %-10s | 점수: %4ld | 날짜: %04d-%02d-%02d %02d:%02d\n",
               i + 1,
               records[i].name,
               records[i].point,
               records[i].year,
               records[i].month,
               records[i].day,
               records[i].hour,
               records[i].min);
    }
}
/* 다음 블록 UI 표시 */
void display_NextBlock(int blockNum) {
    char (*block)[4][4] = NULL;

    switch (blockNum) {
        case I_BLOCK: block = i_block; break;
        case T_BLOCK: block = t_block; break;
        case S_BLOCK: block = s_block; break;
        case Z_BLOCK: block = z_block; break;
        case L_BLOCK: block = l_block; break;
        case J_BLOCK: block = j_block; break;
        case O_BLOCK: block = o_block; break;
        default: return;
    }

    // 출력 기준 위치 조정
    int baseX = BoardX + Board_Width * 2 + 2;
    int baseY = BoardY + 2;

    // 제목
    GotoXY(baseX, baseY - 2);
    printf("NEXT");

    // 기존 영역 지우기
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            GotoXY(baseX + j * 2, baseY + i);
            printf("  ");
        }
    }

    // 새 블럭 출력
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            GotoXY(baseX + j * 2, baseY + i);
           if (block[0][i][j]) {
                const char* color = get_Color(blockNum);
            #ifdef _WIN32
                printf("%s[]%s", color, RESET_COLOR);  // 색상 적용 후 리셋
            #else
                printf("%s■%s", color, RESET_COLOR);
            #endif
}
            else
                printf("  ");
        }
    }
    fflush(stdout);
}
/* 점수 출력 */
void print_Score() {
    int scoreX = BoardX + Board_Width * 2 + 2;  // Next Block 오른쪽에
    int scoreY = BoardY + 8;

    GotoXY(scoreX, scoreY);
    printf("SCORE");

    GotoXY(scoreX, scoreY + 1);
    printf("%6ld", point);  // 점수는 오른쪽 정렬
    fflush(stdout);
}

/* 메뉴 표시*/
int display_menu(void)
{
    char input[10]; // 숫자 판단을 위한한 문자열
    int menu = 0;
    clear_Screen();
    
    while(1)
    {
        GotoXY(4,3);
        printf("\n\n\t\t\t\tText Tetris");
        printf("\n\t\t\t============================");
        printf("\n\t\t\t\tGAME MENU\t\n");
        printf("\n\t\t\t============================");
        printf("\n\t\t\t   1) Game Start");
        printf("\n\t\t\t   2) Search history");
        printf("\n\t\t\t   3) Record Output");
        printf("\n\t\t\t   4) QUIT");
        printf("\n\t\t\t============================");
        printf("\n\t\t\t\t\t SELECT : ");
        
        // 문자열로 입력 받기
        fgets(input, sizeof(input), stdin);
        
        // 숫자인지 검사
        if (sscanf(input, "%d", &menu) != 1 || menu < 1 || menu > 4) {
            printf("\r잘못된 입력입니다. 1~4 사이의 숫자를 입력하세요.");
            fflush(stdout);
            msleep(1000);
            printf("\r\033[K"); // 현재 줄을 지움
            fflush(stdout);
            continue;
        }
        return menu;
    }
    return 0;
}

/// 테트리스 게임 메인 함수
/// 메뉴를 표시하고 사용자의 선택에 따라 게임을 시작하거나 결과를 검색하거나 종료합니다.
/// @param  
/// @return 
int main(void)
{
#ifdef _WIN32
    // Windows ANSI 활성화
    enableWindowsAnsi();
    // Windows 시그널 핸들러 설정
    signal(SIGINT, exit_handle);
#else
    // Unix/Linux/macOS 시그널 핸들러 설정
    signal(SIGINT, exit_handle);
    signal(SIGTERM, exit_handle);
#endif

    int menu = 1;

    while(menu)
    {
        menu = display_menu();

        if(menu == 1)
        {
            game = GAME_START;
            menu = game_start();
        }
        else if(menu == 2)
        {
            search_score(); // 이름으로 기록 검색
            printf("\n엔터를 눌러 계속하세요...");
            getchar(); 
            getchar();
        }
        else if(menu == 3)
        {
            print_score_sorted(); // 기록 출력
            printf("\n엔터를 눌러 계속하세요...");
            getchar();
            menu = 1; // 메뉴로 돌아가기
        }
        else if(menu == 4)
        {
            break;
        }
    }

    CURSOR_SHOW();
    
    set_unblocking(0); // 터미널 모드 복구

    printf("게임을 종료합니다.\n");
    fflush(stdout);
    return 0;
}