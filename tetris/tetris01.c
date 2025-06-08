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
			{{1, 0, 0, 0},   {1, 1, 0, 0},   {1, 0, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 1, 0},   {0, 1, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{0, 0, 1, 0},   {0, 1, 1, 0},   {0, 0, 1, 0},   {0, 0, 0, 0}},
			{{0, 0, 0, 0},   {0, 1, 0, 0},   {1, 1, 1, 0},   {0, 0, 0, 0}}
	};


char s_block[4][4][4] =
	{
			{{1, 0, 0, 0},   {1, 1, 0, 0},   {0, 1, 0, 0},   {0, 0, 0, 0}},
			{{0, 1, 1, 0},   {1, 1, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{0, 1, 0, 0},   {0, 1, 1, 0},   {0, 0, 1, 0},   {0, 0, 0, 0}},
			{{0, 0, 0, 0},   {0, 1, 1, 0},   {1, 1, 0, 0},   {0, 0, 0, 0}}

	};

char z_block[4][4][4] =
	{
			{{0, 1, 0, 0},   {1, 1, 0, 0},   {1, 0, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 0, 0},   {0, 1, 1, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{0, 0, 1, 0},   {0, 1, 1, 0},   {0, 1, 0, 0},   {0, 0, 0, 0}},
			{{0, 0, 0, 0},   {1, 1, 0, 0},   {0, 1, 1, 0},   {0, 0, 0, 0}}

	};

char l_block[4][4][4] =
	{
			{{1, 0, 0, 0},   {1, 0, 0, 0},   {1, 1, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 1, 0},   {1, 0, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{0, 1, 1, 0},   {0, 0, 1, 0},   {0, 0, 1, 0},   {0, 0, 0, 0}},
			{{0, 0, 0, 0},   {0, 0, 1, 0},   {1, 1, 1, 0},   {0, 0, 0, 0}}

	};

char j_block[4][4][4] =
	{
			{{0, 1, 0, 0},   {0, 1, 0, 0},   {1, 1, 0, 0},   {0, 0, 0, 0}},
			{{1, 0, 0, 0},   {1, 1, 1, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 0, 0},   {1, 0, 0, 0},   {1, 0, 0, 0},   {0, 0, 0, 0}},
			{{0, 0, 0, 0},   {0, 0, 1, 0},   {1, 1, 1, 0},   {0, 0, 0, 0}}
	};


char o_block[4][4][4] =
	{
			{{1, 1, 0, 0},   {1, 1, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 1, 0},   {1, 1, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 0, 0},   {1, 1, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}},
			{{1, 1, 0, 0},   {1, 1, 0, 0},   {0, 0, 0, 0},   {0, 0, 0, 0}}

	};


/* 테트리스 판을 2차원 배열로 표현
 * 2차원 배열의 2차원 배열
 * 모든 블록의 모양을 표시
 *
 * 18*8 배열
 * 모든 블록의 모양을 표시
 * 모든 블록의 모양을 표시*/

char tetris_table[19][10];

void GotoXY(int x, int y) {
    printf("\033[%d;%dH", y, x);
    fflush(stdout);
}

void createBoard(){
	for (int y =0; y < Board_Height; y++) {
		board[y][0] = 1; // 왼쪽 벽
		board[y][Board_Width - 1] = 1; // 오른쪽 벽
		for(int x = 0; x < Board_Width; x++) {
			if (y == 0)
				board[y][x] = 0;
				board[Board_Height - 1][x] = 1; // 바닥
			if (y > 0 && y < Board_Height - 1) 
				if(x > 0 && x < Board_Width - 1)
					board[y][x] = 0; // 빈 공간
		}
	}
}

void printBoard() {
	// 테트리스 판을 출력
	for(int x = 1; x < 10; x++) {
		GotoXY(BoardX + x * 2, BoardY+1);
		printf("_");
	}

	for (int y = 0; y < Board_Height; y++) {
		GotoXY(BoardX, BoardY + y);
		if (board [y][0]==1){
			printf("▨");
		}
		if (board[y][Board_Width - 1] == 1) {
			GotoXY(BoardX + Board_Width * 2 - 2, BoardY + y);
			printf("▨");
		}
	}

	for (int x = 0; x < Board_Width; x++) {
		GotoXY (BoardX + x * 2, BoardY + Board_Height-1);
		if(board[Board_Height - 1][x] == 1) {
			printf("▨");
		}
	}
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
    createBoard();
    printBoard();

    set_unblocking(1);

    spawnNewBlock(); // 첫 블럭

    int tick = 0;

    while (1) {
        handleInput();  // 키 입력 처리

        // 매 10틱마다 자동 낙하 (약 1초 주기)
        if (tick % 10 == 0) {
            int newY = y + 1;
            if (!isCollision(block_number, block_state, x, newY)) {
                eraseBlock(block_number, block_state, x, y);
                y = newY;
                drawBlock(block_number, block_state, x, y);
            } else {
                // 바닥 또는 충돌 → 고정
                fixBlockToBoard(block_number, block_state, x, y);
                spawnNewBlock(); // 다음 블럭
            }
        }

        usleep(100000); // 0.1초 루프
        tick++;
    }

    set_unblocking(0);
    printf("\033[?25h"); // 커서 복구
    return 0;
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

    for (int i = 0; i < 4; i++) {  // y축
        for (int j = 0; j < 4; j++) {  // x축
            if (block[rot][i][j]) {
                GotoXY(BoardX + (posX + j) * 2, BoardY + posY + i);
                printf("■");
            }
        }
    }
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
                if (bx >= 0 && bx < Board_Width && by >= 0 && by < Board_Height)
                    board[by][bx] = 1;
            }
        }
    }
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
                if (bx >= 0 && bx < Board_Width && by >= 0 && by < Board_Height)
                    board[by][bx] = 1;
            }
        }
    }
}
void spawnNewBlock() {
    x = 3;
    y = 1;
    block_number = rand() % 7;
    block_state = 0;
    drawBlock(block_number, block_state, x, y);
}

/* 메뉴 표시*/
int display_menu(void)
{
	// 
	int menu = 0;

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
		scanf("%d",&menu);
		if(menu < 1 || menu > 4)
		{
			continue;
		}
		else
		{
			return menu;
		}
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
			//search_result();
		}
		else if(menu == 3)
		{
			//print_result();
		}
		else if(menu == 4)
		{
			exit(0);
		}
	}

	return 0;
}