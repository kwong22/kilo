/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

// store original terminal attributes
struct termios orig_termios;

/*** terminal ***/

// handles error stored in global errno variable
void die(const char *s) {
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

/*** init ***/

int main() {
  enableRawMode();
  
  // read until q is entered
  while (1) {
    char c = '\0';

    // when read() times out in Cygwin, it returns -1 and errno of EAGAIN
    // don't treat EAGAIN as an error
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read");

    if (iscntrl(c)) { // checks if c is a control character, nonprintable
      printf("%d\r\n", c); // %d to format byte as a decimal number
    } else {
      printf("%d ('%c')\r\n", c, c); // %c to write out byte as a character
    }

    if (c == 'q') break;
  }
  return 0;
}
