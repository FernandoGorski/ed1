#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <locale.h>

#define LARGURA_TABULEIRO 80
#define ALTURA_TABULEIRO 40
#define DIRECAO_NORTE 1
#define DIRECAO_SUL 2
#define DIRECAO_LESTE 3
#define DIRECAO_OESTE 4

typedef struct SegmentoCobra {
    int x; // coordenada x da c�lula
    int y; // coordenada y da c�lula
    struct SegmentoCobra* proximo; // pr�ximo segmento da cobra
} SegmentoCobra;

typedef struct Cobra {
    SegmentoCobra* cabeca; // cabe�a da cobra
    SegmentoCobra* cauda; // cauda da cobra
    int comprimento; // comprimento da cobra
} Cobra;

typedef struct RegistroTempo {
    int tempo;
    struct RegistroTempo* proximo;
} RegistroTempo;

typedef struct Movimento {
    int x; // coordenada x do movimento
    int y; // coordenada y do movimento
    struct Movimento* proximo; // pr�ximo movimento
    struct Movimento* anterior; // movimento anterior
} Movimento;

typedef struct RegistoMovimentos {
    Movimento* inicio; // in�cio da lista de movimentos
    Movimento* fim; // fim da lista de movimentos
} RegistoMovimentos;

void posicionarCobraInicial(Cobra* cobra) {
    SegmentoCobra* segmento = (SegmentoCobra*)malloc(sizeof(SegmentoCobra));
    segmento->x = 0;
    segmento->y = 0;
    segmento->proximo = NULL;

    cobra->cabeca = segmento;
    cobra->cauda = segmento;
    cobra->comprimento = 1;
}

void gerarPosicaoComida(int* posicaoX, int* posicaoY) {
    srand(time(NULL));
    *posicaoX = (rand() % (LARGURA_TABULEIRO - 2)) + 1; // Gera um valor aleat�rio entre 1 e LARGURA_TABULEIRO - 2
    *posicaoY = (rand() % (ALTURA_TABULEIRO - 2)) + 1; // Gera um valor aleat�rio entre 1 e ALTURA_TABULEIRO - 2
}

void desenharTabuleiro(Cobra* cobra, int posicaoComidaX, int posicaoComidaY) {
    int i, j;
    for (i = 0; i < ALTURA_TABULEIRO; i++) {
        for (j = 0; j < LARGURA_TABULEIRO; j++) {
            if (i == 0 || i == ALTURA_TABULEIRO - 1 || j == 0 || j == LARGURA_TABULEIRO - 1) {
                printf("#"); // borda do tabuleiro
            } else if (i == posicaoComidaY && j == posicaoComidaX) {
                printf("F"); // comida
            } else {
                // Verifica se a c�lula pertence � cobra
                SegmentoCobra* segmento = cobra->cabeca;
                while (segmento != NULL) {
                    if (segmento->x == j && segmento->y == i) {
                        printf("C"); // parte da cobra
                        break;
                    }
                    segmento = segmento->proximo;
                }
                if (segmento == NULL) {
                    printf(" "); // c�lula vazia
                }
            }
        }
        printf("\n");
    }
}

void criarTabuleiro() {
    system("cls"); // Limpar a tela do console
    setlocale(LC_ALL, ""); // Configurar a localiza��o para permitir a exibi��o correta dos caracteres especiais
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo); // Ocultar o cursor do console
}

int obterDirecao() {
    if (_kbhit()) {
        int tecla = _getch();
        if (tecla == 'w' || tecla == 'W') {
            return DIRECAO_NORTE;
        } else if (tecla == 's' || tecla == 'S') {
            return DIRECAO_SUL;
        } else if (tecla == 'd' || tecla == 'D') {
            return DIRECAO_LESTE;
        } else if (tecla == 'a' || tecla == 'A') {
            return DIRECAO_OESTE;
        }
    }
    return 0; // Se nenhuma tecla de dire��o for pressionada, retorna 0 (dire��o inv�lida)
}
void exibirSegmento(int x, int y, char caractere) {
    COORD posicao;
    HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);

    posicao.X = x;
    posicao.Y = y;
    SetConsoleCursorPosition(saida, posicao);
    printf("%c", caractere);
}


