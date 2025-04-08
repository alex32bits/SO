#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <time.h>
typedef struct {
    int id;
    struct {
        float lat;
        float lon;
    } coordonate;
    char clue[100];
    int value;
} treasure;

//salvam intr-un fisier din direcotr actiunile
void log_action(const char *hunt_id, const char *action) {
    char log_filename[256];
    snprintf(log_filename, sizeof(log_filename), "%s/logged_hunt", hunt_id);
    
    // deschidem fisierul de log
    int log_file = open(log_filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (log_file == -1) {
        perror("Eroare la deschiderea fisierului de log");
        return;
    }

    write(log_file, action, strlen(action)); //scriem actiunea

    write(log_file, "\n", 1);

    close(log_file);//inchidem fisierul

    // facem o legatura simbolica cu fisierul de mai jos
    char link_name[256];
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);  // <ID> este acelasi cu hunt_id
    
    // cream linkul simbolic
    if (symlink(log_filename, link_name) == -1) {
        return;
    }

    printf("Link simbolic creat: %s -> %s\n", link_name, log_filename);
}

// functia pentru a adauga o comoara
void add(const char *hunt_id) {
    DIR *dir = opendir(".");

    // cautam un director cu numele hunt_id
    int hunt_exists = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, hunt_id) == 0) {
            hunt_exists = 1;
            break;
        }
    }
    closedir(dir);

    // daca directorul nu exista, îl creăm
    if (hunt_exists==0) {
        if (mkdir(hunt_id,0755) == -1) {
            perror("Eroare la crearea directorului");
            return;
        }
        printf("Directorul %s a fost creat cu succes.\n", hunt_id);
    } else {
        printf("Directorul %s există deja.\n", hunt_id);
    }

    // cautam sau cream fisierul de comori
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/comoara.txt", hunt_id);

    int in = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (in == -1) {
        perror("Eroare la deschiderea fisierului de comori");
        return;
    }

    treasure a;
    printf("Introduceti ID-ul comorii: ");
    scanf("%d", &a.id);

    printf("Introduceti latitudine: ");
    scanf("%f", &a.coordonate.lat);

    printf("Introduceti longitudine: ");
    scanf("%f", &a.coordonate.lon);

    printf("Introduceti valoarea comorii: ");
    scanf("%d", &a.value);

    printf("Introduceti indiciul: ");
    scanf("%s", a.clue);


    // scriem comoara în fisier
    if (write(in, &a, sizeof(a)) == -1) {
        perror("Eroare la scrierea comorii");
        close(in);
        return;
    }

    close(in);

    // memoram operațiunea
    char action[256];
    snprintf(action, sizeof(action), "Treasure added: ID=%d, Value=%d", a.id, a.value);
    log_action(hunt_id, action);
}



void list(const char *hunt_id) {
    // Cautam directorul vanatorii daca ca parametru
    DIR *dir = opendir(hunt_id);
    if (dir == NULL) {
        perror("Nu s-a putut deschide directorul vanatorii");
        return;
    }

    // afisam numele vanatorii
    printf("Vanatoare: %s\n", hunt_id);

    // cautam fisierul de comori
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/comoara.txt", hunt_id);

    // folosim stat pentru a obtine dimensiunea si timpul ultimei modificari
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        closedir(dir);
        return;
    }

    // afisam dimensiunea fisierului
    printf("Dimensiunea totală a fișierului de comori: %ld bytes\n", st.st_size);

    // afisam timpul ultimei modificari
    printf("Ultima modificare: %s", ctime(&st.st_mtime)); 


    // deschidem fisierul de comori
    int in = open(filename, O_RDONLY);
    if (in == -1) {
        perror("Eroare la deschiderea fișierului de comori");
        closedir(dir);
        return;
    }

    treasure a;
    ssize_t bytesRead;//read are aceasta semnatura cu ssize_t

    // citim si afisamm fiecare comoara
    printf("Comori\n");
    while ((bytesRead = read(in, &a, sizeof(a))) > 0) {
        if (bytesRead < sizeof(a)) {
            fprintf(stderr, "Citire incompleta a datelor pentru comoara\n");
            break;
        }

        printf("ID: %d\n", a.id);
        printf("Latitudine: %.2f\n", a.coordonate.lat);
        printf("Longitudine: %.2f\n", a.coordonate.lon);
        printf("Indiciu: %s\n", a.clue);
        printf("Valoare: %d\n", a.value);
        printf("----------------------\n");
    }

    close(in);
    closedir(dir);
}


