#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <conio.h>


#define linhas 27
#define colunas 81
#define vitoria 10


typedef struct {
    int vidas;
    int pontos;
    int x; // É melhor deixar dois inteiros para ele que outra estrutura com dois inteiros.
    int y; // Assim podemos acessar direto "x" e "y", sem passar pela estrutura extra chamada posição.
    int animacao; // Um inteiro que mostra como vamos exibir o personagem quando ele caputura moedas ou for pego por um fantasma.
    int invencibilidade; // Contador, enquanto for diferente de 0 o personagem é invencível.
} Pac_man;


typedef struct {
    int x;
    int y;
} Fantasma; // Fantasma agora fica um nome melhor pois o Pac_man tem suas coordenadas.


typedef struct Historico {
    char mapa[linhas][colunas + 1]; // Mapa com chars assim a string pode ser gravada totalmente no arquivo e +1 para o /0 ao fim da linha na hora de exibir.
    int jogada;
    int pontos;
} Historico;


int load_data(int matriz[][colunas]) {
     // Mztriz de inteiro ao invés de char. Apesar de int ser maior que char ele é mais rápido.

    int status = -1; // Assim a gente controla se conseguiu ler ou não o arquivo, melhor que chamar um "exit".
    FILE* arquivo = fopen("matriz.txt", "rt");
    if (arquivo) {
        char caracter;
        int linha, coluna;
        for (linha = 0; linha < linhas; linha++) {
            for (coluna = 0; coluna < colunas; coluna++) {
                caracter = fgetc(arquivo);
                if ((caracter == 48) || (caracter == 49)) // A idéia é só processar caracter '0' ou '1'. Assim a matrix vai ter apenas caracteres válidos.
                    matriz[linha][coluna] = (caracter - 48); // Menos 48 pois o caracter '0' vale 48, '1' vale 49 e queremos os inteiros, 0 e 1.
                else
                    break;
            }
            if (coluna == colunas) // Terminou até a quantia máxima de colunas, neste caso precisa remover o caracter 10 (LF).
                caracter = fgetc(arquivo);
            else
                break;
        }
        fclose(arquivo);
        if (linha == linhas) // Se chegou aqui é porque leu tudo certinho. Nota que o arquivo ignora qualquer coisa depois do mapa.
            status = 0;
    }
    return status; // Este return torna a saída do programa mais elegante.

}


int pergunta_qtd_fantasma() {

    int qtd;
    do {
        printf("Informe a quantidade de fantasmas desejada nessa jogada [1-10]: ");
        scanf("%d", &qtd);
    } while ((qtd < 1) || (qtd > 10));
    return qtd;

}


void inicializar_mapa(Pac_man *personagem, Fantasma* fantasmas, const int numero_fantasmas, int matriz[][colunas]) {

    const int limite = linhas * colunas - 1; // Vamos transformar a matriz em um grande vetor sendo cada elemento identificado.
    int valor, linha, coluna;
    for(;;) {   // Aqui a gente vai sortear uma posiçãoo para colocar o jogador.
        valor = rand() % limite;
        linha = valor / colunas;
        coluna = valor % colunas;
        if (matriz[linha][coluna] == 0) { // Só adiciona se for '0', isto é, um espaço vazio.
            personagem->x = linha;
            personagem->y = coluna;
            matriz[linha][coluna] = 'P';
            break;
        }
    }
    for (int atual = 0; atual < numero_fantasmas; atual++) // Agora é hora de preencher todos os fantasmas.
        for(;;) {
            valor = rand() % limite;
            linha = valor / colunas;
            coluna = valor % colunas;
            if (matriz[linha][coluna] == 0) { // Novamente, só adiciona um fantasma em uma posição livre do mapa.
                fantasmas[atual].x = linha;
                fantasmas[atual].y = coluna;
                matriz[linha][coluna] = 'X';
                break;
            }
        }
    int pontos = 0; // Finalmente os pontos.
    while (pontos < vitoria) {
        valor = rand() % limite;
        linha = valor / colunas;
        coluna = valor % colunas;
        if (matriz[linha][coluna] == 0) { // Só adiciona o ponto se a posição for livre.
            matriz[linha][coluna] = '*';
            pontos++;
        }
    }
    for(;;) {   // Aqui a gente vai sortear uma posição para colocar o tesouro.
        valor = rand() % limite;
        linha = valor / colunas;
        coluna = valor % colunas;
        if (matriz[linha][coluna] == 0) { // Só adiciona se for '0', isto é, um espaço vazio.
            matriz[linha][coluna] = '!';
            break;
        }
    }

}


