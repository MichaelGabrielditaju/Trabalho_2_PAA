#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // Para usar 'true' e 'false'

#define BOARD_SIZE 12
#define MAX_SOLUTIONS 1 // Parar após encontrar o primeiro QR code

// Variável global para armazenar as soluções encontradas (simplificação para este exemplo)
// Em uma aplicação real, você usaria uma lista dinâmica ou retornaria a primeira solução.
int*** solutions = NULL;
int num_solutions_found = 0;

// Função para imprimir o tabuleiro 
void print_board(int** board) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 1) {
                printf("██"); // Caractere para célula cheia (bloco sólido)
            } else {
                printf("  "); // Dois espaços para célula vazia
            }
        }
        printf("\n");
    }
}

// Função para copiar um tabuleiro (necessário para armazenar soluções)
void copy_board(int** dest, int** src) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

// Função de validação parcial (poda)
bool is_valid_partial(int** board, int row, int col) {
    // Esta é a parte mais complexa para otimização em tempo real.
    // Para simplificar e evitar lógica muito complexa de predição:
    // A validação de "mínimo de 5 células" é mais eficaz no 'is_valid_full'
    // ou com contadores persistentes para cada linha/coluna durante a recursão.
    // Aqui, podemos adicionar podas muito básicas.

    // Exemplo de poda muito básica: se uma linha já tem mais zeros do que pode ter para atingir 5 '1's
    // Mas para isso, precisaríamos saber quantos '1's já foram colocados na linha.
    // Para este exemplo, manteremos a poda simples, focando na validade final.
    return true; // No momento, nenhuma poda parcial agressiva implementada aqui para simplicidade.
}

// Função de validação completa
bool is_valid_full(int** board) {
    // 1 - Cantos 2x2 Cheios
    int corner_blocks_filled = 0;
    // (0,0), (0,10), (10,0), (10,10) para blocos 2x2
    int corners[4][2] = {{0, 0}, {0, 10}, {10, 0}, {10, 10}};

    for (int k = 0; k < 4; k++) {
        int r_start = corners[k][0];
        int c_start = corners[k][1];
        bool is_filled = true;

        if (r_start + 1 < BOARD_SIZE && c_start + 1 < BOARD_SIZE) {
            for (int i = r_start; i < r_start + 2; i++) {
                for (int j = c_start; j < c_start + 2; j++) {
                    if (board[i][j] != 1) {
                        is_filled = false;
                        break;
                    }
                }
                if (!is_filled) {
                    break;
                }
            }
        } else {
            is_filled = false; // Canto 2x2 extrapolaria o tabuleiro
        }

        if (is_filled) {
            corner_blocks_filled++;
        }
    }

    if (corner_blocks_filled != 3) {
        return false;
    }

    // 2 - Mínimo de Células Cheias por Linha/Coluna
    for (int i = 0; i < BOARD_SIZE; i++) {
        int row_sum = 0;
        int col_sum = 0;
        for (int j = 0; j < BOARD_SIZE; j++) {
            row_sum += board[i][j];
            col_sum += board[j][i];
        }
        if (row_sum < 5 || col_sum < 5) {
            return false;
        }
    }

    // 3 - Sub-regiões Tipo 1 ([CHEIA|VAZIA], [CHEIA|CHEIA])
    typedef struct { int r, c; } RegionCoord;
    RegionCoord type1_regions[BOARD_SIZE * BOARD_SIZE]; // Array para armazenar coordenadas das regiões
    int num_type1_regions = 0;

    for (int r = 0; r < BOARD_SIZE - 1; r++) {
        for (int c = 0; c < BOARD_SIZE - 1; c++) {
            if (board[r][c] == 1 && board[r][c+1] == 0 &&
                board[r+1][c] == 1 && board[r+1][c+1] == 1) {
                if (num_type1_regions < BOARD_SIZE * BOARD_SIZE) { // Evitar overflow
                    type1_regions[num_type1_regions].r = r;
                    type1_regions[num_type1_regions].c = c;
                    num_type1_regions++;
                }
            }
        }
    }
    if (num_type1_regions < 2) {
        return false;
    }

    // 4 - Sub-regiões Tipo 2 ([CHEIA|CHEIA], [VAZIA|CHEIA])
    RegionCoord type2_regions[BOARD_SIZE * BOARD_SIZE];
    int num_type2_regions = 0;

    for (int r = 0; r < BOARD_SIZE - 1; r++) {
        for (int c = 0; c < BOARD_SIZE - 1; c++) {
            if (board[r][c] == 1 && board[r][c+1] == 1 &&
                board[r+1][c] == 0 && board[r+1][c+1] == 1) {
                if (num_type2_regions < BOARD_SIZE * BOARD_SIZE) { // Evitar overflow
                    type2_regions[num_type2_regions].r = r;
                    type2_regions[num_type2_regions].c = c;
                    num_type2_regions++;
                }
            }
        }
    }
    if (num_type2_regions < 2) {
        return false;
    }

    // 5 - Sub-regiões em Sub-tabuleiros Distintos (3x3)
    // Usamos um array para simular um conjunto (set) de sub-tabuleiros
    // Para simplificar, armazenamos (r_block * 100 + c_block) como um ID único
    
    // Critério 5.1: Tipo 1 em sub-tabuleiros distintos
    int sub_boards_type1_ids[BOARD_SIZE * BOARD_SIZE];
    int num_sub_boards_type1 = 0;

    for (int i = 0; i < num_type1_regions; i++) {
        int r_block = type1_regions[i].r / 3;
        int c_block = type1_regions[i].c / 3;
        int id = r_block * 100 + c_block; // ID único para o sub-tabuleiro

        bool found = false;
        for (int j = 0; j < num_sub_boards_type1; j++) {
            if (sub_boards_type1_ids[j] == id) {
                found = true;
                break;
            }
        }
        if (!found) {
            if (num_sub_boards_type1 < BOARD_SIZE * BOARD_SIZE) {
                sub_boards_type1_ids[num_sub_boards_type1++] = id;
            }
        }
    }
    if (num_sub_boards_type1 < 2) {
        return false;
    }

    // Critério 5.2: Tipo 2 em sub-tabuleiros distintos
    int sub_boards_type2_ids[BOARD_SIZE * BOARD_SIZE];
    int num_sub_boards_type2 = 0;

    for (int i = 0; i < num_type2_regions; i++) {
        int r_block = type2_regions[i].r / 3;
        int c_block = type2_regions[i].c / 3;
        int id = r_block * 100 + c_block;

        bool found = false;
        for (int j = 0; j < num_sub_boards_type2; j++) {
            if (sub_boards_type2_ids[j] == id) {
                found = true;
                break;
            }
        }
        if (!found) {
            if (num_sub_boards_type2 < BOARD_SIZE * BOARD_SIZE) {
                sub_boards_type2_ids[num_sub_boards_type2++] = id;
            }
        }
    }
    if (num_sub_boards_type2 < 2) {
        return false;
    }

    return true;
}

