
#include <curses.h>

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif


#define EXIT_CHAR     'q'
#define RESTART_CHAR  'r'

#define ROW_SIZE    4
#define BOARD_SIZE  ROW_SIZE * ROW_SIZE

#define DISPLAY_LENGTH  4
#define DISPLAY_WIDTH   8

#define DISPLAY_POS_ROW  1
#define DISPLAY_POS_COL  1

#define WHITE   0
#define YELLOW  1
#define RED     2

#define COLOR_NUM 3
const short COLOR_TABLE[COLOR_NUM] = {
	COLOR_WHITE, COLOR_YELLOW, COLOR_RED
};

typedef enum direction_t {
	UP,
	LEFT,
	DOWN,
	RIGHT
} direction_t;

typedef uint8_t tile_t;
typedef unsigned int score_t;


void drawGridDisplay(int, int, int, int, int);
void addTile(tile_t [ROW_SIZE][ROW_SIZE]);
void updateTileDisplay(tile_t [ROW_SIZE][ROW_SIZE]);
void updateScoreDisplay(score_t);
score_t evaluateBoard(tile_t [ROW_SIZE][ROW_SIZE], direction_t);
score_t evaluateRow(tile_t *[ROW_SIZE]);
void swap(tile_t *, tile_t *);
bool checkBoard(tile_t [ROW_SIZE][ROW_SIZE], direction_t);
bool checkRow(tile_t[ROW_SIZE]);
int power(int, int);
chtype lookupColor(tile_t);
void initializeBoard(tile_t [ROW_SIZE][ROW_SIZE]);



int main(void) {

	// Initialize

	initscr();
	raw();
	halfdelay(1);
	noecho();
	curs_set(0);
	keypad(stdscr, true);
	nodelay(stdscr, true);

	if (has_colors()) {
		start_color();

		for (short i = 0; i < COLOR_NUM; ++i) {
			init_pair(i, COLOR_TABLE[i], COLOR_BLACK);
		}
	}

	srand((unsigned int)time(NULL));

	// Start

	char buffer[20] = {0};
	sprintf(buffer, "%c%s", RESTART_CHAR, " - Restart");

	mvaddstr(DISPLAY_POS_ROW - 1, DISPLAY_POS_COL, buffer);

	sprintf(buffer, "%c%s", EXIT_CHAR, " - Quit");
	mvaddstr(DISPLAY_POS_ROW - 1, DISPLAY_POS_COL * ROW_SIZE * DISPLAY_WIDTH - 6, buffer);

	score_t score = 0;
	tile_t board[ROW_SIZE][ROW_SIZE] = { 0 };

	initializeBoard(board);

	drawGridDisplay(DISPLAY_POS_ROW, DISPLAY_POS_COL, DISPLAY_LENGTH, DISPLAY_WIDTH, ROW_SIZE);

	updateTileDisplay(board);

	mvaddstr(DISPLAY_POS_ROW + (DISPLAY_LENGTH * ROW_SIZE) + 1, DISPLAY_POS_COL, "Score:  0");

	refresh();

	// Main loop

	bool movable = true;
	chtype last = ERR;
	while (true) {

		chtype current = tolower(getch());

		bool valid = current != last && current != ERR;
		last = current;

		if (!valid) {
			continue;
		}

		if (current == EXIT_CHAR) {
			break;
		}

		if (current == RESTART_CHAR) {

			score = 0;

			initializeBoard(board);
			updateScoreDisplay(score);
			updateTileDisplay(board);

			mvaddstr(DISPLAY_POS_ROW + (DISPLAY_LENGTH * ROW_SIZE) + 1, DISPLAY_POS_COL * ROW_SIZE * DISPLAY_WIDTH - 7, "         ");

			refresh();
			movable = true;
			continue;
		}

		if (!movable) {
			continue;
		}

		bool directionSet = true;
		direction_t direction = UP;
		switch (current) {
		case 'w': case KEY_UP:
			break;

		case 'a': case KEY_LEFT:
			direction = LEFT;
			break;

		case 's': case KEY_DOWN:
			direction = DOWN;
			break;

		case 'd': case KEY_RIGHT:
			direction = RIGHT;
			break;

		default:
			directionSet = false;
			break;
		}

		if (!directionSet || !checkBoard(board, direction)) {
			continue;
		}

		score += evaluateBoard(board, direction);

		addTile(board);
		updateTileDisplay(board);

		updateScoreDisplay(score);

		refresh();

		movable = false;
		for (direction = 0; direction < 4 && !movable; ++direction) {

			if (checkBoard(board, direction)) {
				movable = true;
			}
		}

		if (!movable) {
			mvaddstr(DISPLAY_POS_ROW + (DISPLAY_LENGTH * ROW_SIZE) + 1, DISPLAY_POS_COL * ROW_SIZE * DISPLAY_WIDTH - 7, "Game Over");
		}
	}

	// Exit

	if (last == EXIT_CHAR) {
		clear();
	}

	endwin();

	return 0;
}


