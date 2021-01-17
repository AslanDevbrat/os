/* cdapp.c */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>

#define MAX_STRING (80)   /* 제일 긴 문자열 */
#define MAX_ENTRY (1024)  /* 제일 긴 데이터베이스 엔트리 */

#define MESSAGE_LINE  6   /* 기타 메세지 표현할 라인 */
#define ERROR_LINE   22   /* 에러를 표시할 라인 */
#define Q_LINE       20   /* 질문을 표시할 라인 */
#define PROMPT_LINE  18   /* 입력 프롬프트를 표시할 라인 */

/* 몇가지 전역변수를 정의한다. current_cd는 현재 CD의 제목을 저장, 초기화는 null로, 선택된 CD가 없다는걸 보여준다. current_cat는 현재 CD의 카탈로그 번호를 기록하는데 사용 */
static char current_cd[MAX_STRING] = "\0";
static char current_cat[MAX_STRING];

/* 몇몇 파일 이름을 정의한다. 프로그램을 간단하게 하기 위해 파일 이름을 고정시킨다. */
const char *title_file = "title.cdb";
const char *tracks_file = "tracks.cdb";
const char *temp_file="cdb.tmp";

/* 프로그램에서 필요한 함수 원형 */
void clear_all_screen(void);
void get_return(void);
int get_confirm(void);
int getchoice(char *greet, char *choices[]);
void draw_menu(char *options[], int highlight, int start_row, int start_col);
void insert_title(char *cdtitle);
void get_string(char *string);
void add_record(void);
void count_cds(void);
void find_cd(void);
void list_tracks(void);
void remove_tracks(void);
void remove_cd(void);
void update_cd(void);

/* 메뉴를 출력하기 위한 문자열 배열. 첫번째 문자는 메뉴가 선택됐을때 반환될 문자고, 나머지 텍스트는 출력될 문자다. 현재 선택된 CD가 있을 경우에는 extended_menu에 있는 메뉴를 출력한다. */
char *main_menu[] = {
  "aadd new CD",
  "ffind CD",
  "ccount CDs and tracks in the catalog",
  "qquit",
  0,
};

char *extended_menu[] = {
  "aadd new CD",
  "ffind CD",
  "ccount CDs and tracks in the catalog",
  "llist tracks on current CD",
  "rremove current CD",
  "uupdate track information",
  "qquit",
  0,
};

/* main은 메뉴를 선택하고 q를 누르면 종료한다. */
int main()
{
  int choice;
  initscr();
  do {
    choice = getchoice("Options:", current_cd[0] ? extended_menu : main_menu);
    switch(choice) {
    case 'q':
      break;
    case 'a':
      add_record();
      break;
    case 'c':
      count_cds();
      break;
    case 'f':
      find_cd();
      break;
    case 'l':
      list_tracks();
      break;
    case 'r':
      remove_cd();
      break;
    case 'u':
      update_cd();
      break;
    }
  } while (choice != 'q');
  endwin();
  return 0;
}

/* getchoice 함수는 main에서 호출되는 함수고, greet랑 choices가 인자로 전달된다. choices는 main_menu나 extended_menu에 대한 포인터다. */
int getchoice(char *greet, char *choices[])
{
  static int selected_row = 0;
  int max_row = 0;
  int start_screenrow = MESSAGE_LINE, start_screencol = 10;
  char **option;
  int selected;
  int key = 0;

  option = choices;
  while(*option) {
    max_row++;
    option++;
  }

  if(selected_row >= max_row)
    selected_row = 0;
  clear_all_screen();
  mvprintw(start_screenrow - 2, start_screencol, greet);

  keypad(stdscr, TRUE);
  cbreak();
  noecho();

  key = 0;
  while (key != 'q' && key != KEY_ENTER && key != '\n') {
    if(key == KEY_UP) {
      if(selected_row == 0)
        selected_row = max_row -1;
      else
        selected_row--;
    }
    if(key == KEY_DOWN) {
      if(selected_row == (max_row -1))
        selected_row = 0;
      else
        selected_row++;
    }

    selected = *choices[selected_row];
    draw_menu(choices, selected_row, start_screenrow, start_screencol);
    key = getch();
  }

  keypad(stdscr, FALSE);
  nocbreak();
  echo();

  if(key == 'q')
    selected = 'q';

  return (selected);
}

