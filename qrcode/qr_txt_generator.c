#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <string.h>

// Define o tamanho do tabuleiro do QR Code. O tabuleiro será sempre de 12x12 células.
#define TAMANHO_TABULEIRO 12
// Define o número máximo de soluções (códigos QR hipotéticos válidos) que o programa deve encontrar e armazenar.
// O programa vai parar de procurar novas soluções depois de encontrar este número.
#define MAX_SOLUCOES 10

// Variáveis globais para gerenciar o estado da busca e as soluções encontradas.
// 'solucoes' é um ponteiro para um array de ponteiros para ponteiros de inteiros (int***).
// Isso significa que ele vai armazenar múltiplos tabuleiros, onde cada tabuleiro é um 'int**'.
// Ele é inicializado como NULL e será alocado dinamicamente no início da busca.
int*** solucoes = NULL;
// 'num_solucoes_encontradas' mantém a contagem de quantos códigos QR válidos já foram descobertos.
int num_solucoes_encontradas = 0;

// Arrays para armazenar a contagem de células 'cheias' (valor 1) em cada linha e coluna do tabuleiro atual.
// São essenciais para verificar o Requisito 2 (mínimo de 5 células cheias por linha/coluna)
// e para implementar a poda eficiente durante o processo de backtracking.
int contagem_linhas[TAMANHO_TABULEIRO];
int contagem_colunas[TAMANHO_TABULEIRO];

// Array que armazena as coordenadas (linha, coluna) dos cantos do tabuleiro 12x12.
// Essas coordenadas representam o ponto superior esquerdo de um potencial bloco 2x2.
// As posições são: (0,0) - superior esquerdo, (0,10) - superior direito,
// (10,0) - inferior esquerdo, (10,10) - inferior direito.
// Usado para verificar o Requisito 1 (exatamente 3 cantos com blocos 2x2 cheios).
int coordenadas_cantos[4][2] = {{0, 0}, {0, 10}, {10, 0}, {10, 10}};

// ----- UTILITÁRIOS -----

/**
 * @brief Imprime a representação do tabuleiro do QR Code diretamente no terminal.
 * Usa o caractere '#' (hash) seguido de um espaço para células cheias (valor 1)
 * e um ponto '.' seguido de um espaço para células vazias (valor 0).
 * Essa escolha de "char " garante que cada célula ocupe 2 caracteres de largura,
 * mantendo o alinhamento visual correto no terminal.
 *
 * @param tabuleiro O tabuleiro 2D de inteiros (representando o QR Code) a ser impresso.
 */
void imprimir_tabuleiro(int** tabuleiro) {
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) { // Itera por cada linha do tabuleiro
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) { // Itera por cada coluna na linha atual
            // Usa o operador ternário: se tabuleiro[i][j] for 1 (true), imprime "# ", senão (0/false), imprime ". ".
            printf("%s", tabuleiro[i][j] ? "# " : ". ");
        }
        printf("\n"); // Após imprimir todas as células de uma linha, pula para a próxima linha no terminal.
    }
}

/**
 * @brief Salva a representação do tabuleiro do QR Code em um arquivo de texto.
 * Cria um arquivo chamado "qr_X.txt" (onde X é o índice da solução, começando de 1),
 * e escreve o tabuleiro nele, usando a mesma representação de caracteres de 'imprimir_tabuleiro'.
 *
 * @param tabuleiro O tabuleiro 2D de inteiros (representando o QR Code) a ser salvo.
 * @param indice O índice da solução (0-based), usado para gerar o nome do arquivo (1-based para o usuário).
 */
