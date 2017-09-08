#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define MAZE_NAME "road"
#define ROAD 0
#define WALL 1
#define COIN 2
#define EXPLORED 3

#ifndef ANIM
#define show_result print_result
# ifdef DEBUG
#   define notify_walk print_walk
# else
#   define notify_walk noop
# endif
#else
#include <ncurses.h>
#define notify_walk draw_walk
#define show_result draw_result
#endif

#define push(y, x) {\
  ++top;\
  stack[top][0] = y;\
  stack[top][1] = x;\
}

#define pop() {\
  y = stack[top][0] ;\
  x = stack[top][1];\
  --top;\
}

#define move(d) {\
  next_x = x + direction[d][1];\
  next_y = y + direction[d][0];\
}

#define mark(y, x) {\
  maze[y][x] = EXPLORED;\
}

#define walkable(y, x) (maze[y][x] == COIN || maze[y][x] == ROAD)

static int direction[4][2] = {
  {-1, 0}, // Up
  {0, 1}, // Right
  {1, 0}, // Down
  {0, -1} // Left
};

void noop(int maze[100][100], int size, int y, int x);
void print_walk(int maze[100][100], int size, int y, int x);
void show_result(int coin);

#ifdef ANIM
void draw_map(int maze[100][100], int size);
void draw_walk(int maze[100][100], int size, int y, int x);
void draw_status(int coin);
void draw_result(int coin);
#endif

int main() {
  int maze[100][100], stack[200][2];
  int size, coin = 0, top = -1;
  int end_x, end_y, x, y;
  FILE *fp = fopen(MAZE_NAME, "r");

  // Initialize
  fscanf(fp, "%d", &size);
  fscanf(fp, "%d %d %d %d", &x, &y, &end_x, &end_y);
  for(int i = 0; i < size; ++i) {
    for(int j = 0; j < size; ++j) {
      fscanf(fp, "%d", &maze[i][j]);
    }
  }
  fclose(fp);

#ifdef ANIM
  initscr();
  start_color();
  init_pair(1, COLOR_BLUE, COLOR_BLUE);
  init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(3, COLOR_GREEN, COLOR_GREEN);
  init_pair(4, COLOR_RED, COLOR_RED);
  draw_walk(maze, size, y, x);
  draw_status(coin);
#endif

#ifdef DEBUG
  printf("Start (%d, %d) End (%d, %d)\n", y, x, end_y, end_x);
#endif
  // Main
  while(x != end_x || y != end_y) {
    int next_x, next_y;
    bool found_path = false;
    push(y, x);
    for(int i = 0; i < 4; ++i) {
      move(i);
      if(walkable(next_y, next_x)) {
        x = next_x;
        y = next_y;
        if(maze[y][x] == COIN) {
          ++coin;
#ifdef ANIM
          draw_status(coin);
#endif
        }
        mark(y, x);
        found_path = true;
        notify_walk(maze, size, y, x);
        break;
      }
    }

    // Run backtrace
    while(!found_path) {
      pop();
#ifdef DEBUG
      printf("Backtrace %d, %d\n", y, x);
#endif
      notify_walk(maze, size, y, x);
      for(int i = 0; i < 4; ++i) {
        move(i);
        if(walkable(next_y, next_x)) {
          x = next_x;
          y = next_y;
          if(maze[y][x] == COIN) {
            ++coin;
#ifdef ANIM
            draw_status(coin);
#endif
          }
          mark(y, x);
          found_path = true;
          notify_walk(maze, size, y, x);
          break;
        }
      }
    }
  }
  show_result(coin);
#ifdef ANIM
  endwin();
#endif

  return 0;
}

void noop(int maze[100][100], int size, int y, int x) {
  ;
}

void print_walk(int maze[100][100], int size, int y, int x) {
  printf("Walk: %d, %d\n", y, x);
}

void print_result(int coin) {
  printf("%d\n", coin);
}

#ifdef ANIM
void draw_map(int maze[100][100], int size) {
  for(int i = 0; i < size; ++i) {
    for(int j = 0; j < size; ++j) {
      attron(COLOR_PAIR(maze[i][j]));
      mvaddch(i, j + 8, ' ');
      attroff(COLOR_PAIR(maze[i][j]));
    }
  }
}

void draw_walk(int maze[100][100], int size, int y, int x) {
  draw_map(maze, size);
  attron(COLOR_PAIR(4));
  mvaddch(y, x + 8, ' ');
  attroff(COLOR_PAIR(4));
  refresh();
  usleep(300000);
}

void draw_status(int coin) {
  mvprintw(0, 0, "Coin:%2d", coin);
  refresh();
}

void draw_result(int coin) {
  mvprintw(1, 0, "Done!!");
  refresh();
  sleep(3);
}
#endif
