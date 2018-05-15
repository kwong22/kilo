/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

// macro that bitwise-ANDs a character with 00011111 (sets upper 3 bits to 0)
// equivalent to Ctrl-character
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

// store original terminal attributes
struct termios orig_termios;

/*** terminal ***/

// handles error stored in global errno variable
void die(const char *s) {
  // clear screen and position cursor
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  // prints s and descriptive message for errno
  perror(s);

  // exits program with exit status 1 (failure)
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  // read current terminal attributes into struct
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");

  // restore original terminal attributes upon program exit
  atexit(disableRawMode);

  // create copy of terminal attributes to modify
  struct termios raw = orig_termios;

  // c_iflag is input flags
  // turn off ICRNL to read Ctrl-M as carriage return (13)
  // turn off IXON to disable Ctrl-S and Ctrl-Q data transmission control
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  // c_oflag is output flags
  // turn off OPOST to turn off all output processing (no more "\n" to "\r\n")
  // now have to write the full "\r\n" to start a new line
  raw.c_oflag &= ~(OPOST);

  raw.c_cflag |= (CS8);

  // c_lflag is local flags
  // turn off ECHO feature to stop printing what you type to terminal
  // turn off canonical mode (ICANON) to read input byte-by-byte
  // turn off IEXTEN to disable Ctrl-V
  // turn off ISIG to disable Ctrl-C (SIGINT) and Ctrl-Z (SIGTSTP) signals
  // bitwise-NOT then bitwise-AND to flip bits to 0
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  // VMIN is minimum number of bytes of input needed before read() can return,
  // so now read() returns as soon as there is any input to read
  raw.c_cc[VMIN] = 0;

  // VTIME is maximum amount of time to wait before read() returns, in tenths of
  // a second
  raw.c_cc[VTIME] = 1;

  // write new terminal attributes
  // TCSAFLUSH waits for pending output to be written and discards unread input
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

// waits for keypress and returns it
// in terminal section because it deals with low-level terminal input
char editorReadKey() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    // when read() times out in Cygwin, it returns -1 and errno of EAGAIN
    // don't treat EAGAIN as an error
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }

  return c;
}

/*** input ***/

// asks for keypress and handles it
// in input section because it maps keys to functions at a higher level
void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
    // Ctrl-Q exits the program and clears screen
    case CTRL_KEY('q'):
      // clear screen and position cursor
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;
  }
}

/*** output ***/

// draws each row of the text buffer
void editorDrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen() {
  // write 4 bytes to the terminal (using VT100 escape sequences):
  // \x1b - escape character (27)
  //    [ - after escape character to begin escape sequence
  //    2 - clear entire screen, 1 up to cursor, 0 after cursor
  //    J - clear screen
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // use H command to position cursor
  // add x;y argument before H for x row and y column, indexing starts with 1
  // 1;1 is default
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** init ***/

int main() {
  enableRawMode();
  
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