/* getchoice 함수 내부에서 호출된 clear_all_screen, draw_menu 함수 */
void draw_menu(char *options[], int current_highlight, int start_row, int start_col)
{
  int current_row = 0;
  char **option_ptr;
  char *txt_ptr;

  option_ptr = options;

  while (*option_ptr) {
    if(current_row == current_highlight) {
      mvaddch(start_row + current_row, start_col - 3, ACS_BULLET);
      mvaddch(start_row + current_row, start_col + 40, ACS_BULLET);
    } else {
      mvaddch(start_row + current_row, start_col -3, ' ');
      mvaddch(start_row + current_row, start_col + 40, ' ');
    }

    txt_ptr = options[current_row];
    txt_ptr++;
    mvprintw(start_row + current_row, start_col, "%s", txt_ptr);
    current_row++;
    option_ptr++;
  }

  mvprintw(start_row + current_row + 3, start_col, "Move highlight then press Enter.");

  refresh();
}

/* clear_all_screen은 화면을 지우고 제목을 다시 적는다. CD가 선택되면 해당 정보가 출력된다. */
void clear_all_screen()
{
  clear();
  mvprintw(2, Q_LINE, "%s", "CD Database Application");
  if(current_cd[0]) {
    mvprintw(ERROR_LINE, 0, "Current CD: %s: %s\n", current_cat, current_cd);
  }
  refresh();
}

/* 새로 CD레코드를 추가하는 함수 */
void add_record()
{
  char catalog_number[MAX_STRING];
  char cd_title[MAX_STRING];
  char cd_type[MAX_STRING];
  char cd_artist[MAX_STRING];
  char cd_entry[MAX_STRING];

  int screenrow = MESSAGE_LINE;
  int screencol = 10;

  clear_all_screen();
  mvprintw(screenrow, screencol, "Enter new CD details");
  screenrow += 2;

  mvprintw(screenrow, screencol, "Catalog Number: ");
  get_string(catalog_number);
  screenrow ++;

  mvprintw(screenrow, screencol, "    CD Title: ");
  get_string(cd_title);
  screenrow ++;

  mvprintw(screenrow, screencol, "    CD Type: ");
  get_string(cd_type);
  screenrow ++;

  mvprintw(screenrow, screencol, "    Artist: ");
  get_string(cd_artist);
  screenrow ++;

  mvprintw(15, 5, "About to add this new entry:");
  sprintf(cd_entry, "%s, %s, %s, %s", catalog_number, cd_title, cd_type, cd_artist);
  mvprintw(17, 5, "%s", cd_entry);
  refresh();

  move(PROMPT_LINE, 0);
  if(get_confirm()) {
    insert_title(cd_entry);
    strcpy(current_cd, cd_title);
    strcpy(current_cat, catalog_number);
  }
}

/* get_string 함수는 현재 화면 위치에서 문자열을 읽는다. */
void get_string(char *string)
{
  int len;

  wgetnstr(stdscr, string, MAX_STRING);
  len = strlen(string);
  if(len > 0 && string[len - 1] == '\n')
    string[len - 1] = '\0';
}

/* get_confirm 함수는 사용자의 의사를 확인한다. */
int get_confirm()
{
  int confirmed = 0;
  char first_char = 'N';

  mvprintw(Q_LINE, 5, "Are you sure? ");
  clrtoeol();
  refresh();

  cbreak();
  first_char = getch();
  if(first_char == 'Y' || first_char == 'y') {
    confirmed = 1;
  }
  nocbreak();

  if(!confirmed) {
    mvprintw(Q_LINE, 1, "    Cancelled");
    clrtoeol();
    refresh();
    sleep(1);
  }
  return confirmed;
}

/* insert_title 함수는 타이틀을 추가한다. 타이틀 문자열을 타이틀 파일의 마지막에 추가하는 방식을 사용. */
void insert_title(char *cdtitle)
{
  FILE *fp = fopen(title_file, "a");
  if(!fp) {
    mvprintw(ERROR_LINE, 0, "cannot open CD titles database");
  } else {
    fprintf(fp, "%s\n", cdtitle);
    fclose(fp);
  }
}

/* 화면 창을 표현하기 위한 전역 상수 몇개를 선언한다. */
#define BOXED_LINES   11
#define BOXED_ROWS    60
#define BOX_LINE_POS   8
#define BOX_ROW_POS    2

