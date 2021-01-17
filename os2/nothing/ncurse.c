#include <unistd.h>
#include <stdlib.h>
#include <curses.h>

int main() {
	getchar();
    initscr();

    move(5, 15); // 좌표 (5, 15)로 커서를 이동함
    printw("%s", "Hello World");
    refresh();  ///화면을 갱신

    sleep(2);

    endwin();
    exit(EXIT_SUCCESS);
}
