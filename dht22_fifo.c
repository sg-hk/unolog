#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <signal.h>

#define SERIAL_PORT "/dev/ttyUSB0" 
#define BAUD_RATE B9600  // must match Serial.begin(9600)
#define PIPE_PATH "/tmp/bar/fifo_dht22"

volatile sig_atomic_t stop_flag = 0;
void handle_signal(int signal) 
{
        (void)signal; // suppress compiler warning
        stop_flag = 1;
}

int main() {
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
        struct termios tty;
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

        /* open/create fifo */
        struct stat st;
        if (stat(PIPE_PATH, &st) == -1) {
                if (mkfifo(PIPE_PATH, 0666) == -1) {
                        perror("mkfifo");
                        exit(1);
                }
        }
        int fifo_fd = open(PIPE_PATH, O_WRONLY);
        if (fifo_fd == -1) {
                perror("open fifo");
                exit(1);
        }

        while (!stop_flag) {
                fd_set read_fds;
                FD_ZERO(&read_fds);
                FD_SET(serial_fd, &read_fds);

                struct timeval timeout;
                /* UNO has 60s delay; 10s for polling is fine */
                timeout.tv_sec = 10; 
                timeout.tv_usec = 0;

                int ret = select(serial_fd + 1, &read_fds, NULL, NULL, &timeout);
                if (ret == -1) {
                        perror("select");
                        exit(1);
                }
                if (ret == 0) // no data yet
                        continue;

                char buffer[32];
                ssize_t bytes_read = read(serial_fd, buffer, sizeof(buffer) -1);
                if (bytes_read < 0) {
                        perror("serial read");
                        exit(1);
                }
                if (bytes_read == 0)
                        continue;

                long strend = strcspn(buffer, "\n");
                buffer[strend] = '\0';
                dprintf(fifo_fd, "%s\n", buffer);
        }

        close(serial_fd);
        close (fifo_fd);
        return 0;
}