void swap(tile_t *x, tile_t *y) {

	assert(x != NULL && y != NULL);

	tile_t temp = *x;
	*x = *y;
	*y = temp;
}


score_t evaluateBoard(tile_t board[ROW_SIZE][ROW_SIZE], direction_t direction) {

	assert(direction < 4);

	bool loopDirection = direction     <= 1;
	bool axisDirection = direction % 2 == 0;

	score_t score = 0;

	for (int i = 0; i < ROW_SIZE; ++i) {

		tile_t *row[ROW_SIZE] = { NULL };

		int count = 0;
		for (int j = loopDirection ? 0            : ROW_SIZE - 1;
		             loopDirection ? j < ROW_SIZE : j >= 0      ;
		             loopDirection ? ++j          : --j         ) {

			int x = axisDirection ? j : i;
			int y = axisDirection ? i : j;

			row[count++] = &board[x][y];
		}

		score += evaluateRow(row);
	}

	return score;
}


score_t evaluateRow(tile_t *row[ROW_SIZE]) {

	int last = 0;
	for (int i = 0; i < ROW_SIZE; ++i) {

		if (*row[i] != 0) {
			if (i != last) {
				swap(row[last], row[i]);
			}
			++last;
		}
	}

	score_t score = 0;

	for (int i = 1; i < ROW_SIZE; ++i) {

		if (*row[i] != 0 && *row[i] == *row[i - 1]) {

			score += power(2, *row[i] - 1) * 2;

			++*row[i - 1];

			for (int j = i; j < ROW_SIZE - 1; ++j) {
				*row[j] = *row[j + 1];
			}
			*row[ROW_SIZE - 1] = 0;
		}
	}

	return score;
}


bool checkBoard(tile_t board[ROW_SIZE][ROW_SIZE], direction_t direction) {

	assert(direction < 4);

	bool loopDirection = direction <= 1;
	bool axisDirection = direction % 2 == 0;

	for (int i = 0; i < ROW_SIZE; ++i) {

		tile_t row[ROW_SIZE] = { 0 };

		int count = 0;
		for (int j = loopDirection ? 0            : ROW_SIZE - 1;
		             loopDirection ? j < ROW_SIZE : j >= 0      ;
		             loopDirection ? ++j          : --j         ) {

			int x = axisDirection ? j : i;
			int y = axisDirection ? i : j;

			row[count++] = board[x][y];
		}

		if (checkRow(row)) {
			return true;
		}
	}

	return false;
}


bool checkRow(tile_t row[ROW_SIZE]) {

	bool zero = false;
	int last = 0;
	for (int i = 0; i < ROW_SIZE; ++i) {

		if (row[i] != 0) {

			if (zero) {
				return true;
			}
			else if (row[last] == row[i] && last != i) {
				return true;
			}
			else {
				last = i;
			}
		}
		else if (row[i] == 0 && !zero) {
			zero = true;
		}

	}

	return false;
}


