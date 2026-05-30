#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 25
#define COLS 80
#define SPEED_START 100
#define SPEED_STEP 20
#define SPEED_MIN 20
#define SPEED_MAX 1000

void read_field(int** grid);
void paint_field(int** grid, int speed);
int user_input(int* speed);
int** make_grid();
void free_grid_mem(int** grid);
int** game_tick(int** grid);
int get_amount_of_live_neighbours(int** grid, int x, int y);
int true_x_coord(int x);
int true_y_coord(int y);
void init_screen();

int main() {
    int speed = SPEED_START;

    int** grid = make_grid();
    if (grid == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    read_field(grid);

    if (freopen("/dev/tty", "r", stdin) == NULL) {
        free_grid_mem(grid);
        fprintf(stderr, "Failed to reopen stdin to /dev/tty\n");
        return 1;
    }

    init_screen();

    speed = SPEED_START;

    paint_field(grid, speed);

    int running = 1;
    while (running) {
        int** new_grid = game_tick(grid);

        free_grid_mem(grid);
        grid = new_grid;
        running = user_input(&speed);
        paint_field(grid, speed);
        napms(speed);
    }

    free_grid_mem(grid);
    endwin();

    return 0;
}

void init_screen() {
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    curs_set(0);
}

void read_field(int** grid) {
    char line[COLS + 2];

    for (int i = 0; i < ROWS; i++) {
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        for (int j = 0; j < COLS; j++) {
            if (line[j] == '1') {
                grid[i][j] = 1;
            } else {
                grid[i][j] = 0;
            }
        }
    }
}

void paint_field(int** grid, int speed) {
    clear();
    for (int x = 0; x < COLS + 2; x++) {
        if (x == 0 || x == COLS + 1)
            mvaddch(0, x, '+');
        else
            mvaddch(0, x, '-');
    }
    for (int i = 0; i < ROWS; i++) {
        mvaddch(i + 1, 0, '|');
        for (int j = 0; j < COLS; j++) {
            char ch = (grid[i][j] ? 'O' : '.');
            mvaddch(i + 1, j + 1, ch);
        }
        mvaddch(i + 1, COLS + 1, '|');
    }
    for (int x = 0; x < COLS + 2; x++) {
        if (x == 0 || x == COLS + 1)
            mvaddch(ROWS + 1, x, '+');
        else
            mvaddch(ROWS + 1, x, '-');
    }
    mvprintw(ROWS + 2, 0, "A=fast Z=slow SPACE=quit SPEED=%d", speed);
    refresh();
}

int user_input(int* speed) {
    int key = getch();
    if (key == ' ') return 0;
    if (key == 'a' || key == 'A') {
        *speed -= SPEED_STEP;
        if (*speed < SPEED_MIN) *speed = SPEED_MIN;
    }
    if (key == 'z' || key == 'Z') {
        *speed += SPEED_STEP;
        if (*speed > SPEED_MAX) *speed = SPEED_MAX;
    }
    return 1;
}

int** make_grid() {
    int** data = calloc(ROWS, sizeof(int*));
    int* rows_data = calloc(ROWS * COLS, sizeof(int));
    int** result = NULL;

    if (data != NULL && rows_data != NULL) {
        for (int i = 0; i < ROWS; i++) {
            data[i] = (rows_data + COLS * i);
        }
        result = data;
    }

    if (result == NULL) {
        free(rows_data);
        free(data);
    }

    return result;
}

void free_grid_mem(int** grid) {
    if (grid == NULL) return;
    if (*grid != NULL) free(*grid);
    free(grid);
}

int true_x_coord(int x) {
    int true_x = x;
    if (x < 0)
        true_x = COLS + x;
    else if (x >= COLS)
        true_x = x - COLS;
    return true_x;
}

int true_y_coord(int y) {
    int true_y = y;
    if (y < 0)
        true_y = ROWS + y;
    else if (y >= ROWS)
        true_y = y - ROWS;
    return true_y;
}

int get_amount_of_live_neighbours(int** grid, int x, int y) {
    int sum = 0;
    for (int cx = x - 1; cx <= x + 1; cx++) {
        for (int cy = y - 1; cy <= y + 1; cy++) {
            if (cx != x || cy != y) sum += grid[true_y_coord(cy)][true_x_coord(cx)];
        }
    }
    return sum;
}

int** game_tick(int** grid) {
    int** new_grid = make_grid();

    if (new_grid == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation failed in game_tick\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int nei_amount = get_amount_of_live_neighbours(grid, j, i);

            if (!grid[i][j] && nei_amount == 3) {
                new_grid[i][j] = 1;
            } else if (grid[i][j] && (nei_amount == 2 || nei_amount == 3)) {
                new_grid[i][j] = 1;
            } else {
                new_grid[i][j] = 0;
            }
        }
    }

    return new_grid;
}