// Função de backtracking principal
void solve(int** board, int row, int col) {
    if (num_solutions_found >= MAX_SOLUTIONS) {
        return; // Poda: Parar se já encontramos o número desejado de soluções
    }

    // Se chegamos ao final do tabuleiro, verificamos se é uma solução válida
    if (row == BOARD_SIZE) {
        if (is_valid_full(board)) {
            // Alocar e copiar o tabuleiro para as soluções
            int** found_board = (int**)malloc(BOARD_SIZE * sizeof(int*));
            for (int i = 0; i < BOARD_SIZE; i++) {
                found_board[i] = (int*)malloc(BOARD_SIZE * sizeof(int));
            }
            copy_board(found_board, board);

            // Adicionar à lista de soluções
            if (solutions == NULL) {
                solutions = (int***)malloc(MAX_SOLUTIONS * sizeof(int**));
            }
            solutions[num_solutions_found++] = found_board;
        }
        return;
    }

    // Calcular a próxima célula
    int next_row = row;
    int next_col = col + 1;
    if (next_col == BOARD_SIZE) {
        next_row++;
        next_col = 0;
    }

    // Tentar preencher com 0 (vazio)
    board[row][col] = 0;
    if (is_valid_partial(board, row, col)) {
        solve(board, next_row, next_col);
    }
    
    if (num_solutions_found >= MAX_SOLUTIONS) {
        return; // Poda: Se achou solução com 0, parar
    }

    // Tentar preencher com 1 (cheio)
    board[row][col] = 1;
    if (is_valid_partial(board, row, col)) {
        solve(board, next_row, next_col);
    }
    
    // Não precisamos resetar board[row][col] aqui porque ele será sobrescrito
    // ou a recursão superior já terá sua própria cópia ou valor.
    // É importante garantir que o estado do tabuleiro esteja correto ao retornar.
}

// Função principal para iniciar a geração
void generate_qr_codes() {
    // Alocar o tabuleiro inicial
    int** initial_board = (int**)malloc(BOARD_SIZE * sizeof(int*));
    for (int i = 0; i < BOARD_SIZE; i++) {
        initial_board[i] = (int*)calloc(BOARD_SIZE, sizeof(int)); // Inicia com zeros
    }

    solve(initial_board, 0, 0);

    // Desalocar o tabuleiro inicial
    for (int i = 0; i < BOARD_SIZE; i++) {
        free(initial_board[i]);
    }
    free(initial_board);
}

int main() {
    generate_qr_codes();

    if (num_solutions_found > 0) {
        printf("Encontrados %d codigos QR hipoteticos validos:\n", num_solutions_found);
        for (int i = 0; i < num_solutions_found; i++) {
            printf("\nCodigo QR %d:\n", i + 1);
            print_board(solutions[i]);

            // Desalocar cada tabuleiro de solução
            for (int r = 0; r < BOARD_SIZE; r++) {
                free(solutions[i][r]);
            }
            free(solutions[i]);
        }
        free(solutions); // Desalocar o array de ponteiros para as soluções
    } else {
        printf("Nenhum codigo QR hipotetico valido encontrado com os criterios especificados.\n");
    }

    return 0;
}