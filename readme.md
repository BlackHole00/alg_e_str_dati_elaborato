# Elaborato di Algoritmi e Strutture Dati Laboratorio
TODO

# Building
Il progetto usa il linguaggio `C99` ed usa il sistema di build `CMake`. Il progetto richiede quindi di aver installato ed aggiunto al path i seguenti programmi:
- un compilatore `C` (`gcc`/`clang`/`msvc`)
- `CMake`
- `make` o `ninja` (consigliato) su macchine unix-like
  

E'stato inoltre implementato il debugging attraverso l'editor di testo `VSCode`. Si nota tuttavia che questa funzionalita'e'attualmente funzionante solo so S.O. unix-like e toolchain `lldb`. Questo verra'risolto nel futuro.
Seguono i programmi da avere installati ed aggiunti al path:
- `lldb`

Si nota subito che molti ide (quali `CLion` e `Visual Studio`) possono direttamente aprire progetti cmake, quindi i seguenti passaggi potrebbero essere non necessari se si ha uno di tali ide installato.

Seguono i comandi da eseguire per buildare il progetto (si nota che il file eseguibile viene posto in una locazione diversa a seconda della piattaforma):
```sh
mkdir build
cd build
cmake ..
cmake --build .
```

Se si vuole utilizzare le funzionalita'di debug usare i seguenti comandi:
```sh
mkdir build
cd build
cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
```
O, preferibilmente se `Ninja` e'installato:
```sh
mkdir build
cd build
cmake -G"Ninja Multi-Config" ..
```
Allora sara'possibile utilizzare le configurazioni di debug in `VSCode` a seconda di quale sistema di build (`make` o `ninja`) e'stato utilizzato.

