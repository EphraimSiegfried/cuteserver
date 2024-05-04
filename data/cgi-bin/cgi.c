#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    time_t current_time;

    printf("Content-type: text/html\n\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>Improved Simple Sample</title>\n");
    printf("</head> <body>\n");
    printf("<h1>This is a Heading</h1>\n");
    printf("<p> <pre>\n");
    printf("C Program Version\n");
    current_time = time(NULL); /* get current time */
    printf("It is now %s\n", ctime(&current_time));
    printf("You are signed onto %s\n", getenv("REMOTE_HOST"));
    printf("Your IP Address is %s\n", getenv("REMOTE_ADDR"));
    printf("</pre> </body> </html>\n");
    fflush(stdout); /* force physical write */
    exit(0);
}