void salvar_qr_em_txt(int** tabuleiro, int indice) {
    char nome_arquivo[32]; // Buffer para armazenar o nome do arquivo.
    // snprintf é usada para formatar a string do nome do arquivo de forma segura,
    // prevenindo estouros de buffer.
    snprintf(nome_arquivo, sizeof(nome_arquivo), "qr_%d.txt", indice + 1);
    
    // Tenta abrir o arquivo no modo de escrita ("w"). Se o arquivo não existir, ele será criado.
    // Se existir, seu conteúdo será truncado (apagado) e reescrito.
    FILE* arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) { // Verifica se a abertura do arquivo falhou (fopen retorna NULL em caso de erro).
        perror("Erro ao criar arquivo"); // Imprime uma mensagem de erro descritiva sobre o problema.
        return; // Sai da função, pois não é possível continuar sem o arquivo.
    }

    for (int i = 0; i < TAMANHO_TABULEIRO; i++) { // Itera por cada linha do tabuleiro.
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) { // Itera por cada coluna na linha atual.
            // Escreve a representação da célula (como "# " ou ". ") no arquivo.
            fprintf(arquivo, "%s", tabuleiro[i][j] ? "# " : ". ");
        }
        fprintf(arquivo, "\n"); // Adiciona uma nova linha no arquivo após cada linha do tabuleiro.
    }

    fclose(arquivo); // Fecha o arquivo, liberando os recursos associados a ele.
    printf("QR salvo em: %s\n", nome_arquivo); // Informa ao usuário onde o arquivo foi salvo.
}

/**
 * @brief Verifica se um bloco 2x2 de células, começando na coordenada (r, c),
 * está completamente preenchido (todas as quatro células dentro do bloco são 1).
 *
 * @param tabuleiro O tabuleiro 2D onde a verificação será feita.
 * @param r A linha de início (canto superior esquerdo) do bloco 2x2.
 * @param c A coluna de início (canto superior esquerdo) do bloco 2x2.
 * @return true se o bloco 2x2 estiver completamente cheio, false caso contrário
 * (incluindo se o bloco extrapolar os limites do tabuleiro).
 */
bool verificar_bloco_2x2_cheio(int** tabuleiro, int r, int c) {
    // Primeiro, verifica se o bloco 2x2, começando em (r,c), não excede os limites do tabuleiro.
    // Um bloco 2x2 de (r,c) a (r+1, c+1) deve ter r+1 e c+1 menores que TAMANHO_TABULEIRO.
    if (r + 1 >= TAMANHO_TABULEIRO || c + 1 >= TAMANHO_TABULEIRO) return false;
    
    // Retorna true se todas as quatro células do bloco 2x2 tiverem o valor 1 (cheio).
    return tabuleiro[r][c] == 1 &&    // Célula superior esquerda
           tabuleiro[r][c+1] == 1 &&  // Célula superior direita
           tabuleiro[r+1][c] == 1 &&  // Célula inferior esquerda
           tabuleiro[r+1][c+1] == 1;  // Célula inferior direita
}

// ----- VALIDAÇÃO E PODA -----

/**
 * @brief Implementa a lógica de poda (pruning) para otimizar o backtracking.
 * Esta função verifica se o caminho atual de preenchimento do tabuleiro
 * tem potencial para se tornar uma solução válida, ou se já pode ser descartado.
 * É chamada durante a construção do tabuleiro, para cada célula.
 *
 * @param linha A linha da célula que está sendo preenchida atualmente.
 * @param coluna A coluna da célula que está sendo preenchida atualmente.
 * @return true se o tabuleiro parcial ainda é potencialmente válido, false se já é inviável.
 */
bool eh_valido_parcial(int linha, int coluna) {
    // Poda 1: Verifica o Requisito 2 para a **linha atual**.
    // Se a contagem atual de células cheias na 'linha'
    // mais o número máximo de células que ainda podem ser preenchidas nessa linha (do 'coluna' atual até o final)
    // for menor que 5, significa que esta linha NUNCA alcançará o mínimo de 5 células cheias.
    // Portanto, este caminho é inviável e pode ser podado.
    if (contagem_linhas[linha] + (TAMANHO_TABULEIRO - 1 - coluna) < 5) return false;

    // Poda 2: Verifica o Requisito 2 para a **coluna atual**.
    // Similar à poda de linha, mas para a coluna. Se a contagem atual de células cheias na 'coluna'
    // mais o número máximo de células que ainda podem ser preenchidas nessa coluna (das linhas abaixo da 'linha' atual)
    // for menor que 5, este caminho também é inviável.
    if (contagem_colunas[coluna] + (TAMANHO_TABULEIRO - 1 - linha) < 5) return false;
    
    // Se nenhuma das condições de poda acima foi atendida, o tabuleiro parcial ainda é potencialmente válido.
    return true;
}