void cls() { // É um limpa tela

    COORD top = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, 32, screen.dwSize.X * screen.dwSize.Y, top, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, top, &written);
    SetConsoleCursorPosition(console, top);

}

void exibe(Pac_man *personagem, int matriz[][colunas]) {

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
    printf("Pontos: %d. Vidas [%d].", personagem->pontos, personagem->vidas);
    // A animação informa o que acabaou de ocorrer. 0 nada, 1 significa que pegou moeda e 2 que foi pego.
    if (personagem->animacao == 1)
        printf("   1 ponto pela moeda! :)\n\n");
    else if (personagem->animacao == 2)
        printf("   Fantasma te pegou! :(\n\n");
    else if (personagem->invencibilidade != 0)
        printf(" ******* !PODER! ******* \n\n");
    else
        printf("                         \n\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    int coluna;
    short cor;
    for (int linha = 0; linha < linhas; linha++) {
        for (coluna = 0; coluna < colunas; coluna++) {
            if (matriz[linha][coluna] == 0)
                printf(" ");
            else if (matriz[linha][coluna] == 1)
                printf("1");
            else if (matriz[linha][coluna] == 'P') {
                cor = (personagem->invencibilidade == 0) ? 10 : 13;  // Verde, cor padrão do personagem ou magenda se está usando poder.
                if (personagem->animacao != 0) { // Se é diferente de 0 é porque morreu ou pegou moedas.
                    cor = (personagem->animacao == 1) ? 9 : 12; // 9 se capturou moedas, azul e 12 se morreu, vermelho.
                    personagem->animacao = 0;
                }
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), cor);
                printf("P");
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Branco, cor padrão do texto.
            }
            else if (matriz[linha][coluna] == 'X') { // Exibindo fantasma.
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // Vermelho claro para fantasmas.
                printf("X");
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            }
            else if (matriz[linha][coluna] == '*') { // Exibindo moeda.
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14); // Amarelo para "moedas".
                printf("*");
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            }
            else { // Exibindo tesouro.
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13); // Este rosa brilhante, magenta, chama bem a atenção.
                printf("!");
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            }
        }
        printf("\n");
    }

}


void move_personagem(Pac_man *personagem, int matriz[][colunas]) {

    int tecla = _getch();
    if (tecla == 27) // ESC, chega de jogar.
        personagem->vidas = -1; // Vamos usar o status -1 como opção por sair do jogo.
    else {
        const int limite_vertical = linhas - 1;
        const int limite_horizontal = colunas - 1;
        int linha, coluna;
        // O personagem tenta se mover, independentemente se ele conseguiu ou não nós contamos como um movimento.
        // Assim se ele bater em uma parede, apertar uma tecla que não é movimento, etc, os fantasmas poderão se mover.
        linha = personagem->x; // A idéia aqui é usar variáveis auxiliares para ver se a nova posição é válida.
        coluna = personagem->y; // Assim não fica alterando a estrutura principal a toa.
        if (personagem->invencibilidade != 0) // Precisamos decrementar o "tempo" da invencibilidade.
            personagem->invencibilidade--; // Neste caso tempo é a quantia de movimentos restantes que torna o personagem invencível.
        if (tecla == 224) { // É controle, pega a próxima do buffer.
            tecla = _getch(); // Primeiro vamos apenas analizar o movimento e ajudar as coordenads nas variáveis temporárias.
                if (tecla == 72) { // Seta para cima.
                if (linha != 0) // Se ainda não atingimos o topo, vamos subir.
                    linha--;
                else
                    linha = limite_vertical; // Teletransporte para a posição do outro lado pois já estamos na linha 0, isto é, topo.
            }
            else if (tecla == 80) { // Seta para baixo.
                if (linha != limite_vertical) // Se ainda não atingimos o chão, vamos descer.
                    linha++;
                else
                    linha = 0; // Teletransporte para o topo pois estamos no chão.
            }
            else if (tecla == 75) { // Seta para esquerda.
                if (coluna != 0) // Se ainda não atingimos a parede lateral, vamos na direção dela.
                    coluna--;
                else
                    coluna = limite_horizontal; // Teletransporte para o outro extremo na mesma linha.
            }
            else if (tecla == 77) { // Seta para direita.
                if (coluna != limite_horizontal) // Se ainda não atingimos a parede lateral, vamos na direção dela.
                    coluna++;
                else
                    coluna = 0; // Teletransporte para o outro extremo na mesma linha.
            }
            if (matriz[linha][coluna] != 1) {// Ufa, agora podemos analisar se a posição nova é válida. Se for parede a gente ignora.
                matriz[personagem->x][personagem->y] = 0; // Desocupamos a posição aonde o pacMan estava.
                personagem->x = linha; // Atualiza a posição do personagem já que foi um movimento válido.
                personagem->y = coluna;
                if (matriz[linha][coluna] == '*') { // Ponto, só somar no placar.
                    personagem->pontos++;
                    personagem->animacao = 1; // Usado para demonstrar que pegou moedas.
                }
                else if (matriz[linha][coluna] == 'X') { // Fantasma, tira uma vida.
                    if (personagem->invencibilidade == 0) {
                        personagem->vidas--;
                        personagem->animacao = 2; // Usado para demonstrar que esbarrou em um fanstasma.
                    }
                }
                else if (matriz[linha][coluna] == '!') // Tesouro. Invencibilidade.
                    personagem->invencibilidade = 21; // 21 movimentos é um número razoável? Achei que 15 é pouco tempo.
                matriz[linha][coluna] = 'P'; // Atualizamos a posição do pacMan no mapa.
            }
        }
    }

}