/* update_cd는 CD 트랙을 수정한다. */
void update_cd()
{
  FILE *tracks_fp;
  char track_name[MAX_STRING];
  int len;
  int track = 1;
  int screen_line = 1;
  WINDOW *box_window_ptr;
  WINDOW *sub_window_ptr;
  clear_all_screen();
  mvprintw(PROMPT_LINE, 0, "Re-entering tracks for CD. ");
  if(!get_confirm())
    return;
  move(PROMPT_LINE, 0);
  clrtoeol();
  remove_tracks();
  mvprintw(MESSAGE_LINE, 0, "Enter a blank line to finish");

  tracks_fp = fopen(tracks_file, "a");

  box_window_ptr = subwin(stdscr, BOXED_LINES + 2, BOXED_ROWS + 2, BOX_LINE_POS - 1, BOX_ROW_POS - 1);
  if(!box_window_ptr)
    return;
  box(box_window_ptr, ACS_VLINE, ACS_HLINE);

  sub_window_ptr = subwin(stdscr, BOXED_LINES, BOXED_ROWS, BOX_LINE_POS, BOX_ROW_POS);
  if(!sub_window_ptr)
    return;
  scrollok(sub_window_ptr, TRUE);
  werase(sub_window_ptr);
  touchwin(stdscr);

  do {
    mvwprintw(sub_window_ptr, screen_line++, BOX_ROW_POS + 2, "Track %d: ", track);
    clrtoeol();
    refresh();
    wgetnstr(sub_window_ptr, track_name, MAX_STRING);
    len = strlen(track_name);
    if(len > 0 && track_name[len - 1] == '\n')
      track_name[len - 1] = '\0';

    if(*track_name)
       fprintf(tracks_fp, "%s, %d, %s\n", current_cat, track, track_name);
    track++;
    if(screen_line > BOXED_LINES - 1) {
      /* time to start scrolling */
      scroll(sub_window_ptr);
      screen_line--;
    }
  } while(*track_name);
  delwin(sub_window_ptr);

  fclose(tracks_fp);
}

/* 다음은 remove_cd */
void remove_cd()
{
  FILE *titles_fp, *temp_fp;
  char entry[MAX_ENTRY];
  int cat_length;

  if(current_cd[0] == '\0')
    return;

  clear_all_screen();
  mvprintw(PROMPT_LINE, 0, "About to remove CD %s: %s. ", current_cat, current_cd);
  if(!get_confirm())
    return;

  cat_length = strlen(current_cat);

  /* 타이틀 파일을 임시파일로 복사한다 */
  titles_fp = fopen(title_file, "r");
  temp_fp = fopen(temp_file, "w");
  while(fgets(entry, MAX_ENTRY, titles_fp)) {
    /* 카타로그 넘버랑 복사된 엔트리랑 체크 */
    if(strncmp(current_cat, entry, cat_length) != 0)
      fputs(entry, temp_fp);
  }
  fclose(titles_fp);
  fclose(temp_fp);

  /* 타이틀 파일을 삭제하고, 임시파일을 타이틀파일로 덮어쓴다. */
  unlink(title_file);
  rename(temp_file, title_file);

  /* 트랙 파일도 마찬가지 작업을 한다. */
  remove_tracks();

  /* 현재 CD는 없는 CD라고 입력해둔다. */
  current_cd[0] = '\0';
}

/* remove_tracks는 트랙을 제거한다. update_cd랑 remove_cd에서 호출됨. */
void remove_tracks()
{
  FILE *tracks_fp, *temp_fp;
  char entry[MAX_ENTRY];
  int cat_length;

  if(current_cd[0] == '\0')
    return;

  cat_length = strlen(current_cat);

  tracks_fp = fopen(tracks_file, "r");
  temp_fp = fopen(temp_file, "w");

  while(fgets(entry, MAX_ENTRY, tracks_fp)) {
    /* 카타로그 넘버랑 복사된 엔트리랑 체크 */
    if(strncmp(current_cat, entry, cat_length) != 0)
      fputs(entry, temp_fp);
  }
  fclose(tracks_fp);
  fclose(temp_fp);

  unlink(tracks_file);
  rename(temp_file, tracks_file);
}

/* count_cds는 디비를 뒤져서 타이틀과 트랙의 갯수를 구한다. */
void count_cds()
{
  FILE *titles_fp, *tracks_fp;
  char entry[MAX_ENTRY];
  int titles = 0;
  int tracks = 0;

  titles_fp = fopen(title_file, "r");
  if(titles_fp) {
    while(fgets(entry, MAX_ENTRY, titles_fp))
      titles++;
    fclose(titles_fp);
  }
  tracks_fp = fopen(tracks_file, "r");
  if(tracks_fp) {
    while(fgets(entry, MAX_ENTRY, tracks_fp))
      tracks++;
    fclose(tracks_fp);
  }
  mvprintw(ERROR_LINE, 0, "Database contains %d titles, with a total of %d tracks.", titles, tracks);
  get_return();
}