/**
 * @brief Valida se um tabuleiro completo (12x12 células totalmente preenchidas)
 * atende a todos os critérios específicos do QR Code hipotético válido.
 * Esta função é chamada apenas quando o backtracking atinge o final do tabuleiro.
 *
 * @param tabuleiro O tabuleiro 2D completo a ser validado.
 * @return true se o tabuleiro atende a todos os critérios, false caso contrário.
 */
bool eh_valido_completo(int** tabuleiro) {
    // --- Requisito 1: Exatamente 3 cantos do tabuleiro devem ter blocos 2x2 totalmente cheios. ---
    int cantos_2x2_cheios = 0; // Contador para os cantos cheios.
    for (int k = 0; k < 4; k++) { // Itera sobre as 4 coordenadas de canto pré-definidas.
        int l_canto = coordenadas_cantos[k][0]; // Linha do canto.
        int c_canto = coordenadas_cantos[k][1]; // Coluna do canto.
        // Usa a função auxiliar para verificar se o bloco 2x2 no canto está cheio.
        if (verificar_bloco_2x2_cheio(tabuleiro, l_canto, c_canto)) {
            cantos_2x2_cheios++; // Incrementa o contador se o canto estiver cheio.
        }
    }
    // Se a contagem não for exatamente 3, o tabuleiro é inválido.
    if (cantos_2x2_cheios != 3) return false;

    // --- Requisito 2: A quantidade de células cheias de uma linha ou coluna não pode ser menor do que 5. ---
    // Esta verificação é feita para todas as linhas e colunas no tabuleiro completo.
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        // Se alguma linha ou alguma coluna tiver menos de 5 células cheias, o tabuleiro é inválido.
        if (contagem_linhas[i] < 5 || contagem_colunas[i] < 5) {
            return false;
        }
    }

    // --- Requisitos 3 e 4: Pelo menos duas sub-regiões de Tipo 1 e Tipo 2 devem existir. ---
    // Tipo 1: [CHEIA|VAZIA] ou [# . ]
    //         [CHEIA|CHEIA]    [# # ]
    // Tipo 2 é a inversão do Tipo 1, na coordenada transposta.
    // Tipo 2: [CHEIA|CHEIA] ou [# # ]
    //         [VAZIA|CHEIA]    [. # ]
    // 'Coordenada' é uma struct local para armazenar a linha e coluna de uma região encontrada.
    typedef struct { int l, c; } Coordenada;
    // Arrays para armazenar as coordenadas de todas as regiões de Tipo 1 e Tipo 2 encontradas.
    // O tamanho é superestimado para evitar estouro de buffer.
    Coordenada regioes_tipo1[TAMANHO_TABULEIRO * TAMANHO_TABULEIRO];
    Coordenada regioes_tipo2[TAMANHO_TABULEIRO * TAMANHO_TABULEIRO];
    int num_regioes_tipo1 = 0; // Contador para regiões de Tipo 1.
    int num_regioes_tipo2 = 0; // Contador para regiões de Tipo 2.

    // Itera sobre o tabuleiro, considerando blocos 2x2, para encontrar as sub-regiões.
    // Os loops vão até TAMANHO_TABULEIRO - 1 para garantir que o bloco 2x2 não ultrapasse o limite.
    for (int l = 0; l < TAMANHO_TABULEIRO - 1; l++) {
        for (int c = 0; c < TAMANHO_TABULEIRO - 1; c++) {
            // Verifica se o bloco 2x2 atual corresponde ao padrão do Tipo 1.
            if (tabuleiro[l][c] == 1 && tabuleiro[l][c+1] == 0 && // Primeira linha: Cheia, Vazia
                tabuleiro[l+1][c] == 1 && tabuleiro[l+1][c+1] == 1) { // Segunda linha: Cheia, Cheia
                // Se sim, armazena a coordenada inicial do bloco e incrementa o contador.
                if (num_regioes_tipo1 < TAMANHO_TABULEIRO * TAMANHO_TABULEIRO) {
                    regioes_tipo1[num_regioes_tipo1++] = (Coordenada){l, c};
                }
            }
            // Verifica se o bloco 2x2 atual corresponde ao padrão do Tipo 2.
            if (tabuleiro[l][c] == 1 && tabuleiro[l][c+1] == 1 && // Primeira linha: Cheia, Cheia
                tabuleiro[l+1][c] == 0 && tabuleiro[l+1][c+1] == 1) { // Segunda linha: Vazia, Cheia
                // Se sim, armazena a coordenada inicial do bloco e incrementa o contador.
                if (num_regioes_tipo2 < TAMANHO_TABULEIRO * TAMANHO_TABULEIRO) {
                    regioes_tipo2[num_regioes_tipo2++] = (Coordenada){l, c};
                }
            }
        }
    }
    // Se não foram encontradas pelo menos duas regiões de cada tipo, o tabuleiro é inválido.
    if (num_regioes_tipo1 < 2 || num_regioes_tipo2 < 2) return false;

    // --- Requisito 5: As sub-regiões dos passos 3 e 4 não podem fazer parte do mesmo sub-tabuleiro 3x3. ---
    // Isso significa que, para cada tipo de região, as pelo menos duas ocorrências devem estar em blocos 3x3 diferentes.
    // Arrays para armazenar os IDs únicos dos sub-tabuleiros 3x3 onde as regiões foram encontradas.
    int sub_tabuleiros_vistos1[TAMANHO_TABULEIRO * TAMANHO_TABULEIRO];
    int count_sub_tabuleiros1 = 0; // Contador para IDs únicos de regiões Tipo 1.
    
    // Processa as regiões de Tipo 1.
    for (int i = 0; i < num_regioes_tipo1; i++) {
        // Calcula o ID do sub-tabuleiro 3x3 ao qual a região pertence.
        // Ex: (l=0, c=0) -> ID 0; (l=0, c=3) -> ID 1; (l=3, c=0) -> ID 10.
        int id_bloco = (regioes_tipo1[i].l / 3) * 10 + (regioes_tipo1[i].c / 3);
        bool ja_visto = false;
        // Verifica se este ID de bloco 3x3 já foi registrado.
        for (int j = 0; j < count_sub_tabuleiros1; j++) {
            if (sub_tabuleiros_vistos1[j] == id_bloco) {
                ja_visto = true;
                break;
            }
        }
        // Se o ID ainda não foi visto, adiciona-o ao array de IDs únicos.
        if (!ja_visto) {
            if (count_sub_tabuleiros1 < TAMANHO_TABULEIRO * TAMANHO_TABULEIRO) {
                sub_tabuleiros_vistos1[count_sub_tabuleiros1++] = id_bloco;
            }
        }
    }
    // Para ser válido, deve haver pelo menos duas regiões de Tipo 1 em sub-tabuleiros 3x3 *distintos*.
    if (count_sub_tabuleiros1 < 2) return false;

    // Faz o mesmo processamento para as regiões de Tipo 2.
    int sub_tabuleiros_vistos2[TAMANHO_TABULEIRO * TAMANHO_TABULEIRO];
    int count_sub_tabuleiros2 = 0;

    for (int i = 0; i < num_regioes_tipo2; i++) {
        int id_bloco = (regioes_tipo2[i].l / 3) * 10 + (regioes_tipo2[i].c / 3);
        bool ja_visto = false;
        for (int j = 0; j < count_sub_tabuleiros2; j++) {
            if (sub_tabuleiros_vistos2[j] == id_bloco) {
                ja_visto = true;
                break;
            }
        }
        if (!ja_visto) {
            if (count_sub_tabuleiros2 < TAMANHO_TABULEIRO * TAMANHO_TABULEIRO) {
                sub_tabuleiros_vistos2[count_sub_tabuleiros2++] = id_bloco;
            }
        }
    }
    // Para ser válido, deve haver pelo menos duas regiões de Tipo 2 em sub-tabuleiros 3x3 *distintos*.
    if (count_sub_tabuleiros2 < 2) return false;

    // Se todas as verificações acima passaram, o tabuleiro é considerado um QR Code hipotético válido.
    return true;
}

