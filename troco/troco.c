#include <stdio.h>    
#include <stdlib.h>  
#include <limits.h>
/*
 * ===================================================================================
 * FUNÇÃO PRINCIPAL: encontrarTrocoOtimoComPeso
 * ===================================================================================
 * @brief  Esta função implementa o algoritmo de programação dinâmica para resolver
 * o problema do troco com peso mínimo e exibe a tabela de DP no formato 2D.
 *
 * @param valores   Array contendo os valores de cada tipo de moeda (ex: 1, 5, 10).
 * @param pesos     Array contendo os pesos correspondentes de cada tipo de moeda.
 * @param n         O número de tipos diferentes de moedas disponíveis.
 * @param troco     O valor do troco final que desejamos compor.
 */
void encontrarTrocoOtimoComPeso(int valores[], int pesos[], int n, int troco) {
    // Se o troco for 0, não há o que fazer.
    if (troco == 0) {
        printf("O peso minimo para o troco de 0 e: 0\n");
        return;
    }
    // Se o troco for negativo, é inválido.
    if (troco < 0) {
        printf("Valor de troco invalido.\n");
        return;
    }

    /*
     * -------------------------------------------------------------------------------
     * ALOCAÇÃO DE MEMÓRIA PARA AS ESTRUTURAS DA PROGRAMAÇÃO DINÂMICA
     * -------------------------------------------------------------------------------
     */
    long long* peso_minimo = (long long*)malloc((troco + 1) * sizeof(long long));
    int* ultima_moeda = (int*)malloc((troco + 1) * sizeof(int));
    int* contagem_moedas = (int*)calloc(n, sizeof(int));

    // Validação da alocação de memória
    if (peso_minimo == NULL || ultima_moeda == NULL || contagem_moedas == NULL) {
        printf("Falha na alocacao de memoria!\n");
        free(peso_minimo);
        free(ultima_moeda);
        free(contagem_moedas);
        return;
    }

    /*
     * -------------------------------------------------------------------------------
     * PASSO 1: INICIALIZAÇÃO DOS ARRAYS DA DP
     * -------------------------------------------------------------------------------
     */
    peso_minimo[0] = 0;
    ultima_moeda[0] = 0;

    for (int i = 1; i <= troco; i++) {
        peso_minimo[i] = LLONG_MAX;
        ultima_moeda[i] = -1;
    }

    /*
     * -------------------------------------------------------------------------------
     * EXIBIÇÃO DA TABELA DE PROGRAMAÇÃO DINÂMICA
     * -------------------------------------------------------------------------------
     */
    printf("\n--- Tabela de Programacao Dinamica Gerada ---\n");

    printf("v  p |");
    for (int v = 0; v <= troco; v++) { printf("%4d", v); }
    printf("\n-----+");
    for (int v = 0; v <= troco; v++) { printf("----"); }
    printf("\n");

    printf("0  0 |%4lld", peso_minimo[0]);
    for (int v = 1; v <= troco; v++) { printf(" inf"); }
    printf("\n");

    /*
     * -------------------------------------------------------------------------------
     * PASSO 2: PREENCHIMENTO E EXIBIÇÃO DA TABELA
     * -------------------------------------------------------------------------------
     */
    for (int i = 0; i < n; i++) {
        int valor_moeda = valores[i];
        int peso_moeda = pesos[i];

        for (int v = valor_moeda; v <= troco; v++) {
            if (peso_minimo[v - valor_moeda] != LLONG_MAX &&
                peso_minimo[v] >= peso_moeda + peso_minimo[v - valor_moeda])
            {
                peso_minimo[v] = peso_moeda + peso_minimo[v - valor_moeda];
                ultima_moeda[v] = valor_moeda;
            }
        }

        printf("%d %2d |", valor_moeda, peso_moeda);
        for (int v = 0; v <= troco; v++) {
            if (peso_minimo[v] == LLONG_MAX) {
                printf(" inf");
            } else {
                printf("%4lld", peso_minimo[v]);
            }
        }
        printf("\n");
    }
    printf("-----+");
    for (int v = 0; v <= troco; v++) { printf("----"); }
    printf("\n\n");

    /*
     * -------------------------------------------------------------------------------
     * PASSO 3: APRESENTAÇÃO DOS RESULTADOS
     * -------------------------------------------------------------------------------
     */
    if (peso_minimo[troco] == LLONG_MAX) {
        printf("Nao e possivel dar o troco de %d com as moedas fornecidas.\n", troco);
    } else {
        printf("O peso minimo para o troco de %d e: %lld\n", troco, peso_minimo[troco]);

        int valor_atual = troco;
        while (valor_atual > 0) {
            int moeda_valor = ultima_moeda[valor_atual];
            for (int i = 0; i < n; i++) {
                if (valores[i] == moeda_valor) {
                    contagem_moedas[i]++;
                    break;
                }
            }
            valor_atual -= moeda_valor;
        }

        printf("Moedas utilizadas para a solucao otima:\n");
        for (int i = 0; i < n; i++) {
            if (contagem_moedas[i] > 0) {
                printf("  -> %d x Moeda de valor %d (peso unitario: %d)\n",
                       contagem_moedas[i], valores[i], pesos[i]);
            }
        }
    }

    /*
     * -------------------------------------------------------------------------------
     * LIBERAÇÃO DA MEMÓRIA
     * -------------------------------------------------------------------------------
     */
    free(peso_minimo);
    free(ultima_moeda);
    free(contagem_moedas);
}