void drawGridDisplay(int row, int col, int length, int width, int size) {

	// Write corners

	for (int i = 0; i <= size; ++i) {
		for (int j = 0; j <= size; ++j) {

			chtype ch = 0;

			if (j == 0) {
				if (i == 0) {
					ch = ACS_ULCORNER;
				}
				else if (i == size) {
					ch = ACS_LLCORNER;
				}
				else {
					ch = ACS_LTEE;
				}
			}
			else if (j == size) {
				if (i == 0) {
					ch = ACS_URCORNER;
				}
				else if (i == size) {
					ch = ACS_LRCORNER;
				}
				else {
					ch = ACS_RTEE;
				}
			}
			else if (i == 0) {
				ch = ACS_TTEE;
			}
			else if (i == size) {
				ch = ACS_BTEE;
			}
			else {
				ch = ACS_PLUS;
			}

			int x = row + (i * length);
			int y = col + (j * width);

			mvaddch(x, y, ch);
		}
	}

	// Write horizontal lines

	for (int i = 0; i <= size; ++i) {
		int x = row + (i * length);

		for (int j = 0; j < size * width; ++j) {

			if (j % width != 0) {
				int y = col + j;

				mvaddch(x, y, ACS_HLINE);
			}
		}
	}

	// Write vertical lines

	for (int i = 0; i <= size; ++i) {
		int y = col + (i * width);

		for (int j = 0; j < size * length; ++j) {

			if (j % length != 0) {
				int x = row + j;

				mvaddch(x, y, ACS_VLINE);
			}
		}
	}
}


void updateTileDisplay(tile_t board[ROW_SIZE][ROW_SIZE]) {

	for (int i = 0; i < ROW_SIZE; ++i) {
		for (int j = 0; j < ROW_SIZE; ++j) {

			int x = DISPLAY_POS_ROW + (i * DISPLAY_LENGTH) + (DISPLAY_LENGTH / 2);
			int y = DISPLAY_POS_COL + (j *  DISPLAY_WIDTH) + (DISPLAY_WIDTH  / 2);

			char str[DISPLAY_WIDTH] = { 0 };

			memset(str, ' ', DISPLAY_WIDTH - 1);

			mvaddstr(x, y - (strlen(str) / 2), str);

			if (board[i][j] > 0) {
				sprintf(str, "%d", power(2, board[i][j] - 1));
			}

			attrset(lookupColor(board[i][j]));

			size_t length = strlen(str);
			mvaddstr(x, y - ((length + 1) / 2) + 1, str);

			attrset(A_NORMAL);
		}
	}
}


chtype lookupColor(tile_t value) {

	unsigned int attribute = A_NORMAL;
	unsigned int color     = WHITE;

	switch (value) {
	case 1: case 2:
		break;
	case 3: case 4: case 5: case 6:
		color = RED;
		break;
	case 7:  case 8:  case 9:  case 10: case 11:
	case 12: case 13: case 14: case 15: case 16:
		color = YELLOW;
		break;
	}

	return COLOR_PAIR(color | attribute);
}


int power(int base, int exponent) {

	int result = base;
	for (int i = 0; i < exponent; ++i) {
		result *= base;
	}

	return result;
}


void updateScoreDisplay(score_t score) {

	char str[15] = { 0 };
	memset(str, ' ', sizeof(str) - 1);

	mvaddstr(DISPLAY_POS_ROW + (DISPLAY_LENGTH * ROW_SIZE) + 1, DISPLAY_POS_COL + 8, str);

	sprintf(str, "%d", score);

	mvaddstr(DISPLAY_POS_ROW + (DISPLAY_LENGTH * ROW_SIZE) + 1, DISPLAY_POS_COL + 8, str);
}


void initializeBoard(tile_t board[ROW_SIZE][ROW_SIZE]) {

	memset(board, 0, BOARD_SIZE * sizeof(tile_t));

	addTile(board);
	addTile(board);
}


void addTile(tile_t board[ROW_SIZE][ROW_SIZE]) {

	tile_t valid[BOARD_SIZE] = { 0 };

	int count = 0;
	for (int i = 0; i < BOARD_SIZE; ++i) {

		if (board[i / ROW_SIZE][i % ROW_SIZE] == 0) {

			valid[count++] = i;
		}
	}

	if (count > 0) {

		bool tile = rand() % 10 == 0;

		int position = rand() % count;

		board[valid[position] / ROW_SIZE][valid[position] % ROW_SIZE] = tile ? 2 : 1;
	}
}
