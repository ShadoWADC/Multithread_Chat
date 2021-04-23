#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * gcc -o loginsystem loginsystem.c
 * */

struct login                           
{
    char username[30];
    char password[20];
};

void login (void);
void registration (void);

int main (void)
{
    int option;
	do{
		printf("Chat Service Menu:");
		printf("\n\t(1)Register\n\t(2)Login\n\t(0)Exit\n\n");
		printf("Select your option: ");
		scanf("%d",&option);
		while( getchar() != '\n' ){ /* flush to end of input line */ }
		
		if(option == 1)
		{
			system("clear");
			registration();
		}

		else if(option == 2)
			{
				system("clear");
				login();
			}
	}while (option != 0);
}
    

void login (void)
{
    char username[30],password[20];

    FILE *log;

    log = fopen("login.txt","r");
    if (log == NULL)
    {
        fputs("Error at opening File!", stderr);
        exit(1);
    }

    struct login l;

    printf("\nPlease Enter your login credentials below\n\n");
    printf("Username:  ");
    scanf("%s",username);
    while( getchar() != '\n' ){ /* flush to end of input line */ }
    printf("\nPassword: ");
    scanf("%s",password);
    while( getchar() != '\n' ){ /* flush to end of input line */ }
	
    while(fread(&l,sizeof(l),1,log))
        {
        if(strcmp(username,l.username)==0 && strcmp(password,l.password)==0)
            {   
                printf("\nLogin Successful!\n");
            }
        else 
            {
                printf("\nIncorrect credentials!\nPlease enter the correct credentials\n");
            }
        }

    fclose(log);
    return;
}


void registration(void)
{
    FILE *log;

    log=fopen("login.txt","w");
    if (log == NULL)
    {
        fputs("Error at opening File!", stderr);
        exit(1);
    }

    struct login l;

    printf("\nWelcome to the chat service!\nPlease proceed with the registration of your account.\n\n");

    printf("\nEnter Username:\n");
    scanf("%s",l.username);
    while( getchar() != '\n' ){ /* flush to end of input line */ }
    printf("\nEnter Password:\n");
    scanf("%s",l.password);
    while( getchar() != '\n' ){ /* flush to end of input line */ }


    fwrite(&l,sizeof(l),1,log);
    fclose(log);
	
	printf("\nRegistration Successful!\n");
    printf("\nWelcome, %s!\n\n",l.username);

    printf("Press any key to continue...");
    getchar();
    system("clear");
}
