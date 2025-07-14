from PIL import Image

TAMANHO_TABULEIRO = 12
MAX_SOLUCOES = 5
BLOCO_PREENCHIDO = (0, 0, 0)      # Preto
BLOCO_VAZIO = (255, 255, 255)     # Branco
PIXELS_POR_CELULA = 20            # Tamanho da célula no PNG

solucoes = []

def verifica_bloco_canto(tabuleiro, r, c):
    if r + 1 >= TAMANHO_TABULEIRO or c + 1 >= TAMANHO_TABULEIRO:
        return False
    return (tabuleiro[r][c] == 1 and tabuleiro[r][c+1] == 1 and
            tabuleiro[r+1][c] == 1 and tabuleiro[r+1][c+1] == 1)

def conta_linhas_colunas(tabuleiro):
    linhas = [sum(linha) for linha in tabuleiro]
    colunas = [sum(tabuleiro[r][c] for r in range(TAMANHO_TABULEIRO)) for c in range(TAMANHO_TABULEIRO)]
    return linhas, colunas

def encontra_subregioes_tipo1(tabuleiro):
    regioes = []
    for r in range(TAMANHO_TABULEIRO - 1):
        for c in range(TAMANHO_TABULEIRO - 1):
            if (tabuleiro[r][c] == 1 and tabuleiro[r][c+1] == 0 and
                tabuleiro[r+1][c] == 1 and tabuleiro[r+1][c+1] == 1):
                regioes.append((r, c))
    return regioes

def encontra_subregioes_tipo2(tabuleiro):
    regioes = []
    for r in range(TAMANHO_TABULEIRO - 1):
        for c in range(TAMANHO_TABULEIRO - 1):
            if (tabuleiro[r][c] == 1 and tabuleiro[r][c+1] == 1 and
                tabuleiro[r+1][c] == 0 and tabuleiro[r+1][c+1] == 1):
                regioes.append((r, c))
    return regioes

def verifica_subtabuleiros_distintos(regioes):
    vistos = set()
    for r, c in regioes:
        id_subtabuleiro = (r // 3, c // 3)
        vistos.add(id_subtabuleiro)
    return len(vistos) >= 2

def tabuleiro_valido(tabuleiro):
    cantos = [(0,0),(0,10),(10,0),(10,10)]
    count_cantos = sum(verifica_bloco_canto(tabuleiro, r, c) for r,c in cantos)
    if count_cantos != 3:
        return False

    linhas, colunas = conta_linhas_colunas(tabuleiro)
    if any(x < 5 for x in linhas) or any(x < 5 for x in colunas):
        return False

    tipo1 = encontra_subregioes_tipo1(tabuleiro)
    tipo2 = encontra_subregioes_tipo2(tabuleiro)

    if len(tipo1) < 2 or len(tipo2) < 2:
        return False

    if not verifica_subtabuleiros_distintos(tipo1) or not verifica_subtabuleiros_distintos(tipo2):
        return False

    return True

def imprimir_tabuleiro(tabuleiro):
    for linha in tabuleiro:
        print(''.join('██' if celula == 1 else '  ' for celula in linha))

def salvar_txt(tabuleiro, indice):
    nome_arquivo = f'qr_{indice+1}.txt'
    with open(nome_arquivo, 'w', encoding='utf-8') as f:
        for linha in tabuleiro:
            f.write(''.join('██' if celula == 1 else '  ' for celula in linha) + '\n')
    print(f'QR salvo em {nome_arquivo}')

def salvar_png(tabuleiro, indice):
    tamanho_imagem = TAMANHO_TABULEIRO * PIXELS_POR_CELULA
    imagem = Image.new('RGB', (tamanho_imagem, tamanho_imagem), color=BLOCO_VAZIO)
    
    for r in range(TAMANHO_TABULEIRO):
        for c in range(TAMANHO_TABULEIRO):
            cor = BLOCO_PREENCHIDO if tabuleiro[r][c] == 1 else BLOCO_VAZIO
            for i in range(PIXELS_POR_CELULA):
                for j in range(PIXELS_POR_CELULA):
                    imagem.putpixel((c * PIXELS_POR_CELULA + j, r * PIXELS_POR_CELULA + i), cor)

    nome_arquivo = f'qr_{indice+1}.png'
    imagem.save(nome_arquivo)
    print(f'QR salvo em {nome_arquivo}')

def backtracking(tabuleiro, linha=0, coluna=0):
    if len(solucoes) >= MAX_SOLUCOES:
        return

    if linha == TAMANHO_TABULEIRO:
        if tabuleiro_valido(tabuleiro):
            copia = [linha[:] for linha in tabuleiro]
            solucoes.append(copia)
        return

    proxima_linha = linha + (coluna + 1) // TAMANHO_TABULEIRO
    proxima_coluna = (coluna + 1) % TAMANHO_TABULEIRO

    # Tenta vazio
    tabuleiro[linha][coluna] = 0
    backtracking(tabuleiro, proxima_linha, proxima_coluna)

    # Tenta cheio
    tabuleiro[linha][coluna] = 1
    backtracking(tabuleiro, proxima_linha, proxima_coluna)

    # Reseta (não obrigatória, só para clareza)
    tabuleiro[linha][coluna] = 0

def main():
    tabuleiro = [[0]*TAMANHO_TABULEIRO for _ in range(TAMANHO_TABULEIRO)]
    backtracking(tabuleiro)

    if solucoes:
        for i, sol in enumerate(solucoes):
            print(f'\nCódigo QR {i+1}:')
            imprimir_tabuleiro(sol)
            salvar_txt(sol, i)
            salvar_png(sol, i)
    else:
        print("Nenhum QR Code válido encontrado.")

if __name__ == "__main__":
    main()