void view(const char *hunt_id,int id) {
    // cautam directorul vanatorii
    DIR *dir = opendir(hunt_id);
    if (dir == NULL) {
        perror("Nu s-a putut deschide directorul vânătorii");
        return;
    }

    // cautam fisierul de comori
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/comoara.txt", hunt_id);

    // citim comorile din fisierul "comoara.txt"
    int in = open(filename, O_RDONLY);
    if (in == -1) {
        perror("Eroare la deschiderea fisierului de comori");
        closedir(dir);
        return;
    }

    treasure a;
    ssize_t bytesRead;
    int ok=0;
    // citim si afisam comoara dorita
    while ((bytesRead = read(in, &a, sizeof(a))) > 0) {
        if (bytesRead < sizeof(a)) {
            fprintf(stderr, "Citire incompletă a datelor pentru comoara\n");
            break;
        }
    if(a.id==id)
    {
        printf("ID: %d\n", a.id);
        printf("Latitudine: %.2f\n", a.coordonate.lat);
        printf("Longitudine: %.2f\n", a.coordonate.lon);
        printf("Indiciu: %s\n", a.clue);
        printf("Valoare: %d\n", a.value);
        printf("----------------------\n");
        ok=1;
    }
    }
    if(ok==0)
        printf("nu s-a gasit comoara cu id-ul %d\n",id);

    close(in);
    closedir(dir);
}

void remove_treasure(const char *hunt_id, int id) {
    // construim calea pentru fisierul de comori
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/comoara.txt", hunt_id);

    // deschidem fisierul de comori pentru citire
    int in = open(filename, O_RDONLY);
    if (in == -1) {
        perror("Eroare la deschiderea fisierului de comori");
        return;
    }

    // cream un fisier temporar pentru a scrie comorile care nu trebuie sterse
    int temp_fis = open("temp.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fis == -1) {
        perror("Eroare la deschiderea fisierului temporar");
        close(in);
        return;
    }

    treasure a;
    ssize_t bytesRead;
    int found = 0;

    // citim fisierul si scriem in fisierul temporar doar comorile care nu trebuie sterse
    while ((bytesRead = read(in, &a, sizeof(a))) > 0) {
        if (bytesRead < sizeof(a)) {
            fprintf(stderr, "Citire incompleta a datelor pentru comoara\n");
            break;
        }

        // scriem in temp tot ce nu trebuie sters
        if (a.id != id) {
            if (write(temp_fis, &a, sizeof(a)) == -1) {
                perror("Eroare la scrierea în fisierul temporar");
                close(in);
                close(temp_fis);
                return;
            }
        } 
        else 
        {
            found = 1;  // am gasit comoara de sters
        }
    }

    close(in);
    close(temp_fis);

    // daca am gasit comoara de sters,inlocuim fisierul cu comori cu actualul
    if (found) {
        if (rename("temp.dat", filename) == -1) {
            perror("Eroare la inlocuirea fisierului de comori");
            return;
        }
        printf("Comoara cu ID-ul %d a fost stearsa.\n", id);

        // adauga operatiunea
        char action[256];
        snprintf(action, sizeof(action), "Treasure removed: ID=%d", id);
        log_action(hunt_id, action);
    } else {
        // daca nu am gasit comoara, stergem fisierul temporar
        printf("Nu s-a gasit comoara cu ID-ul %d.\n", id);
        remove("temp.dat");
    }
}

void remove_files_in_directory(const char *dir_name) {
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("Nu s-a putut deschide directorul");
        return;
    }

    struct dirent *entry;
    char path[512];

    // parcurgem toate fisierele din director
    while ((entry = readdir(dir)) != NULL) {
      
        // Construim calea completa catre fisier/subdirector
        snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
        remove(path);
        
    }

    closedir(dir);
}

void remove_hunt(const char *hunt_id) {
    // cautam daca exista directorul vanatorii
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Nu s-a putut deschide directorul curent");
        return;
    }

    struct dirent *entry;
    int hunt_exists = 0;

    // parcurgem directoarele din directorul curent
    //pana il gasim pe cel de care avem nevoie
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, hunt_id) == 0) {
            hunt_exists = 1;
            break;
        }
    }

    // inchidem directorul
    closedir(dir);

    // daca directorul exista, il stergem
    if (hunt_exists) {
        remove_files_in_directory(hunt_id);
        if (rmdir(hunt_id) == -1) {
            perror("Eroare la stergerea directorului");
        } else {
            printf("Directorul %s a fost sters cu succes.\n", hunt_id);
        }
    } else {
        printf("Directorul %s nu exista.\n", hunt_id);
    }
}

int main(int argc, char **argv) {
   
    if(argc<3) 
    {   
        printf("argumente invalide\n");
        exit(-1);
    }
    
    if (strcmp(argv[1], "--add") == 0) 
    {
        add(argv[2]);
    } 
    else if (strcmp(argv[1], "--list") == 0) 
    {
        list(argv[2]);
        // Afisez continutul fisierului din directorul cu numele care e dupa list
    } 
    else if (strcmp(argv[1], "--view") == 0) 
    {
        // Caut in fisierul cu id=argv[2] din directorul argv[2] si afisez detaliile
        view(argv[2],atoi(argv[3]));
    } 
    else if (strcmp(argv[1], "--remove_treasure") == 0) 
    {
        // Caut in fisierul cu id=argv[2] din directorul argv[2] si sterg datele acestei comori
        remove_treasure(argv[2],atoi(argv[3]));
    } 
    else if (strcmp(argv[1], "--remove_hunt") == 0) 
    {
        remove_hunt(argv[2]);
        // Caut in fisierul cu id=argv[2] din directorul argv[2] si sterg o vanatoare
    } else {
        fprintf(stderr, "Comanda necunoscuta: %s\n", argv[1]);
    }
    
    return 0;
}

