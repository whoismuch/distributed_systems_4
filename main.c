#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include "pa2345.h"
#include "common.h"
#include "msg_handler.h"

FILE *p_log;
FILE *e_log;
int process_number;
int pipes_to_read[12][12];
int pipes_to_write[12][12];


Message* prepare_msg(void *payload, uint16_t payload_len, int16_t type);

void handle_msg(Message* msg);

int close_not_my_pipes(local_id pid) {
    for (int i = 0; i <= process_number; i++) {
        for (int j = 0; j <= process_number; j++) {
            if (i != j) {
                if (i != pid) {
                    if (close(pipes_to_write[i][j]) != 0) return -1;
                }
                if (j != pid) {
                    if (close(pipes_to_read[j][i]) != 0) return -1;
                }
            }
        }
    }

    return 0;
}

void initPipes() {
    for (int i = 0; i <= process_number; i++) {
        for (int j = 0; j <= process_number; j++) {
            int fdPipes[2];
            if (i != j) {
                pipe(fdPipes);
                fcntl(fdPipes[0], F_SETFL, O_NONBLOCK);
                fcntl(fdPipes[1], F_SETFL, O_NONBLOCK);
                pipes_to_read[j][i] = fdPipes[0];
                pipes_to_write[i][j] = fdPipes[1];
            }
        }
    }
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            if (i == j) {
                pipes_to_read[j][i] = -999;
                pipes_to_write[i][j] = -999;
            } else if (i > process_number) {
                pipes_to_read[j][i] = -1;
                pipes_to_read[i][j] = -1;
                pipes_to_write[j][i] = -1;
                pipes_to_write[i][j] = -1;
            }
        }

    }
}

void sendSynchronize(local_id i, const char* log, MessageType type) {

    printf(log, get_lamport_time(), i, getpid(), getppid(), 0);
    fprintf(e_log, log, get_lamport_time(), i, getpid(), getppid(), 0);

    char *message_content = (char *) malloc(255 * sizeof(char));
    sprintf(message_content, log, get_lamport_time(), i, getpid(), getppid(), 0);

    send_multicast(pipes_to_write[i], prepare_msg(message_content, strlen(message_content), type));

    printf("child %1d has sent %u in all channels descriptors\n", i, type);
    fprintf(p_log, "child %1d has sent %u in all channels descriptors\n", i, type);

}

void receiveSynchronize(local_id i, const char* log) {

    local_id j = 0;
    int res;
    Message message;
    memset(&message, 0, sizeof(Message));
    while (pipes_to_read[i][j] != -1) {
        if (pipes_to_read[i][j]  != -999 && j != 0) {
            res = receive(pipes_to_read[i] , j, &message);
            if (res == 1) {
                j--;
            }
            else if (res == -2) {

            }
            else {
                handle_msg(&message);
            }

        }
        j++;
    }

    printf(log, get_lamport_time(), i);
    fprintf(e_log, log, get_lamport_time(), i);
}

void initChildProcesses() {
    for (int i = 1; i <= process_number; i++) // loop will run n times (n=5)
    {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            close_not_my_pipes(i);
            char *message_content = (char *) malloc(255 * sizeof(char));
            sprintf(message_content, log_started_fmt, get_lamport_time(), i, getpid(), getppid(), 0);
            sendSynchronize(i, message_content, STARTED);
            receiveSynchronize(i, log_received_all_started_fmt);
            //полезная работа
            //конец полезной работы
            sprintf(message_content, log_started_fmt, get_lamport_time(), i, getpid(), getppid(), 0);
            sendSynchronize(i, message_content, DONE);
            receiveSynchronize(i, log_received_all_done_fmt);
            exit(0);
        }
    }
}

void waitForChildrenTerminating() {
    for (int i = 1; i <= process_number; i++) // loop will run n times (n=5)
        wait(NULL);
}

void parent_routine() {
    initChildProcesses();
    close_not_my_pipes(0);
    receiveSynchronize(0, log_received_all_started_fmt);
    receiveSynchronize(0, log_received_all_done_fmt);
    waitForChildrenTerminating();
}

void start() {
    initPipes();
    parent_routine();
}

int main(int argc, char *argv[]) {
    if (0 != strcmp(argv[1], "-p")) {
        return 1;
    }
    process_number = atoi(argv[2]);

    p_log = fopen(pipes_log, "wa+");
    if (p_log == NULL) { return 1; }

    e_log = fopen(events_log, "wa+");
    if (e_log == NULL) { return 1; }


    start();
}