// ----- BACKTRACKING -----

/**
 * @brief Função principal de backtracking que explora todas as combinações possíveis
 * de preenchimento do tabuleiro para encontrar códigos QR válidos.
 * Esta é uma função recursiva que tenta preencher cada célula do tabuleiro.
 *
 * @param tabuleiro O tabuleiro 2D atual sendo construído.
 * @param linha A linha da célula atual a ser preenchida.
 * @param coluna A coluna da célula atual a ser preenchida.
 */
void resolver(int** tabuleiro, int linha, int coluna) {
    // Poda: Se já encontramos o número máximo de soluções (MAX_SOLUCOES),
    // não precisamos continuar a busca. Retorna imediatamente.
    if (num_solucoes_encontradas >= MAX_SOLUCOES) {
        return;
    }

    // Caso base da recursão: Se a linha for igual a TAMANHO_TABULEIRO,
    // significa que todas as células do tabuleiro foram preenchidas (da 0,0 até a última).
    if (linha == TAMANHO_TABULEIRO) {
        // Neste ponto, o tabuleiro está completo. Agora, verificamos se ele é um QR Code válido
        // de acordo com todos os requisitos (chama 'eh_valido_completo').
        if (eh_valido_completo(tabuleiro)) {
            // Se for válido, é uma solução! Precisamos armazená-la.
            // Aloca memória para uma nova cópia do tabuleiro válido.
            int** solucao_encontrada = malloc(TAMANHO_TABULEIRO * sizeof(int*));
            if (!solucao_encontrada) { perror("Erro de alocacao para solucao"); return; }

            for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
                solucao_encontrada[i] = malloc(TAMANHO_TABULEIRO * sizeof(int));
                if (!solucao_encontrada[i]) { perror("Erro de alocacao para linha da solucao"); return; }
                // Copia o conteúdo da linha do tabuleiro atual para a nova solução.
                for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
                    solucao_encontrada[i][j] = tabuleiro[i][j];
                }
            }
            // Adiciona a solução copiada ao array global 'solucoes' e incrementa o contador.
            solucoes[num_solucoes_encontradas++] = solucao_encontrada;
        }
        return; // Retorna após processar um tabuleiro completo (seja ele válido ou não).
    }

    // Calcula as coordenadas da próxima célula a ser preenchida na sequência (esquerda para direita, cima para baixo).
    int proxima_linha = linha;
    int proxima_coluna = coluna + 1;
    // Se a coluna atual for a última da linha (TAMANHO_TABULEIRO - 1),
    // a próxima célula será na próxima linha, começando da coluna 0.
    if (proxima_coluna == TAMANHO_TABULEIRO) {
        proxima_linha++;
        proxima_coluna = 0;
    }

    // Loop principal do backtracking: Tenta preencher a célula atual com 0 (vazia) ou 1 (cheia).
    for (int valor = 0; valor <= 1; valor++) {
        // 1. Fazer a escolha: Define o valor da célula atual.
        tabuleiro[linha][coluna] = valor;
        // Atualiza as contagens de células cheias para a linha e coluna correspondentes.
        contagem_linhas[linha] += valor;
        contagem_colunas[coluna] += valor;

        // 2. Podar (Pruning): Verifica se a escolha atual ainda leva a um caminho potencialmente válido.
        // Chama 'eh_valido_parcial' para verificar as condições de poda (min. de 5 células por linha/coluna).
        if (eh_valido_parcial(linha, coluna)) {
            // Se o caminho ainda é válido, faz a chamada recursiva para a próxima célula.
            resolver(tabuleiro, proxima_linha, proxima_coluna);
        }

        // 3. Desfazer a escolha (Backtrack): Após a chamada recursiva retornar,
        // é preciso reverter as alterações para explorar outras possibilidades.
        contagem_linhas[linha] -= valor; // Remove o valor da contagem da linha.
        contagem_colunas[coluna] -= valor; // Remove o valor da contagem da coluna.
        tabuleiro[linha][coluna] = 0; // Reseta a célula para 0 (vazia), preparando para a próxima tentativa ou retorno.
    }
}

