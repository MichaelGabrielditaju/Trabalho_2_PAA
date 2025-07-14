// ===================================================================================
// INCLUSÃO DE BIBLIOTECAS PADRÃO
// ===================================================================================
#include <stdio.h>  // Biblioteca para funções de entrada e saída padrão (como printf).
#include <stdlib.h> // Biblioteca para funções gerais, incluindo alocação de memória (malloc, calloc, free).
#include <limits.h> // Biblioteca para acessar constantes que definem os limites dos tipos de dados, como LLONG_MAX.

/*
 * ===================================================================================
 * FUNÇÃO PRINCIPAL: encontrarTrocoOtimoComPeso
 * ===================================================================================
 * @brief  Esta função implementa o algoritmo de programação dinâmica para resolver
 * o problema do troco com peso mínimo.
 *
 * @param valores   Array contendo os valores de cada tipo de moeda (ex: 1, 5, 10).
 * @param pesos     Array contendo os pesos correspondentes de cada tipo de moeda.
 * @param n         O número de tipos diferentes de moedas disponíveis.
 * @param troco     O valor do troco final que desejamos compor.
 */
void encontrarTrocoOtimoComPeso(int valores[], int pesos[], int n, int troco) {
    /*
     * -------------------------------------------------------------------------------
     * ALOCAÇÃO DE MEMÓRIA PARA AS ESTRUTURAS DA PROGRAMAÇÃO DINÂMICA
     * -------------------------------------------------------------------------------
     * Precisamos de 3 arrays auxiliares. O tamanho deles depende do valor do troco,
     * por isso alocamos a memória dinamicamente com malloc/calloc.
     */

    // 'peso_minimo': Armazenará o peso mínimo para formar cada valor de 0 até 'troco'.
    // Ex: peso_minimo[v] = peso mínimo para o valor v.
    // Usamos 'long long' para evitar estouro (overflow) caso a soma dos pesos seja muito grande.
    long long* peso_minimo = (long long*)malloc((troco + 1) * sizeof(long long));

    // 'ultima_moeda': Armazenará qual foi a última moeda adicionada para alcançar o peso
    // mínimo para um determinado valor. Essencial para reconstruir a solução no final.
    // Ex: ultima_moeda[v] = valor da moeda que completou a solução ótima para v.
    int* ultima_moeda = (int*)malloc((troco + 1) * sizeof(int));

    // 'contagem_moedas': Array para contar quantas moedas de cada tipo foram usadas
    // na solução final. Usado apenas no final, para exibir o resultado.
    // 'calloc' é usado aqui para já inicializar todos os contadores com 0.
    int* contagem_moedas = (int*)calloc(n, sizeof(int));


    /*
     * -------------------------------------------------------------------------------
     * PASSO 1: INICIALIZAÇÃO DOS ARRAYS DA DP
     * -------------------------------------------------------------------------------
     * Preparamos os arrays para o cálculo. A inicialização correta é crucial.
     */

    // Condição de base: O peso para um troco de valor 0 é 0 (nenhuma moeda).
    peso_minimo[0] = 0;
    ultima_moeda[0] = 0;

    // Inicializamos o peso para todos os outros valores como "infinito".
    // Isso garante que o primeiro peso válido que calcularmos será menor e o substituirá.
    // LLONG_MAX é o maior valor possível para um 'long long'.
    for (int i = 1; i <= troco; i++) {
        peso_minimo[i] = LLONG_MAX;
        ultima_moeda[i] = -1; // Usamos -1 para indicar que uma solução para este valor ainda não foi encontrada.
    }


    /*
     * -------------------------------------------------------------------------------
     * PASSO 2: PREENCHIMENTO DA TABELA (O CORAÇÃO DO ALGORITMO)
     * -------------------------------------------------------------------------------
     * Este é o loop principal da programação dinâmica, que constrói a solução
     * de baixo para cima (bottom-up), resolvendo subproblemas menores primeiro.
     */

    // Loop externo: itera sobre cada valor de troco possível, de 1 até o valor final.
    for (int v = 1; v <= troco; v++) {
        // Loop interno: para o valor 'v' atual, testamos cada tipo de moeda disponível.
        for (int j = 0; j < n; j++) {
            int valor_moeda = valores[j]; // Pega o valor da moeda atual.
            int peso_moeda = pesos[j];   // Pega o peso da moeda atual.

            // Verificamos se a moeda atual pode ser usada para formar o valor 'v'.
            if (valor_moeda <= v) {
                // Esta é a verificação principal da programação dinâmica.
                // 1. `peso_minimo[v - valor_moeda] != LLONG_MAX`: Garante que o subproblema (para o valor restante) JÁ TEM uma solução.
                // 2. `peso_minimo[v] > ...`: Compara o peso mínimo atual de 'v' com um novo peso possível
                //    (o peso da moeda atual + o peso mínimo já conhecido para o valor restante).
                if (peso_minimo[v - valor_moeda] != LLONG_MAX &&
                    peso_minimo[v] > peso_moeda + peso_minimo[v - valor_moeda])
                {
                    // Se encontramos um caminho com peso menor, atualizamos nossos arrays.
                    peso_minimo[v] = peso_moeda + peso_minimo[v - valor_moeda]; // Atualiza o peso mínimo para 'v'.
                    ultima_moeda[v] = valor_moeda;                               // Registra que esta moeda gerou a nova solução ótima.
                }
            }
        }
    }


    /*
     * -------------------------------------------------------------------------------
     * EXIBIÇÃO DA TABELA DE PROGRAMAÇÃO DINÂMICA
     * -------------------------------------------------------------------------------
     * Mostra os resultados dos subproblemas, o que ajuda a visualizar o processo.
     */
    printf("\n--- Tabela de Programacao Dinamica Gerada ---\n");
    printf(" Valor do Troco -> Peso Minimo Calculado\n");
    printf("-------------------------------------------\n");
    // Itera de 1 até o troco para imprimir cada entrada da tabela 'peso_minimo'.
    for (int v = 1; v <= troco; v++) {
        printf("      %2d        -> ", v);
        // Se o peso continuou "infinito", significa que o valor é inalcançável.
        if (peso_minimo[v] == LLONG_MAX) {
            printf("Inalcancavel\n");
        } else {
            // Caso contrário, imprime o peso mínimo encontrado.
            printf("%lld\n", peso_minimo[v]);
        }
    }
    printf("-------------------------------------------\n\n");


    /*
     * -------------------------------------------------------------------------------
     * PASSO 3 & 4: APRESENTAÇÃO DOS RESULTADOS E RECONSTRUÇÃO DA SOLUÇÃO
     * -------------------------------------------------------------------------------
     */

    // Primeiro, verificamos se foi possível encontrar uma solução para o troco final.
    if (peso_minimo[troco] == LLONG_MAX) {
        printf("Nao e possivel dar o troco de %d com as moedas fornecidas.\n", troco);
    } else {
        // Se uma solução foi encontrada, exibimos o peso mínimo.
        printf("O peso minimo para o troco de %d e: %lld\n", troco, peso_minimo[troco]);

        // Agora, reconstruímos a combinação de moedas usada (processo de backtracking).
        // Começamos do valor total do troco e vamos "voltando".
        int valor_atual = troco;
        while (valor_atual > 0) {
            // Pega o valor da última moeda usada para formar 'valor_atual'.
            int moeda_valor = ultima_moeda[valor_atual];

            // Encontra o índice dessa moeda para podermos incrementar sua contagem.
            for (int i = 0; i < n; i++) {
                if (valores[i] == moeda_valor) {
                    contagem_moedas[i]++; // Incrementa a contagem para este tipo de moeda.
                    break;                // Para o loop interno, pois já encontramos a moeda.
                }
            }
            // Subtrai o valor da moeda para continuar o processo com o valor restante.
            valor_atual -= moeda_valor;
        }

        // Finalmente, exibimos a contagem de cada moeda usada na solução ótima.
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
     * Uma boa prática em C: sempre liberar a memória que foi alocada dinamicamente
     * para evitar vazamentos de memória (memory leaks).
     */
    free(peso_minimo);
    free(ultima_moeda);
    free(contagem_moedas);
}


/*
 * ===================================================================================
 * FUNÇÃO main
 * ===================================================================================
 * Ponto de entrada do programa. Define os dados do problema e chama a função
 * principal para resolvê-lo.
 */
int main() {
    // --- Definição do Problema ---
    // Você pode alterar estes arrays e a variável 'troco' para testar outros casos.
    int valores[] = {1, 5, 10, 25, 50, 100}; // Valores de moedas em centavos (ex: R$ 1,00 = 100)
    int pesos[] =   {1, 3, 2,  5,  7,  4};   // Pesos hipotéticos para cada moeda
    
    // Calcula automaticamente o número de tipos de moedas.
    int n = sizeof(valores) / sizeof(valores[0]);

    int troco = 45; // O valor do troco que queremos encontrar.

    // --- Execução e Exibição ---
    printf("Calculando troco otimo (peso minimo) para o valor: %d\n", troco);
    printf("Moedas disponiveis (valor -> peso):\n");
    // Imprime as moedas e pesos disponíveis para o usuário saber as condições.
    for(int i=0; i<n; i++) {
        printf("  %d -> %d\n", valores[i], pesos[i]);
    }
    printf("---------------------------------------------\n");

    // Chama a função que resolve o problema.
    encontrarTrocoOtimoComPeso(valores, pesos, n, troco);

    // Retorna 0 para indicar que o programa foi executado com sucesso.
    return 0;
}