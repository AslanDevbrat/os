#include <stdio.h> 
#include <ncurses.h> 
#define ctrl(x)   ((x) & 0x1f) 

int 
main(void) 
{ 
 int c; 
 initscr(); 
 keypad(stdscr, TRUE); 
 cbreak(); 
 noecho(); 
 nonl(); 
 c = getch(); 
 switch (c) { 
 case KEY_ENTER: 
  printw("\nkey_enter: %d", c); 
  break; 
 case ctrl('j'): 
  printw("\nkey: ctrl j"); 
  break; 
 default: 
  printw("\nkeyname: %d = %s\n", c, keyname(c)); 
  break; 
 } 
 printw("\nnow press a key to end"); 
 getch(); 
 endwin(); 
 return 0; 
} 
