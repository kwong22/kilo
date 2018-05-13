#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// store original terminal attributes
struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  // read current terminal attributes into struct
  tcgetattr(STDIN_FILENO, &orig_termios);

  // restore original terminal attributes upon program exit
  atexit(disableRawMode);

  // create copy of terminal attributes to modify
  struct termios raw = orig_termios;

  // c_lflag is local flags, ECHO is bitflag
  // turn off ECHO feature to stop printing what you type to terminal
  // turn off canonical mode (ICANON) to read input byte-by-byte
  // bitwise-NOT then bitwise-AND to flip bits to 0
  raw.c_lflag &= ~(ECHO | ICANON);

  // write new terminal attributes
  // TCSAFLUSH waits for pending output to be written and discards unread input
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();
  
  char c;
  // read until Ctrl-D or q is entered
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
  return 0;
}
