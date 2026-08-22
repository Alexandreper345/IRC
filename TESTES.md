Tabela só da parte **obrigatória** da [régua do ft_irc](https://www.42evalhub.com/common/ftirc). Bonus (DCC / bot) fica de fora.

Usa porta `6667` e senha `senha42`. O primeiro a criar `#geral` fica operador (`alice`).

Registo base no `nc` (Enter no Mac chega; irssi manda `\r\n`):

```
PASS senha42
NICK alice
USER alice 0 * :Alice
```

| Comandos a digitar | Descrição do teste |
|---|---|
| `make` | Compila com as flags exigidas, em C++, e gera `ircserv`. |
| `grep -n "poll(" src/server/*.cpp include/*.hpp` | Há **um único** `poll()` (ou equivalente) no código. |
| `grep -n "fcntl(" src/server/*.cpp` | Cada `fcntl` é exactamente `fcntl(fd, F_SETFL, O_NONBLOCK)`. Outro uso = 0. |
| Ver no código: `poll` antes de `accept` / `recv` / `send`; nenhum `if (errno == EAGAIN)` para reler/reenviar | `poll()` corre **antes** de cada I/O. Não se usa `errno` para decidir acção. |
| `./ircserv 6667 senha42` | O servidor sobe e escuta em **todas** as interfaces nessa porta. |
| Terminal 2: `nc 127.0.0.1 6667` depois o bloco `PASS` / `NICK` / `USER` acima | Com `nc` ligas, envias comandos e o servidor **responde** (`001 Welcome`). |
| `irssi` → `/connect 127.0.0.1 6667 senha42` → `/nick alice` | Liga com o cliente de referência (dizem irssi na defesa). |
| Manter `nc` + irssi ligados ao mesmo tempo; num: `JOIN #geral` + `PRIVMSG #geral :ola`; no outro: `/join #geral` e falar | Várias ligações em paralelo; o servidor **não bloqueia** e responde a todos. |
| `alice`: `JOIN #geral` `PRIVMSG #geral :ola a todos` — `bob` (outro `nc`, já registado): `JOIN #geral` | Entrar no canal; a mensagem de um cliente chega a **todos** os outros nesse canal. |
| `nc` de `bob`: escrever `PRIVMSG #geral :parcial` **sem Enter**; no `alice` continuar a falar no canal | Comando a meio; o servidor não trava e as outras ligações continuam. Depois Enter e a frase completa deve sair. |
| `kill -9 <pid_do_nc_de_bob>` (ou fechar o irssi à força); `alice` continua; novo `nc` liga e faz `PASS`/`NICK`/`USER` | Matar um cliente de surpresa; o servidor continua e aceita novos. |
| `nc` novo: `PASS senha42` depois `NICK ali` **sem Enter**, e `Ctrl+C` | Matar o `nc` com o comando pela metade; o servidor não fica preso nem num estado estranho. |
| `alice` e `bob` em `#geral`. No `bob`: `Ctrl+Z`. No `alice`: várias vezes `PRIVMSG #geral :flood`. Depois no `bob`: `fg`. Correr `leaks`/`valgrind` no servidor | Cliente parado (`^Z`); flood noutro. O servidor não trava. Ao retomar, processa o pendente. Sem fugas de memória. |
| `nc` **e** irssi: `PASS senha42` `NICK alice` `USER alice 0 * :Alice` `JOIN #geral` (irssi: `/connect … senha42` `/join #geral`) | Autenticar, nick, username e entrar no canal nos dois clientes. |
| `PRIVMSG #geral :msg no canal` | PRIVMSG para **canal**: todos no canal recebem (menos o autor). |
| `PRIVMSG bob :ola privado` | PRIVMSG para **nick**: só o `bob` recebe. |
| `PRIVMSG` e `PRIVMSG bob` (sem texto) | PRIVMSG com parâmetros em falta: erros (`411` / `412`), não crash. |
| `PRIVMSG #naoexiste :x` e `PRIVMSG ghost :x` | Destino inválido: `404` (canal) / `401` (nick). |
| `bob` (não-op) em `#geral`: `KICK #geral alice` `INVITE carol #geral` `TOPIC #geral :hack` `MODE #geral +i` | User normal **não** tem privilégios: `482 You're not channel operator`. |
| `alice` (op): `KICK #geral bob :sai` | Operador **KICK**: `bob` sai do canal. |
| `alice`: `MODE #geral +i` depois `INVITE bob #geral` — `bob`: `JOIN #geral` | Operador **INVITE** (+ modo `i`): só o convidado entra. |
| `alice`: `TOPIC #geral` depois `TOPIC #geral :topico da eval` | Operador **TOPIC**: consultar e definir; o canal vê a mudança. |
| `alice`: `MODE #geral +i` / `MODE #geral -i` | Modo **i** (invite-only): liga e desliga. |
| `alice`: `MODE #geral +t` `TOPIC #geral :so op` — `bob`: `TOPIC #geral :nao` — depois `alice`: `MODE #geral -t` e o `bob` tenta de novo | Modo **t**: com `+t` só op muda tópico; com `-t` o user normal também. |
| `alice`: `MODE #geral +k chave42` — `carol` (novo `nc` registado): `JOIN #geral` (falha) depois `JOIN #geral chave42` — `alice`: `MODE #geral -k` | Modo **k**: JOIN sem chave falha (`475`); com chave entra; `-k` remove. |
| `alice`: `MODE #geral +o bob` depois `MODE #geral -o bob` | Modo **o**: dá e tira operador ao `bob`. |
| `alice`: `MODE #geral +l 2` — `carol`: `JOIN #geral` (falha `471`) — `alice`: `MODE #geral -l` e o `carol` entra | Modo **l**: limite de users; `-l` tira o limite. |

Notas rápidas para a defesa:

- Bónus da régua (file transfer e bot) **não** entram nesta tabela.
- No `USER`, o espaço antes de `:` é obrigatório: `USER alice 0 * :Alice`.
- Quem cria o canal é op; o segundo a dar `JOIN` não é.
- `INVITE` no vosso código é `INVITE <nick> <canal>` (nick primeiro).