/* keypad.c */
#include <curses.h>

#define LOCAL_ESCAPE_KEY 27

int main()
{
  int key;

  initscr();
  crmode();
  keypad(stdscr, TRUE);

  /* echo off하고 약간 메세지를 출력한다. 프로그램은 키입력을 기다려서 'q'면 종료하고, 에러가 나지 않았으면 뭐가 눌렸는지 대충 출력해본다. */
  noecho();

  clear();
  mvprintw(5, 5, "Key pad demonstration. Press 'q' to quit.");
  move(7, 5);
  refresh();

  key = getch();
  while(key != ERR && key != 'q') {
    move(7, 5);
    clrtoeol();

    if((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {
      printw("Key was %c", (char)key);
    }
    else {
      switch(key) {
      case LOCAL_ESCAPE_KEY: printw("%s", "Escape key"); break;
      case KEY_END: printw("%s", "END key"); break;
      case KEY_BEG: printw("%s", "BEGINNING key"); break;
      case KEY_RIGHT: printw("%s", "RIGHT key"); break;
      case KEY_LEFT: printw("%s", "LEFT key"); break;
      case KEY_UP: printw("%s", "UP key"); break;
      case KEY_DOWN: printw("%s", "DOWN key"); break;
      default: printw("Unmatched - %d", key); break;
      } /* switch */
    } /* else */

    refresh();
    key = getch();
  } /* while */

  endwin();
  return 0;
}