/*
 * ===================================================================================
 * FUNÇÃO main 
 * ===================================================================================
 */
int main() {
    int n;     // Variável para guardar o número de tipos de moedas
    int troco; // Variável para guardar o valor do troco

    // --- Coleta de Dados do Usuário ---
    printf("--- Sistema de Troco com Peso Minimo ---\n");

    // 1. Pergunta quantos tipos de moeda existem
    printf("Quantos tipos de moedas voce deseja inserir? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Entrada invalida. Por favor, insira um numero inteiro positivo.\n");
        return 1; // Encerra o programa com código de erro
    }

    // 2. Aloca memória para os arrays de valores e pesos
    int* valores = (int*)malloc(n * sizeof(int));
    int* pesos = (int*)malloc(n * sizeof(int));

    // Verifica se a alocação foi bem-sucedida
    if (valores == NULL || pesos == NULL) {
        printf("Erro ao alocar memoria. O programa sera encerrado.\n");
        free(valores);
        free(pesos);
        return 1;
    }

    // 3. Pede para o usuário inserir o valor e peso de cada moeda
    printf("\nAgora, insira os dados para cada moeda:\n");
    for (int i = 0; i < n; i++) {
        printf("--- Moeda %d ---\n", i + 1);
        printf("Digite o VALOR da moeda: ");
        scanf("%d", &valores[i]);
        printf("Digite o PESO da moeda: ");
        scanf("%d", &pesos[i]);
    }

    // 4. Pergunta qual o valor do troco a ser calculado
    printf("\nQual o valor do troco que voce deseja calcular? ");
    scanf("%d", &troco);
    
    // --- Execução e Exibição ---
    printf("\nCalculando troco otimo (peso minimo) para o valor: %d\n", troco);
    printf("Moedas disponiveis (valor -> peso):\n");
    for(int i = 0; i < n; i++) {
        printf("  %d -> %d\n", valores[i], pesos[i]);
    }
    printf("---------------------------------------------\n");

    // Chama a função principal que faz todo o cálculo e exibição
    encontrarTrocoOtimoComPeso(valores, pesos, n, troco);

    // --- Liberação da Memória ---
    // É crucial liberar a memória que foi alocada dinamicamente
    free(valores);
    free(pesos);

    return 0; // Indica que o programa terminou com sucesso
}
