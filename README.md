# Monitor de Processos Linux (Estilo top)
Este projeto é um monitor de processos implementado em C, semelhante ao comando top do Linux. Ele utiliza informações extraídas do sistema de arquivos /proc para exibir dados como PID, uso de CPU e memória, tempo de execução, e outros atributos de processos ativos no sistema.

## Funcionalidades
Exibição em tempo real (atualizada a cada 5 segundos) de processos com:

- PID
- Usuário proprietário
- Prioridade e nice
- Memória virtual (VIRT), residente (RES) e compartilhada (SHR)
- Estado atual (S)
- Uso de CPU e memória (%)
- Tempo total de execução
- Nome do processo
- Leitura e formatação de arquivos /proc/[pid]/stat e /proc/[pid]/smaps
- Utilização de threads para cálculo simultâneo do uso de CPU
- Leitura segura com tratamento de erros

## Extensões Futuras
- Controle interativo de processos (já iniciado, mas comentado no código):
    - Finalizar processo ao pressionar k
    - Encerrar o monitor com q
- Melhor formatação da saída
- Exportação para arquivo

## Pré-requisitos
- Compilador C (como gcc)
- Sistema Linux com acesso ao diretório /proc

## Compilação
O projeto utiliza um Makefile para facilitar a compilação. Para compilar o projeto, utilize o seguinte comando no terminal:

```bash
make
```

## Execução
Após a compilação, execute o programa com:
```bash
sudo ./monitor
```

## Observações
- Algumas funcionalidades estão em desenvolvimento e podem estar comentadas no código.
- O programa requer permissões de leitura em /proc e /etc/passwd.