#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>

#define SERIAL_PORT "/dev/ttyUSB0" 
#define BAUD_RATE B9600  // must match Serial.begin(9600)
#define LOGDIR "/.local/share/unolog/"

volatile sig_atomic_t stop_flag = 0;
void handle_signal(int signal) 
{
        (void)signal; // suppress compiler warning
        stop_flag = 1;
}

int main() {
        struct termios tty;
        FILE *log_file = NULL;
        char logpath[128];
        int last_logged_day = -1;
        char buffer[32];

        /* set up signal handlers for SIGINT (Ctrl+C) and SIGTERM (kill) */
        struct sigaction sa;
        sa.sa_handler = handle_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        /* open serial port */
        int serial_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
        if (serial_fd == -1) {
                perror("serial open");
                exit(1);
        }
        if (tcgetattr(serial_fd, &tty) != 0) {
                perror("serial attr get");
                close(serial_fd);
                exit(1);
        }

        /* configure terminal attributes */
        cfsetospeed(&tty, BAUD_RATE);
        cfsetispeed(&tty, BAUD_RATE);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
        tty.c_iflag &= ~IGNBRK; // process break signal
        if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
                perror("serial attr set");
                close(serial_fd);
                exit(1);
        }

        while (!stop_flag) {
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                int current_day = t->tm_mday;

                /* check if we need a new log file */
                if (current_day != last_logged_day) {
                        if (log_file) fclose(log_file);
                        snprintf(logpath, sizeof(logpath), 
                                        "%s%s%04d-%02d-%02d.log",
                                        getenv("HOME"), LOGDIR,
                                        t->tm_year+1900, t->tm_mon+1, current_day
                                );
                        log_file = fopen(logpath, "a+");
                        if (!log_file) {
                                perror("log");
                                break;
                        }
                        last_logged_day = current_day;
                }

                int n = read(serial_fd, buffer, sizeof(buffer)-1);
                if (n > 0) {
                        buffer[n] = '\0';
                        fprintf(log_file, "%s", buffer);
                        fflush(log_file);
                } else if (n < 0) {
                        perror("serial read");
                        break;
                } /* n == 0 is simply end of stream */
        }

        if (log_file) fclose(log_file);
        close(serial_fd);
        return 0;
}
