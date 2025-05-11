#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char **argv)
{
    if(argc!=2)
    {
        perror("Argumente invalide");
        exit(1);
    }
    
    execl("./treasure_manager","treasure_manager","--scor",argv[1],NULL);

    perror("execl");
    exit(1);

    return 0;
}