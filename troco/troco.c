#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // Para usar LLONG_MAX

/**
 * @brief Encontra o troco de peso mínimo usando programação dinâmica.
 * @param valores Array com os valores das moedas disponíveis.
 * @param pesos Array com os pesos correspondentes de cada moeda.
 * @param n O número de tipos de moedas disponíveis.
 * @param troco O valor do troco a ser calculado.
 */
void encontrarTrocoOtimoComPeso(int valores[], int pesos[], int n, int troco) {
    // Arrays para a programação dinâmica.
    long long* peso_minimo = (long long*)malloc((troco + 1) * sizeof(long long));
    int* ultima_moeda = (int*)malloc((troco + 1) * sizeof(int));
    int* contagem_moedas = (int*)calloc(n, sizeof(int));

    // Passo 1: Inicialização
    peso_minimo[0] = 0;
    ultima_moeda[0] = 0;
    for (int i = 1; i <= troco; i++) {
        peso_minimo[i] = LLONG_MAX;
        ultima_moeda[i] = -1;
    }

    // Passo 2: Preencher a tabela DP
    for (int v = 1; v <= troco; v++) {
        for (int j = 0; j < n; j++) {
            int valor_moeda = valores[j];
            int peso_moeda = pesos[j];

            if (valor_moeda <= v) {
                if (peso_minimo[v - valor_moeda] != LLONG_MAX && 
                    peso_minimo[v] > peso_moeda + peso_minimo[v - valor_moeda]) {
                    peso_minimo[v] = peso_moeda + peso_minimo[v - valor_moeda];
                    ultima_moeda[v] = valor_moeda;
                }
            }
        }
    }

    // NOVO: Bloco para exibir a tabela de DP gerada
    printf("\n--- Tabela de Programacao Dinamica Gerada ---\n");
    printf(" Valor do Troco -> Peso Minimo Calculado\n");
    printf("-------------------------------------------\n");
    for (int v = 1; v <= troco; v++) {
        // %2d alinha os números de 1 a 99 para uma melhor visualização
        printf("      %2d        ->        ", v);
        if (peso_minimo[v] == LLONG_MAX) {
            printf("Inalcancavel\n");
        } else {
            printf("%lld\n", peso_minimo[v]);
        }
    }
    printf("-------------------------------------------\n\n");


    // Passo 3: Apresentar os resultados finais
    if (peso_minimo[troco] == LLONG_MAX) {
        printf("Nao e possivel dar o troco de %d com as moedas fornecidas.\n", troco);
    } else {
        printf("O peso minimo para o troco de %d e: %lld\n", troco, peso_minimo[troco]);
        
        // Passo 4: Backtracking para encontrar e contar as moedas
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

    // Libera a memória alocada dinamicamente
    free(peso_minimo);
    free(ultima_moeda);
    free(contagem_moedas);
}

int main() {
    // Exemplo de uso:
    int valores[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 25, 50, 100};
    int pesos[] = {1, 5, 3, 8, 4, 2, 7, 6, 3, 4, 2, 7, 5};
    int n = sizeof(valores) / sizeof(valores[0]);
    int troco = 45;

    printf("Calculando troco otimo (peso minimo) para o valor: %d\n", troco);
    printf("Moedas disponiveis (valor -> peso):\n");
    for(int i=0; i<n; i++) {
        printf("  %d -> %d\n", valores[i], pesos[i]);
    }
    printf("---------------------------------------------\n");
    
    encontrarTrocoOtimoComPeso(valores, pesos, n, troco);

    return 0;
}