int move_fantasma(Fantasma* fantasmas, const int numero_fantasmas, int matriz[][colunas]) { // Devolve int assim sabemos se o fantasma capturou o pacMan.

    int capturou = 0; // Vamos usar esta variável para controlar.
    const int limite_vertical = linhas - 1;
    const int limite_horizontal = colunas - 1;
    int linha, coluna, tentativas, valor;
    for (int fantasma = 0; fantasma < numero_fantasmas; fantasma++)
        // Se o pacMan esbarrou no fantasma a gente não pode mover este fantasma! Senão será como se ele tivesse fugido.
        if (matriz[fantasmas[fantasma].x][fantasmas[fantasma].y] != 'P') { // Pulamos o fantasma se ele capturou o pacMan.
            tentativas = 0;
             // É o seguinte... As vezes o fantasma pode ficar preso e neste caso o jogo estaria em loop. Por exemplo se o fantasma está em
             // um caminho que só tem uma saída aí outro fantasma entra nesta posição bloqueando a saída. Neste caso o fantasma fica preso.
            while (tentativas < 7) { // Então tentamos mover o fantasma 7 vezes, se não conseguir vamos para o outro fantasma.
                linha = fantasmas[fantasma].x; // A idéia aqui é usar variáveis auxiliares para ver se a nova posição é válida.
                coluna = fantasmas[fantasma].y; // Assim não fica alterando a estrutura principal a toa.
                valor = rand() % 4; // 4 pois só pode andar como o jogador. Vamos seguir o exemplo do jogador, mesmo código.
                if (valor == 0) { // Seta para cima.
                    if (linha != 0) // Se ainda não atingimos o topo, vamos subir.
                        linha--;
                    else
                        linha = limite_vertical; // Teletransporte para a posição do outro lado pois já estamos na linha 0, isto é, topo.
                }
                else if (valor == 1) { // Seta para baixo.
                    if (linha != limite_vertical) // Se ainda não atingimos o chão, vamos descer.
                        linha++;
                    else
                        linha = 0; // Teletransporte para o topo pois estamos no chão.
                }
                else if (valor == 2) { // Seta para esquerda.
                    if (coluna != 0) // Se ainda não atingimos a parede lateral, vamos na direção dela.
                        coluna--;
                    else
                        coluna = limite_horizontal; // Teletransporte para o outro extremo na mesma linha.
                }
                else { // Seta para direita. No caso seria o valor 3 pois não tem outra opção.
                    if (coluna != limite_horizontal) // Se ainda não atingimos a parede lateral, vamos na direção dela.
                        coluna++;
                    else
                        coluna = 0; // Teletransporte para o outro extremo na mesma linha.
                }
                if ((matriz[linha][coluna] == 0) || (matriz[linha][coluna] == 'P')) { // Fantasma não pega pontos, então só pode local vazio ou pegar o jogador.
                    matriz[fantasmas[fantasma].x][fantasmas[fantasma].y] = 0; // Desocupamos a posição do fantasma.
                    fantasmas[fantasma].x = linha; // Atualiza a posição do fantasma já que foi um movimento válido.
                    fantasmas[fantasma].y = coluna;
                    if (matriz[linha][coluna] == 'P')
                        capturou = 1;
                    else // Com este else garantimos que quando o fantasma comer o PacMan o ícone do personagem ainda permanece na tela.
                        matriz[linha][coluna] = 'X';
                    break; // Pode sair deste fantasma pois já conseguiu movimentar.
                }
                else
                    tentativas++; // Se movimento é inválido, tentaremos mais uma vez até o limite de 7.
            }
        }
    return capturou; // 0 se nenhum fantasma pegou o pacMan ou 1 se ao menos um deles conseguiu pegar.

}