/* 트랙 목록 검색해서 CD 타이틀을 찾을 수 있다 */
void find_cd()
{
  char match[MAX_STRING], entry[MAX_ENTRY];
  FILE *titles_fp;
  int count = 0;
  char *found, *title, *catalog;

  mvprintw(Q_LINE, 0, "Enter a string to search for in CD titles: ");
  get_string(match);
  titles_fp = fopen(title_file, "r");
  if(titles_fp) {
    while(fgets(entry, MAX_ENTRY, titles_fp)) {

      /* 이전 카다록 스킵 */
      catalog = entry;
      if(found = strstr(catalog, ",")) {
        *found = 0;
        title = found + 1;

        /* 콤마 지우기 */
        if(found = strstr(title, ",")) {
          *found = '\0';
          if(found = strstr(title, match)) {
            count++;
            strcpy(current_cd, title);
            strcpy(current_cat, catalog);
          }
        }
      }
    }
    fclose(titles_fp);
  }
  if(count != 1) {
    if(count == 0)
      mvprintw(ERROR_LINE, 0, "Sorry, no matching CD found. ");
    if(count > 1) {
      mvprintw(ERROR_LINE, 0, "Sorry, match is ambiguous: %d CDs found. ", count);
    }
    current_cd[0] = '\0';
    get_return();
  }
}

void list_tracks()
{
    FILE *tracks_fp;
    char entry[MAX_ENTRY];
    int cat_length;
    int lines_op = 0;
    WINDOW *track_pad_ptr;
    int tracks = 0;
    int key;
    int first_line = 0;

    if (current_cd[0] == '\0') {
      mvprintw(ERROR_LINE, 0, "You must select a CD first. ", stdout);
      get_return();
      return;
    }

    clear_all_screen();
    cat_length = strlen(current_cat);

    /* 현재 CD의 카운트 */
    tracks_fp = fopen(tracks_file, "r");
    if(!tracks_fp)
      return;
    while(fgets(entry, MAX_ENTRY, tracks_fp)) {
      if(strncmp(current_cat, entry, cat_length) == 0)
        tracks++;
    }
    fclose(tracks_fp);

    /* 새 패드를 만든다. */
    track_pad_ptr = newpad(tracks + 1 + BOXED_LINES, BOXED_ROWS + 1);
    if(!track_pad_ptr)
      return;
    tracks_fp = fopen(tracks_file, "r");
    if(!tracks_fp)
      return;

    mvprintw(4, 0, "CD Track Listening\n");

    /* 트랙 정보를 패드에 작성 */
    while(fgets(entry, MAX_ENTRY, tracks_fp)) {
      /* 카다록 넘버랑 나머지를 출력 */
      if(strncmp(current_cat, entry, cat_length) == 0) {
        mvwprintw(track_pad_ptr, lines_op++, 0, "%s", entry + cat_length + 1);
      }
    }
    fclose(tracks_fp);

    if(lines_op > BOXED_LINES) {
      mvprintw(MESSAGE_LINE, 0, "Cursor keys to scroll, Enter or q to exit");
    } else {
      mvprintw(MESSAGE_LINE, 0, "Enter or q to exit");
    }
    wrefresh(stdscr);
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    key = 0;
    while(key != 'q' && key != KEY_ENTER && key != '\n') {
      if(key == KEY_UP) {
        if(first_line > 0)
          first_line--;
      }
      if(key == KEY_DOWN) {
        if(first_line + BOXED_LINES + 1 < tracks)
          first_line++;
      }
      /* 이제 패드에서 적절한 부분을 떼서 그린다. */
      prefresh(track_pad_ptr, first_line, 0, BOX_LINE_POS, BOX_ROW_POS, BOX_LINE_POS + BOXED_LINES, BOX_ROW_POS + BOXED_ROWS);
      /* wrefresh(stdscr); */
      key = getch();
    }
    delwin(track_pad_ptr);
    keypad(stdscr, FALSE);
    nocbreak();
    echo();
  }

/* get_return 함수는 리턴문자가 입력될때까지 프롬프트를 출력 */
void get_return()
{
  int ch;

  mvprintw(23, 0, "%s", " Press Enter ");
  refresh();
  while((ch = getchar()) != '\n' && ch != EOF);
}