// Fun��o para adicionar um novo movimento ao registro
void adicionarMovimento(RegistoMovimentos* registro, int x, int y) {
    Movimento* novoMovimento = (Movimento*)malloc(sizeof(Movimento));
    novoMovimento->x = x;
    novoMovimento->y = y;
    novoMovimento->proximo = NULL;

    if (registro->inicio == NULL) {
        registro->inicio = novoMovimento;
        registro->fim = novoMovimento;
        novoMovimento->anterior = NULL;
    } else {
        novoMovimento->anterior = registro->fim;
        registro->fim->proximo = novoMovimento;
        registro->fim = novoMovimento;
    }
}

// Fun��o para desfazer o �ltimo movimento realizado
void desfazerMovimento(RegistoMovimentos* registro) {
    if (registro->fim != NULL) {
        Movimento* movimentoDesfeito = registro->fim;
        registro->fim = movimentoDesfeito->anterior;
        if (registro->fim != NULL) {
            registro->fim->proximo = NULL;
        } else {
            registro->inicio = NULL;
        }
        free(movimentoDesfeito);
    }
}

int verificarMovimentoValido(Cobra cobra, int proximoX, int proximoY) {
    // Verificar se o movimento est� dentro dos limites do tabuleiro
    if (proximoX < 0 || proximoX >= LARGURA_TABULEIRO || proximoY < 0 || proximoY >= ALTURA_TABULEIRO) {
        return 0; // Movimento inv�lido (fora dos limites)
    }

    // Verificar se a cobra est� se movendo para tr�s (n�o � permitido)
    if (cobra.comprimento > 1 && proximoX == cobra.cauda->x && proximoY == cobra.cauda->y) {
        return 0; // Movimento inv�lido (voltando sobre si mesma)
    }

    return 1; // Movimento v�lido
}

int moverCobra(Cobra* cobra, int direcao, int* posicaoComidaX, int* posicaoComidaY, RegistoMovimentos* registroMovimentos) {
    int proximoX = cobra->cabeca->x;
    int proximoY = cobra->cabeca->y;

    switch (direcao) {
        case DIRECAO_NORTE:
            proximoY--;
            break;
        case DIRECAO_SUL:
            proximoY++;
            break;
        case DIRECAO_LESTE:
            proximoX++;
            break;
        case DIRECAO_OESTE:
            proximoX--;
            break;
    }

    if (verificarMovimentoValido(*cobra, proximoX, proximoY)) {
        // Atualizar a posi��o da cabe�a da cobra
        SegmentoCobra* novoSegmento = (SegmentoCobra*)malloc(sizeof(SegmentoCobra));
        novoSegmento->x = proximoX;
        novoSegmento->y = proximoY;
        novoSegmento->proximo = cobra->cabeca;
        cobra->cabeca = novoSegmento;

        // Verificar se a cabe�a da cobra atingiu a comida
        if (proximoX == *posicaoComidaX && proximoY == *posicaoComidaY) {
            // Aumentar o comprimento da cobra
            cobra->comprimento++;

            // Gerar nova posi��o para a comida
            gerarPosicaoComida(posicaoComidaX, posicaoComidaY);

            // Adicionar o movimento ao registro
            adicionarMovimento(registroMovimentos, proximoX, proximoY);

            return 1; // Atingiu a comida
        } else {
            // Remover o �ltimo segmento da cauda da cobra
            SegmentoCobra* segmentoAtual = cobra->cabeca;
            while (segmentoAtual->proximo != cobra->cauda) {
                segmentoAtual = segmentoAtual->proximo;
            }
            free(cobra->cauda);
            cobra->cauda = segmentoAtual;
            cobra->cauda->proximo = NULL;

            // Adicionar o movimento ao registro
            adicionarMovimento(registroMovimentos, proximoX, proximoY);

            return 0; // N�o atingiu a comida
        }
    } else {
        return 0; // Movimento inv�lido
    }
}