int resumo(Pac_man *personagem, int *jogadas, int total) { // Apenas uma função, pelo status sabemos se venceu ou perdeu.

    cls();
    printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    if (personagem->vidas == 0)
        printf("*-*-*-*-*-*-*-VOCE MORREU !-*-*-*-*-*-*-*-*-*\n");
    else
        printf("*-*-*-*-*-*-*-VOCE GANHOU !-*-*-*-*-*-*-*-*-*\n");
    printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("VOCE FEZ %d PONTOS!\nSuas ultimas jogadas estao abaixo:\n", personagem->pontos);
    for(int indice = 0; indice < total; indice++)
        printf("Jogada %d: [%d].\n", indice + 1, jogadas[indice]);
    printf("Digite [1] se deseja jogar novamente.\n"); // Assim não precisa ficar controlando o que for digitado.
    int sair = 1;
    int opcao = getch();
    if (opcao == 0) // Limpa o buffer se for tecla de controle.
        opcao = getch();
    else if (opcao == 49)
        sair = 0;
    return sair;

}


void limpa_mapa(int matriz[][colunas]) {

    int linha, coluna;
    for (linha = 0; linha < linhas; linha++)
        for (coluna = 0; coluna < colunas; coluna++)
            if ((matriz[linha][coluna] != 0) && (matriz[linha][coluna] != 1)) // Se tem fantasma, pontos ou mesmo pacMan precisa limpar.
                matriz[linha][coluna] = 0;
}


void registra(Historico* historico, int matriz[][colunas], const int indice, const int pontos) { // Salva o labirinto quando as vidas foram todas perdidas.

    int coluna;
    int elemento;
    for (int linha = 0; linha < linhas; linha++) {
        for (coluna = 0; coluna < colunas; coluna++) {
            elemento = matriz[linha][coluna]; // A matriz de histórico é char.
            if (elemento == 0) // Caso seja 0 melhor colocar espaço para exibir.
                elemento = 32;
            else if (elemento == 1) // Se for 1 usamos o caracter um.
                elemento = 49;
            historico[indice].mapa[linha][coluna] = elemento;
        }
        historico[indice].mapa[linha][coluna] = 0;
    }
    historico[indice].jogada = (indice + 1); // Pois o indice sempre tem o valor da jogadas menos 1.
    historico[indice].pontos = pontos;

}


void ordena(Historico* historico, Historico** ordenados, const int jogadas) {

    int indice = 0;
    for (; indice < jogadas; indice++) // Carrega vetor de ponteiros.
        ordenados[indice] = &historico[indice]; // Cada posição aponta para um elemento do vetor de historico.
    const int ultimo = jogadas - 1; // Não precisa analisar o último elemento pois sempre estará ordenado.
    Historico* temporario; // Apenas ponteiros, nada de mover uma estrutura toda.
    int elemento;
    for (indice = 0; indice < ultimo; indice++)
        for (elemento = 0; elemento < ultimo - indice; elemento++)
            if ((ordenados[elemento]->pontos < ordenados[elemento + 1]->pontos) ||
                ((ordenados[elemento]->pontos == ordenados[elemento + 1]->pontos) && (ordenados[elemento]->jogada > ordenados[elemento + 1]->jogada))) {
                temporario = ordenados[elemento];
                ordenados[elemento] = ordenados[elemento + 1];
                ordenados[elemento + 1] = temporario;
            }

}


int salva_historico(Historico* const historico, const int jogadas) {

    int status = -1;
    // Ao invés de ordenar movendo as estruturas vale a pena usar um array de ponteiros e ordenas apenas este array.
    Historico** ordenados = malloc(jogadas * sizeof(Historico*)); // Cada elemento tem apenas o tamanho de um endereço.
    ordena(historico, ordenados, jogadas); // Ordenação.
    FILE* arquivo = fopen("saida.txt", "w"); // Arquivo aberto para escrita sobrescrevendo seu conteúdo.
    if (arquivo) {
        fprintf(arquivo, "Resumo:\n");
        int linha;
        for (int indice = 0; indice < jogadas; indice++) {
            if (ordenados[indice]->pontos == vitoria) // Informa resultado da partida.
                fprintf(arquivo, "\n\nVITORIA: ");
            else
                fprintf(arquivo, "\n\nDERROTA: "); 
            fprintf(arquivo, "Jogada %d - Pontos[%d].\n\n", ordenados[indice]->jogada, ordenados[indice]->pontos); // Resumo.
            for (linha = 0; linha < linhas; linha++) {
                fprintf(arquivo, "%s", ordenados[indice]->mapa[linha]); // Exibe como uma string pois já tem o 0 no final.
                fprintf(arquivo, "\n");
            }
            fprintf(arquivo, "\n");
        }
        fclose(arquivo); // Só fecha arquivo se realmente abriu.
        status = 0;
    }
    free (ordenados); // Libera memória do vetor auxiliar usado na ordenação.
    return status;

}

