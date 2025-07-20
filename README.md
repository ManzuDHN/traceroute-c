# Traceroute in C

## Descrizione
Questo progetto è una semplice implementazione del comando `traceroute` su sistemi Linux.  
Il programma invia pacchetti UDP con TTL variabile per mappare il percorso (hop) verso una destinazione IP o hostname specificata, ricevendo messaggi ICMP di risposta per identificare i router intermedi.  

## Caratteristiche
- Invio di pacchetti UDP su porte alte (BASE_PORT + TTL)
- Ricezione di messaggi ICMP "Time Exceeded" per scoprire router intermedi
- Calcolo del Round Trip Time (RTT) per ogni hop
- Limite massimo di 30 hop
- Supporto IPv4
- Uso di socket raw per ICMP, richiede permessi di root
- Ambiente di sviluppo: Visual Studio Code (VSCode)
- Compilazione con gcc
- Testato su Fedora 42 e Ubuntu WSL

## Limiti e considerazioni
- Non supporta IPv6
- Potrebbe non funzionare verso destinazioni che bloccano pacchetti UDP verso porte alte
- Il tempo massimo di attesa per risposta è impostato a 2 secondi
- Se il RTT supera 500 ms viene segnalato un possibile problema di rete
- Alcuni router potrebbero non rispondere, mostrando asterischi nel risultato

## Requisiti
- Linux o ambiente compatibile con socket raw (ad esempio WSL su Windows)
- Permessi di root per eseguire il programma
- gcc per compilare

## Come compilare
```bash
gcc -o traceroute traceroute.c utils.c