/**
 * @brief Inicia o processo de geração e busca por códigos QR hipotéticos válidos.
 * Esta função configura o ambiente inicial e chama a função de backtracking.
 */
void gerar_codigos_qr() {
    // Inicializa os contadores de linha e coluna com zero no início de cada geração.
    for(int i = 0; i < TAMANHO_TABULEIRO; i++) {
        contagem_linhas[i] = 0;
        contagem_colunas[i] = 0;
    }

    // Pré-aloca a memória para o array de ponteiros que armazenará as soluções encontradas.
    // Isso é feito uma única vez no início, otimizando a alocação de memória durante o backtracking.
    solucoes = malloc(sizeof(int**) * MAX_SOLUCOES);
    if (!solucoes) { // Verifica se a alocação falhou.
        perror("Erro de alocacao para array de solucoes");
        return; // Sai da função se não for possível alocar.
    }

    // Aloca o tabuleiro inicial que será usado pela função de backtracking.
    // É um tabuleiro 12x12 de inteiros.
    int** tabuleiro_inicial = malloc(TAMANHO_TABULEIRO * sizeof(int*));
    if (!tabuleiro_inicial) { perror("Erro de alocacao para tabuleiro inicial"); return; }
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        // Para cada linha, aloca memória para as colunas e inicializa todas com 0 (célula vazia) usando calloc.
        tabuleiro_inicial[i] = calloc(TAMANHO_TABULEIRO, sizeof(int));
        if (!tabuleiro_inicial[i]) { perror("Erro de alocacao para linha do tabuleiro inicial"); return; }
    }

    // Inicia o processo de backtracking, começando da primeira célula (0,0).
    resolver(tabuleiro_inicial, 0, 0);

    // Após a busca, libera a memória alocada para o tabuleiro inicial.
    // As soluções encontradas (se houver) já foram copiadas e ainda estão em 'solucoes'.
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        free(tabuleiro_inicial[i]);
    }
    free(tabuleiro_inicial);
}