int main(int argc, char *argv[]) {

    int mapa[linhas][colunas];
    if (load_data(mapa) == 0) { // Só seguimos se conseguimos carregar o arquivo.
        time_t hora;
        srand((unsigned) time(&hora)); // Aqui e apenas aqui o seed é colocado. Depois você não mexe, assim garante uma melhor distribuição de números.
        COORD inicio = {0, 0}; // Usado para determinar aonde o cursor será posicionado.
        const int numero_fantasmas = pergunta_qtd_fantasma();
        Pac_man personagem; // Direto, não precisa alocar memória dinamicamente pois só há um.
        Fantasma* fantasmas = malloc(numero_fantasmas * sizeof(Fantasma)); // Este sim é dinâmico.
        Historico* historico = NULL;
        int* jogadas = NULL;
        int numero_jogadas = 0;
        int sair = 0;
        while (!sair) {
            numero_jogadas++; // Sempre soma uma posição ao começar.
            jogadas = realloc(jogadas, numero_jogadas * sizeof(int));
            personagem.vidas = 3; // 3 vidas
            personagem.pontos = 0; // Começa com 0 pontos, claro.
            personagem.animacao = 0; // Sem nenhum status de animação.
            personagem.invencibilidade = 0; // Não começa com poder.
            inicializar_mapa(&personagem, fantasmas, numero_fantasmas, mapa);
            cls();
            for (;;) { // Bora começar o jogo.
                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), inicio); // Posiciona o cursor na posição 0, 0 da tela. Mais simples que limpar a tela toda.
                exibe(&personagem, mapa); // Não precisa limpar a tela sempre pois o labirinto não muda de tamanho, basta exibir novamente.
                move_personagem(&personagem, mapa); // já começa mexendo pois é impossóvel ele já ter ganhado ou perdido antes de se mover.
                if (personagem.vidas == -1) { // Saiu porque não quer mais jogar, sendo assim vamos parar. Não precisa nem atualizar a tela a toa.
                    sair = 1;
                    break;
                }
                // Se não morreu nem venceu então temos de atualizar o movimento dos fantasmas.
                // Se a função de mover fantasmas retorna algo diferente de 0 é porque ao menos um fantasma conseguiu pegar o pacMan.
                // Junta tudo em um único "if" pois ele só chama a move_fantasmas se realmente antes tudo retornar "true".
                if ((personagem.vidas != 0) && (personagem.pontos != linhas) && (move_fantasma(fantasmas, numero_fantasmas, mapa) != 0)) {
                    if (personagem.invencibilidade == 0) { // Se está usando o poder o fantasma não tem poder algum aqui.
                        personagem.vidas--; // Tiramos uma vida.
                        personagem.animacao = 2; // O fantasma pegou o pacMan.
                    }
                }
                if ((personagem.vidas == 0) || (personagem.pontos == vitoria)) { // Se perdeu ou venceu a partida terminou, mostrando a conclusóo.
                    jogadas[numero_jogadas - 1] = personagem.pontos; // Atualizamos o placar desta jogada.
                    historico = realloc(historico, numero_jogadas * sizeof(Historico));
                    registra(historico, mapa, numero_jogadas - 1, personagem.pontos);
                    if (resumo(&personagem, jogadas, numero_jogadas) == 0) // Se retornou diferente de 1 é porque não quer continuar.
                        limpa_mapa(mapa); // Temos que restaurar o mapa original, isto é, deixar apenas parede e espaço vazio.
                    else
                        sair = 1;
                    break;
                }
            }
        }
        if ((personagem.vidas != -1) && (historico) && (salva_historico(historico, numero_jogadas) != 0))
            printf("ERRO NA GRAVAÇAO DOS PONTOS");
        if (historico)
            free (historico);
        free (jogadas);
        free (fantasmas);
    }
    else
        printf("Matriz corrompida.\n"); //Uma saída elegante
    return 0;

}
