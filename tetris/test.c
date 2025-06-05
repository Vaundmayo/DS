#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#define Left 106, 74

// Implementation of getch() for Unix-like systems

int main (void) {
    int ch;

    printf("Press any key to continue...\n");
    ch = getch();  // Wait for a key press
    printf("You pressed: %c\n", ch);
    printf("%d", ch);

    if (ch == Left) {
        printf("Exiting the program.\n");
        return 0;  // Exit the program if 'q' or 'Q' is pressed
    }

    printf("You can continue with the program.\n");

    return 0;  // Normal exit
}