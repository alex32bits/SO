#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

int monitor_pid=-1;
const char* command_file="comenzi.txt";  // Fisier pentru a scrie comenzi

//Functie pentru a scrie comenzi in fisizer
void send_command(const char* cmd) {
    if (monitor_pid==-1) {
        printf("Monitorul nu este pornit!\n");
        return;
    }

    FILE* file=fopen(command_file,"w");
    if (file==NULL) {
        perror("Eroare la deschiderea fisierului de comenzi");
        return;
    }

    //Scriem comanda in fis
    fprintf(file,"%s\n",cmd);
    fclose(file);

    //Trimitem semanul SIGUSR1 deoarece avem o functie de apelat din treasure_manager
    kill(monitor_pid,SIGUSR1);
}


void start_monitor() {
    if (monitor_pid!=-1) {
        printf("Monitorul este deja pornit (PID: %d)\n",monitor_pid);
        return;
    }

    pid_t pid=fork();
    if (pid<0) {
        perror("fork");
        exit(1);
    }

    if (pid==0) {
        //Procesul copil este monitorul
        execl("./treasure_manager", "./treasure_manager", "--monitor",command_file,NULL);
        //Rulam funcia --monitor cu argumentul fisierul de comanda
        
        //In caz de eroare la execl
        perror("execl");
        exit(1);
    } else {
        //Procesul parinte pune pid-ul monitorului in monitor_pid 
        //pentru a stii cui ii trimitem semnalele
        monitor_pid=pid;
        printf("Monitorul a fost pornit (PID: %d)\n",monitor_pid);
    }
}

//Inchidem monitorul
void stop_monitor() {
    if (monitor_pid==-1) {
        printf("Monitorul nu este pornit!\n");
        return;
    }

    kill(monitor_pid,SIGUSR2);  //Folosim SIGUSR2 pentru a inchide monitorul
    int status;
    waitpid(monitor_pid,&status,0);  //Asteptam sa se termine procesul monitor
    printf("Monitorul s-a oprit cu codul de iesire %d\n",WEXITSTATUS(status));
    monitor_pid=-1;
}

void calculate()
{
    DIR *dir=opendir(".");
    if (dir==NULL) {
        printf("Directorul curent nu exista\n");
        return;
    }

    struct dirent *entry;

    // Parcurgem toate fis din directorul curent
    while ((entry=readdir(dir))!= NULL) {
        // Verificam daca numele directorului incepe cu "hunt"  asa am eu numele vanatorilor
        if (strncmp(entry->d_name,"hunt",4)==0) {
            int pfd[2];
            if(pipe(pfd)==-1)
            {
                perror("Eroare la crearea pipe-ului\n");
	            exit(1);
            }

            int pid=fork();
            if(pid==-1)
            {
                perror("fork");
                exit(1);
            }

            if(pid==0)//procesul fiu
            {
                close(pfd[0]);//Inchid capatul de citire
                dup2(pfd[1],1);
                execl("./score_calculate","score_calculate",entry->d_name,NULL);
                perror("execl");
                exit(1);
            }
            //procesul parinte
            close(pfd[1]);//Inchid capatul de scriere
            char buffer[256];
            ssize_t bytesRead;

            printf("Scoruri pentru %s:\n",entry->d_name);
            while ((bytesRead=read(pfd[0], buffer, sizeof(buffer)-1))>0) {
                buffer[bytesRead]='\0';
                printf("%s",buffer);
            }

            close(pfd[0]);
            waitpid(pid, NULL, 0); // Asteapta copilul
        }
    }
    closedir(dir);
}

int main() {

    char cmd[256];
    printf("Introdu comenzi: \nstart_monitor\nlist_hunts\nlist_treasures huntID\nview_treasure huntID ID\nstop_monitor\nexit\ncalcuate_score\n");
    while (1) {
        if (!fgets(cmd,sizeof(cmd),stdin)) break;
        cmd[strcspn(cmd,"\n")]=0;  //Stergem caracterul \n 

        if(strcmp(cmd,"calculate_score")==0)
        {
            calculate();
        }else if (strcmp(cmd,"start_monitor")==0) {
            start_monitor();
        } else if (strncmp(cmd,"list_hunts",10)==0) {
            send_command(cmd);
        } else if (strncmp(cmd,"list_treasures ",15)==0) {
            char buffer[256];
            snprintf(buffer,sizeof(buffer),"list %s",cmd+15);  // Send: list <hunt_id>
            send_command(buffer);
        } else if (strncmp(cmd,"view_treasure ",14)==0) {
            char buffer[256];
            snprintf(buffer,sizeof(buffer),"view %s",cmd+14);  // Send: view <hunt_id> <id>
            send_command(buffer);
        } else if (strcmp(cmd,"stop_monitor")==0) {
            stop_monitor();
        } else if (strcmp(cmd,"exit")==0) {
            if (monitor_pid!=-1) {
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
