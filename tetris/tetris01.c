#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <sys/select.h>
/* 타이머  */
#define CCHAR 0
#ifdef CTIME
#undef CTIME
#endif
#define CTIME 1

/* 왼쪽, 오른쪽, 아래, 회전  */
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

#define RESET_COLOR  "\033[0m"
#define RED_COLOR    "\033[31m"
#define GREEN_COLOR  "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR   "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR   "\033[36m"
#define WHITE_COLOR  "\033[37m"
#define GRAY_COLOR   "\033[90m"

typedef struct {
    char name[50];
    long score;
    int year, month, day, hour, min;
} ScoreRecord; // 점수 기록 구조체
const char* getBlockColor(int blockNum) {
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
			{{1, 0, 0, 0},   
            {1, 1, 0, 0},   
            {1, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 1, 0},   
            {0, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 1, 0, 0},   
            {1, 1, 0, 0},   
            {0, 1, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 1, 0, 0},   
            {1, 1, 1, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}}
	};


char s_block[4][4][4] =
	{
			{{1, 0, 0, 0},   
            {1, 1, 0, 0},   
            {0, 1, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 1, 1, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 0, 0, 0},   
            {1, 1, 0, 0},   
            {0, 1, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 1, 1, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}}

	};

char z_block[4][4][4] =
	{
			{{0, 1, 0, 0},   
            {1, 1, 0, 0},   
            {1, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {0, 1, 1, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 1, 0, 0},   
            {1, 1, 0, 0},   
            {1, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {0, 1, 1, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}}

	};

char l_block[4][4][4] =
	{
			{{1, 0, 0, 0},   
            {1, 0, 0, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 1, 0},   
            {1, 0, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {0, 1, 0, 0},   
            {0, 1, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 0, 1, 0},   
            {1, 1, 1, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}}

	};

char j_block[4][4][4] =
	{
			{{0, 1, 0, 0},   
            {0, 1, 0, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 0, 0, 0},   
            {1, 1, 1, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {1, 0, 0, 0},   
            {1, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{0, 0, 1, 0},   
            {1, 1, 1, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}}
	};


char o_block[4][4][4] =
	{
			{{1, 1, 0, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}},

			{{1, 1, 0, 0},   
            {1, 1, 0, 0},   
            {0, 0, 0, 0},   
            {0, 0, 0, 0}}

	};


/* 테트리스 판을 2차원 배열로 표현
 * 2차원 배열의 2차원 배열
 * 모든 블록의 모양을 표시
 *
 * 19*10 배열
 * 모든 블록의 모양을 표시
 * 모든 블록의 모양을 표시*/

char tetris_table[19][10];
void GotoXY(int x, int y);
void set_unblocking(int flag);
int kbhit();
void createBoard();
void printBoard();
void drawBlock(int blockNum, int rot, int posX, int posY);
void eraseBlock(int blockNum, int rot, int posX, int posY);
int isCollision(int blockNum, int rot, int posX, int posY);
void fixBlockToBoard(int blockNum, int rot, int posX, int posY);
void spawnNewBlock();
void handleInput();
int removeFullLines();
void save_score(const char* name, long point);
void search_score();
void print_score_sorted();
void drawNextBlock(int blockNum);
void printScore();

void handle_exit(int sig) {
    printf("\033[?25h"); // 커서 보이기
    set_unblocking(0); // 터미널 모드 복구
    exit(0);
}

void GotoXY(int x, int y) {
    printf("\033[%d;%dH", y, x);
    fflush(stdout);
}

void createBoard() {
    for (int y = 0; y < Board_Height; y++) {
        for (int x = 0; x < Board_Width; x++) {
            if (y == Board_Height - 1 || x == 0 || x == Board_Width - 1) {
                board[y][x] = 1; // 바닥, 좌우 벽
            } else {
                board[y][x] = 0;
            }
        }
    }
}
void printBoard() {
    for (int y = 0; y < Board_Height; y++) {
        for (int x = 0; x < Board_Width; x++) {
            GotoXY(BoardX + x * 2, BoardY + y);

            if (board[y][x] == 1) {
                if (y == 0 || y == Board_Height - 1 || x == 0 || x == Board_Width - 1)
                    printf("\033[37m▩\033[0m"); // 하얀 벽
                else
                    printf("\033[90m■\033[0m"); // 고정된 블럭은 회색
            }   else {
                    printf(" ");
            }
        }
    }
    fflush(stdout);
}

/* 게임 종료 때마다
 * 이름과 득점수와 
 * 날짜와 시간과 순위를 저장
 * */
static struct result
{
	char name[30];
	long point;
	int year;
	int month;
	int day;
	int hour;
	int min;
	int rank;
}temp_result;

int block_number = 0;  /*블록 번호*/
int next_block_number = 0; /*다음 블록 번호 */
int block_state = 0; /*블록 상태, 왼쪽, 오른쪽, 아래, 회전  */

int x = 3, y = 0; /*블록의 위치*/

int game = GAME_END; /*게임 시작, 게임 종료*/
int best_point = 0; /* 최고 점수*/

long point = 0; /* 현재 점수*/

int quit_by_user = 0;


int display_menu(void); /* 메뉴 표시*/

void set_unblocking(int flag)
{
	// 터미널 설정을 변경하여 입력을 비차단 모드로 설정
	struct termios term;
	tcgetattr(STDIN_FILENO, &term);
	if (flag) {
		term.c_lflag &= ~(ICANON | ECHO); // 비차단 모드
	} else {
		term.c_lflag |= (ICANON | ECHO); // 차단 모드
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

int kbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}
int game_start(){
    system("clear");
    printf("\033[?25l"); // 커서 숨기기
    fflush(stdout);

    srand(time(NULL));
    next_block_number = rand() % 7;
    spawnNewBlock();

    createBoard();
    printBoard();

    set_unblocking(1);
    srand(time(NULL));
    spawnNewBlock(); // 첫 블럭 생성

    int tick = 0;

    while (1) {
        handleInput();  // 실시간 키 입력
        if (game != GAME_START) break;
        // 매 0.5초마다 자동 낙하
        if (tick % 5 == 0) {
            int newY = y + 1;

            if (!isCollision(block_number, block_state, x, newY)) {
                eraseBlock(block_number, block_state, x, y);
                y = newY;
                drawBlock(block_number, block_state, x, y);
            } else {
                // 바닥 또는 블럭과 충돌 → 즉시 고정
                fixBlockToBoard(block_number, block_state, x, y);
                int lines = removeFullLines();
                if (lines > 0) {
                    printBoard();
                    point += lines * 100; // 라인 제거 시 점수 추가
                    printScore(); // 점수 출력
                }
                if (game == GAME_END) {
                    break;
                }
                if (game == GAME_START) {
                    spawnNewBlock();
                }  
            }
        }

        usleep(100000); // 루프 틱 간격: 0.1초
        tick++;
    }
    if(quit_by_user == 0) {
        set_unblocking(0);
        printf("\033[?25h"); // 커서 보이기

        char name[50];
        GotoXY(0, BoardY + Board_Height + 2);
        printf("\nGAME OVER! 점수: %ld\n", point);
        printf("이름을 입력하세요: ");
        scanf("%s", name);
        save_score(name, point);
    }
    // 메뉴로 복귀
    return 1;
}

void drawBlock(int blockNum, int rot, int posX, int posY) {
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

    const char* color = getBlockColor(blockNum);

    for (int i = 0; i < 4; i++) {  // y축
        for (int j = 0; j < 4; j++) {  // x축
            if (block[rot][i][j]) {
                GotoXY(BoardX + (posX + j) * 2, BoardY + posY + i);
                printf("%s■%s", color, RESET_COLOR);
            }
        }
    }
    GotoXY(0, BoardY + Board_Height + 1);
    fflush(stdout);
}


int isCollision(int blockNum, int rot, int posX, int posY) {
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
void handleInput() {
    if (!kbhit()) return;

    int ch = getchar();
    int newX = x;
    int newY = y;
    int newRot = block_state;

    switch (ch) {
        case LEFT1: case LEFT2:
            newX = x - 1;
            break;
        case RIGHT1: case RIGHT2:
            newX = x + 1;
            break;
        case DOWN1: case DOWN2:
            newY = y + 1;
            break;
        case ROTATE1: case ROTATE2:
            newRot = (block_state + 1) % 4;
            break;
        case DROP1: case DROP2:  // 'a'
            while (!isCollision(block_number, block_state, x, y + 1)) {
                eraseBlock(block_number, block_state, x, y);
                y++;
                drawBlock(block_number, block_state, x, y);
                usleep(10000);  // 아주 짧은 시간 지연 (시각적 효과용)
            }
            return;
        case QUIT1: case QUIT2:
            game = GAME_END;
            eraseBlock(block_number, block_state, x, y);
            
            set_unblocking(0);               // 터미널 모드 복구
            printf("\033[?25h");             // 커서 다시 보이기
            fflush(stdout);

            GotoXY(0, BoardY + Board_Height + 2);
            printf("\n게임을 종료합니다.\n");

            printf("이름을 입력하세요: ");
            char name[50];
            scanf("%s", name);
            getchar();
            save_score(name, point);         // 점수 저장
            getchar();
            printf("메뉴로 돌아갑니다...\n");
            sleep(2);                         // 2초 대기
            system("clear"); // 화면 지우기
            quit_by_user = 1;
            break; 
        default:
            return;
    }

    if (!isCollision(block_number, newRot, newX, newY)) {
        eraseBlock(block_number, block_state, x, y);  // 이전 위치 지움
        x = newX;
        y = newY;
        block_state = newRot;
        drawBlock(block_number, block_state, x, y);   // 새 위치 그림
    }
    
}

void eraseBlock(int blockNum, int rot, int posX, int posY) {
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
void fixBlockToBoard(int blockNum, int rot, int posX, int posY) {
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
                    printf("%s■%s", GRAY_COLOR, RESET_COLOR);
                }
            }
        }
    }
    fflush(stdout);
}

void spawnNewBlock() {
    if (game != GAME_START) return;
    x = 4;
    y = 1;

    block_number = next_block_number;
    next_block_number = rand() % 7;

    drawNextBlock(next_block_number);  // 다음 블럭 표시

    block_state = 0;

    if (isCollision(block_number, block_state, x, y)) {
        game = GAME_END;
        return;
    }

    drawBlock(block_number, block_state, x, y);
    printScore();
}
int removeFullLines() {
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

            // 가장 위 줄(1번째 줄)을 0으로 초기화 (0번째는 천장 또는 벽)
            for (int col = 1; col < Board_Width - 1; col++) {
                board[1][col] = 0;
            }
            
            y++; // 같은 줄을 다시 검사 (위에서 한 줄 내렸기 때문)
        }
    }

    return linesRemoved;
}
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

    // 새 점수보다 낮은 점수가 있으면 교체할 수 있도록 정렬 (낮은 점수 맨 뒤로)
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
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s %ld %04d-%02d-%02d %02d:%02d\n",
                records[i].name, records[i].point,
                records[i].year, records[i].month, records[i].day,
                records[i].hour, records[i].min);
    }
    fclose(fp);
}

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
int compare_scores(const void* a, const void* b) {
    return ((ScoreRecord*)b)->score - ((ScoreRecord*)a)->score;
}
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
                  &records[count].score,
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
               records[i].score,
               records[i].year,
               records[i].month,
               records[i].day,
               records[i].hour,
               records[i].min);
    }
}
void drawNextBlock(int blockNum) {
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
            if (block[0][i][j])
                printf("■");
            else
                printf("  ");
        }
    }
    fflush(stdout);
}

void printScore() {
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
	// 
    char input[10]; // 입력을 받을 문자열
    // 메뉴 선택 변수
    // 1: 게임 시작, 2: 기록 검색, 3: 기록 출력, 4: 종료
	int menu = 0;
    system("clear");
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
            sleep(1);
            printf("\r\033[K"); // 현재 줄을 깨끗하게 지움
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
            getchar(); getchar();
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
			exit(0);
		}
	}
    signal(SIGINT, handle_exit); // Ctrl+C로 종료 시 핸들러 호출
    signal(SIGTERM, handle_exit); // 프로그램 종료 시 핸들러 호출

    printf("\033[?25h"); // 커서 보이기
    set_unblocking(0); // 터미널 모드 복구

    printf("게임을 종료합니다.\n");
    fflush(stdout);
	return 0;
}