#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h>
  void dormir(int micros) { Sleep(micros / 1000); } // aproximação
#else
  #include <unistd.h>
  void dormir(int micros) { usleep(micros); }
#endif

void limparTela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void barraCarregando(const char *texto, int passos, int delay_us) {
    const char *frames[] = {"[=     ]", "[==    ]", "[===   ]", "[====  ]", "[===== ]", "[======]"};
    int nframes = 6;

    printf("%s ", texto);
    fflush(stdout);

    for (int i = 0; i < passos; i++) {
        printf("\r%s %s %3d%%", texto, frames[i % nframes], (i + 1) * 100 / passos);
        fflush(stdout);
        dormir(delay_us);
    }
    printf("\n");
}

void desenharFogueteSubindo(int altura, int flameFrame) {
    // "céu" com estrelas simples
    printf("     .        *        .        *\n");
    printf("   *     .        *        .     \n");
    printf("        .     *        .       * \n");
    printf("\n");

    // espaço acima do foguete (quanto maior altura, mais perto do topo)
    for (int i = 0; i < altura; i++) printf("\n");

    // foguete (ASCII simples)
    printf("        /\\\n");
    printf("       /  \\\n");
    printf("       |  |\n");
    printf("       |  |\n");
    printf("      /____\\\n");
    printf("       /||\\\n");

    // chama alternando (dá sensação de tremular)
    if (flameFrame % 3 == 0) {
        printf("        **\n");
        printf("        **\n");
    } else if (flameFrame % 3 == 1) {
        printf("       *  *\n");
        printf("        **\n");
    } else {
        printf("        **\n");
        printf("       *  *\n");
    }

    printf("\n");
    printf("Terra: ███████████████████████████████\n");
}

void viagemHorizontal(int pos, int largura) {
    // Uma linha só com \r (igual teu estilo), simulando foguete indo pro lado
    // planeta lá no fim
    char linha[256];
    if (largura > 200) largura = 200;

    for (int i = 0; i < largura; i++) linha[i] = ' ';
    linha[largura] = '\0';

    // foguete
    if (pos >= 0 && pos < largura) linha[pos] = '>';
    if (pos - 1 >= 0 && pos - 1 < largura) linha[pos - 1] = '=';
    if (pos - 2 >= 0 && pos - 2 < largura) linha[pos - 2] = '=';

    // planeta alvo (fixo à direita)
    int planeta = largura - 6;
    if (planeta >= 0) {
        linha[planeta + 0] = '(';
        linha[planeta + 1] = 'O';
        linha[planeta + 2] = 'O';
        linha[planeta + 3] = ')';
        linha[planeta + 4] = ' ';
        linha[planeta + 5] = ' ';
    }

    printf("\r%s", linha);
    fflush(stdout);
}

void pousoNoPlaneta(int passo) {
    // limpa e desenha um cenário simples com o foguete descendo
    limparTela();

    printf("      *        .     *        .      \n");
    printf("   .        *      .      *        . \n");
    printf("\n");
    printf("                (OO--)\n");
    printf("                (OOOO)   Planeta X\n");
    printf("                \n");
    printf("\n");

    int espacos = 8 - passo;   // desce aos poucos
    if (espacos < 0) espacos = 0;

    for (int i = 0; i < espacos; i++) printf("\n");

    printf("          /\\\n");
    printf("         /  \\\n");
    printf("         |  |\n");
    printf("         |  |\n");
    printf("        /____\\\n");
    printf("         /||\\\n");

    // sem chama no pouso final
    if (passo < 7) {
        printf("          **\n");
    } else {
        printf("          ||\n");
    }

    printf("\n");
    printf("Solo:  ███████████████████████████████\n");
    fflush(stdout);
}

int main() {
    // 1) Preparação / carregamento (mesma linha com \r)
    barraCarregando("Preparando sistemas", 30, 90000);
    barraCarregando("Carregando combustível", 30, 90000);
    barraCarregando("Calculando rota", 30, 90000);

    // 2) Decolagem (limpa tela e move verticalmente)
    for (int t = 0; t < 12; t++) {
        limparTela();
        int altura = 12 - t;          // sobe
        desenharFogueteSubindo(altura, t);
        dormir(120000);
    }

    // 3) Viagem (mesma linha com \r, como teu “>>>”)
    printf("\nViagem interplanetária: ");
    fflush(stdout);

    int largura = 60;
    for (int p = 2; p < largura - 10; p++) {
        viagemHorizontal(p, largura);
        dormir(60000);
    }
    printf("\n");

    // 4) Chegada / pouso
    for (int k = 0; k <= 8; k++) {
        pousoNoPlaneta(k);
        dormir(140000);
    }

    printf("\nMissão completa! Bem-vinda ao Planeta X. 🚀\n");
    return 0;
}
