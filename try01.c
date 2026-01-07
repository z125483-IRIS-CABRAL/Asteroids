#include <stdio.h>


// WELCOME TO THE PROJECT
// SYSTEM MANAGEMENT

// declarar as funções

void writeWelcomeMessage();
void writeOnFile(char[] text);

int main()
{

    /**
     * WELCOME MESSAGE
     */

     printf("WELCOME TO THE NASA\nSUPER SECRET SYSTEM\n");
     // desenhar um foguete. se der colocar alguma animação também


    /*
     * OPENING A FILE 
     */
    FILE *fp;
    char ch;
    fp = fopen("test.txt", "r");
    if(fp == NULL){
        printf("Error opening a file");
    }
    else{
        while(ch!=EOF){
            ch = getc(fp);
            putchar(ch);
        }
        fclose(fp); 
    }


    /**
     * WRITING IN A FILE 
     * */ 
    char ch;
    char sentence[1000];
    if(fp == NULL){
        printf("Error opening a file");
    }
    else{
        printf("lets write bro!");
        fgets(sentence, sizeof(sentence), stdin);
        fprintf(fp, "%s", sentence);
        fclose(fp); 
    }


    return 0;
}