void adicionarRegistro(RegistroTempo** pilha, int novoTempo) {
    RegistroTempo* novoRegistro = (RegistroTempo*)malloc(sizeof(RegistroTempo));
    novoRegistro->tempo = novoTempo;

    if (*pilha == NULL || novoTempo < (*pilha)->tempo) {
        novoRegistro->proximo = *pilha;
        *pilha = novoRegistro;
    } else {
        RegistroTempo* atual = *pilha;
        while (atual->proximo != NULL && novoTempo >= atual->proximo->tempo) {
            atual = atual->proximo;
        }
        novoRegistro->proximo = atual->proximo;
        atual->proximo = novoRegistro;
    }
}

void exibirMelhoresTempos(RegistroTempo* pilha) {
    printf("----- MELHORES TEMPOS -----\n");
    int contador = 1;
    RegistroTempo* atual = pilha;
    while (atual != NULL && contador <= 5) {
        printf("%d. %d ms\n", contador, atual->tempo);
        atual = atual->proximo;
        contador++;
    }
}

int main() {
    int opcao = 0;
    Cobra cobra;
    int posicaoComidaX, posicaoComidaY;
    RegistoMovimentos registroMovimentos;
    RegistroTempo* pilhaMelhoresTempos = NULL;
    criarTabuleiro();
    posicionarCobraInicial(&cobra);
    gerarPosicaoComida(&posicaoComidaX, &posicaoComidaY);

    while (opcao != 6) {
        printf("\n----- MENU -----\n");
        printf("1. Criar um novo tabuleiro\n");
        printf("2. Recriar do zero o �ltimo caminho percorrido\n");
        printf("3. Retornar o �ltimo caminho do fim para o in�cio\n");
        printf("4. Alterar a velocidade de avan�o da cobra\n");
        printf("5. Listar os melhores tempos (recordes)\n");
        printf("6. Sair do programa\n");
        printf("Escolha uma op��o: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                criarTabuleiro();
                posicionarCobraInicial(&cobra);
                gerarPosicaoComida(&posicaoComidaX, &posicaoComidaY);
                registroMovimentos.inicio = NULL;
                registroMovimentos.fim = NULL;
                break;
            case 2:
                while (registroMovimentos.inicio != NULL) {
                    desfazerMovimento(&registroMovimentos);
                }
                break;
            case 3:
                if (registroMovimentos.fim != NULL) {
                    Movimento* movimentoAtual = registroMovimentos.fim;
                    while (movimentoAtual != NULL) {
                        int proximoX = cobra.cabeca->x;
                        int proximoY = cobra.cabeca->y;
                        int direcao;
                        if (movimentoAtual->x < proximoX) {
                            direcao = DIRECAO_OESTE;
                        } else if (movimentoAtual->x > proximoX) {
                            direcao = DIRECAO_LESTE;
                        } else if (movimentoAtual->y < proximoY) {
                            direcao = DIRECAO_NORTE;
                        } else if (movimentoAtual->y > proximoY) {
                            direcao = DIRECAO_SUL;
                        }
                        if (moverCobra(&cobra, direcao, &posicaoComidaX, &posicaoComidaY, &registroMovimentos)) {
                            registroMovimentos.fim = movimentoAtual->anterior;
                            free(movimentoAtual);
                            movimentoAtual = registroMovimentos.fim;
                        } else {
                            break;
                        }
                    }
                }
                break;
            case 4:
                // C�digo para alterar a velocidade de avan�o da cobra
                break;
            case 5:
                exibirMelhoresTempos(pilhaMelhoresTempos);
                break;
            case 6:
                break;
            default:
                printf("Op��o inv�lida. Por favor, escolha uma op��o v�lida.\n");
                break;
        }

        if (opcao != 6) {
    printf("\nPressione qualquer tecla para continuar...");
    getchar();
    getchar(); // Aguarda o usu�rio pressionar qualquer tecla para continuar
    
    return 0;
}}}

           
