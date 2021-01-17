/* subscl.c */
#include <unistd.h>
#include <curses.h>

int main()
{
  WINDOW *sub_window_ptr;
  int x_loop;
  int y_loop;
  int counter;
  char a_letter = '1';

  initscr();

  for(x_loop = 0; x_loop < COLS -1; x_loop++) {
    for(y_loop = 0; y_loop < LINES -1; y_loop++) {
      mvaddch(stdscr, y_loop, x_loop, a_letter);
      a_letter++;
      if(a_letter > '9') a_letter = '1';
    }
  }

  /* 스크롤 할 보조윈도우를 만들고, 리프레시하기전에 한번 만진다. */
  sub_window_ptr = subwin(stdscr, 10, 20, 10, 10);
  scrollok(sub_window_ptr, 1);

  touchwin(stdscr);
  refresh();
  sleep(1);

  /* 보조 윈도우의 내용을 지우고 보조 윈도우에 텍스트를 출력하고 리프레시한다. 스크롤하는 텍스트는 루프에 의해 만들어짐ㅋ */
  werase(sub_window_ptr);
  mvwprintw(sub_window_ptr, 2, 0, "%s", "This window will now scroll");
  wrefresh(sub_window_ptr);
  sleep(1);

  for(counter = 1; counter < 10; counter++) {
    wprintw(sub_window_ptr, "%s", "This text is both wrapping and scrolling.");
    wrefresh(sub_window_ptr);
    scroll(1);
  }

  /* 루프가 끝나면 보조윈도우를 제거하고 기본화면을 리프레시한다. */
  delwin(sub_window_ptr);

  touchwin(stdscr);
  refresh();
  sleep(1);

  endwin();
  return 0;
}
