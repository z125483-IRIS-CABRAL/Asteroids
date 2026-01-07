#include <stdio.h>
#include <stdlib.h> // Para system()
#include <unistd.h> // Para sleep() em Linux/macOS (use <windows.h> para Sleep() no Windows)

// Para Windows, use #include <windows.h> e Sleep(milissegundos)
// Para Linux/macOS, use #include <unistd.h> e usleep(microssegundos)

void limparTela() {
#ifdef _WIN32
    system("cls"); // Limpa tela no Windows
#else
    system("clear"); // Limpa tela em Linux/macOS
#endif
}

int main() {
    int i;
    char frame1[] = ">>>";
    char frame2[] = ".>.";
    char frame3[] = "..>";
    char frame4[] = "...";

    // Animação simples de um ponto se movendo na mesma linha
    printf("Animação com printf (mesma linha):\n");
    for (i = 0; i < 5; i++) {
        printf("\r%s", frame1); // \r volta ao início da linha
        fflush(stdout); // Garante que a saída é mostrada imediatamente
        usleep(200000); // Pausa de 200ms (200000 microssegundos)

        printf("\r%s", frame2);
        fflush(stdout);
        usleep(200000);

        printf("\r%s", frame3);
        fflush(stdout);
        usleep(200000);

        printf("\r%s", frame4);
        fflush(stdout);
        usleep(200000);
    }
    printf("\n"); // Pula linha ao final

    // Animação de uma bola quicando na tela (limpa a tela a cada frame)
    printf("\nAnimação de bola quicando (limpando tela):\n");
    for (i = 0; i < 10; i++) {
        limparTela();
        // Desenha um 'O' na posição (i%5, i%10) (exemplo simplificado)
        // Para algo mais complexo, use posicionamento de cursor com códigos ANSI
        printf("\n\n"); // Pula algumas linhas
        int j;
        for (j = 0; j < i % 5; j++){
            printf(" 
               * ");
        }

        printf("O\n");
        fflush(stdout);
        usleep(100000); // Pausa de 100ms
    }

    printf("\nAnimação finalizada!\n");

    return 0;
}
