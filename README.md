# Proxy com Cache

Abra um terminal no sistema operacional linux:

Primeiro Passo:
```sh
$ gcc proxy_cache.c -o proxy 
```

Segundo Passo:
Nesse passo é necessário informar a porta que será utilizada.
```sh
$ ./proxy 10000
```

Configuração do Navegador Firefox:
```
    1) Clicar em Preferências
    
    2) Procurar por Network Proxy
    
    3) Selecionar configuração
    
    4) Escolher a proxy de forma manual
    
    5) HTTP Proxy = localhost
       Port = tem que informar a porta que foi selecionada para executar o script.
    
    6) Clique em OK.
```
Durante a execução, é necessário acessar o navegador escolhido e digitar os termos ou URLs desejadas,
para conferir as filtragens e armazenamento em cache. Alguns logs serão transmitidos para facilitar
o acompanhamento.

Vale ressaltar que o proxy desenvolvido não abrange sites HTTPS, apenas HTTP.