// ----- FUNÇÃO PRINCIPAL -----

/**
 * @brief Função principal do programa. Inicia o processo de geração de QR Codes,
 * imprime as soluções encontradas no terminal e as salva em arquivos de texto,
 * e finalmente libera toda a memória alocada.
 *
 * @return 0 se o programa executar com sucesso.
 */
int main() {
    // Chama a função para iniciar a busca e geração dos códigos QR hipotéticos.
    gerar_codigos_qr();

    // Verifica se alguma solução válida foi encontrada.
    if (num_solucoes_encontradas > 0) {
        printf("Encontrado %d codigo(s) QR hipotetico(s) valido(s):\n", num_solucoes_encontradas);
        // Itera sobre cada solução encontrada.
        for (int i = 0; i < num_solucoes_encontradas; i++) {
            printf("\nCodigo QR %d:\n", i + 1);
            imprimir_tabuleiro(solucoes[i]); // Imprime a solução no terminal.
            salvar_qr_em_txt(solucoes[i], i); // Salva a solução em um arquivo de texto.

            // Libera a memória de cada tabuleiro de solução individualmente.
            for (int r = 0; r < TAMANHO_TABULEIRO; r++) {
                free(solucoes[i][r]);
            }
            free(solucoes[i]);
        }
        free(solucoes); // Libera o array principal de ponteiros para as soluções.
    } else {
        // Mensagem caso nenhuma solução seja encontrada.
        printf("Nenhum QR Code hipotetico valido encontrado com os criterios especificados.\n");
    }

    return 0; // Indica que o programa terminou com sucesso.
}