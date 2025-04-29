#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

int monitor_pid=-1;
int pipe_fd[2]; // [0]-citire, [1]-scrie

void send_command(const char* cmd) {
    if (monitor_pid==-1) {
        printf("Monitorul nu este pornit!\n");
        return;
    }

    write(pipe_fd[1],cmd,strlen(cmd));
    kill(monitor_pid,SIGUSR1);
}

void start_monitor() {
    if (monitor_pid!=-1) {
        printf("Monitorul este deja pornit (PID: %d)\n",monitor_pid);
        return;
    }

    if (pipe(pipe_fd)<0) {
        perror("pipe");
        exit(1);
    }

    pid_t pid=fork();
    if (pid<0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        //procesul fiu ->monitor
        close(pipe_fd[1]); // inchidem capatul de scriere

        char fd_str[10];
        snprintf(fd_str,sizeof(fd_str),"%d",pipe_fd[0]);

        execl("./treasure_manager", "treasure_manager", "--monitor",fd_str,NULL);
        
        // daca exec nu merge
        perror("execl");
        exit(1);
    } else {
        // procesul parinte
        close(pipe_fd[0]); //Inchidem capatul de citire
        monitor_pid=pid;
        printf("Monitorul a fost pornit (PID: %d)\n",monitor_pid);
    }
}


void stop_monitor() {
    if (monitor_pid==-1) {
        printf("Monitorul nu este pornit!\n");
        return;
    }

    kill(monitor_pid,SIGUSR2);
    int status;
    waitpid(monitor_pid,&status, 0);
    printf("Monitorul s-a oprit cu codul de iesire %d\n",WEXITSTATUS(status));
    monitor_pid = -1;
}

int main() {
    char cmd[256];

    while (1) {
        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        cmd[strcspn(cmd,"\n")] = 0;

        if (strcmp(cmd, "start_monitor") == 0) {
            start_monitor();
        } else if (strncmp(cmd, "list_hunts", 10) == 0) {
            send_command(cmd); 
        } else if (strncmp(cmd, "list_treasures ", 15) == 0) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "list %s", cmd + 15);  // trimite: list <hunt_id>
            send_command(buffer);
        } else if (strncmp(cmd, "view_treasure ", 14) == 0) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "view %s", cmd + 14);  // trimite: view <hunt_id> <id>
            send_command(buffer);
        } else if (strcmp(cmd, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(cmd, "exit") == 0) {
            if (monitor_pid != -1) {
                printf("Eroare: Monitorul ruleaza inca!\n");
            } else {
                break;
            }
        } else {
            printf("Comanda necunoscuta.\n");
        }
        
    }

    return 0;
}
