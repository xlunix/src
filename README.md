1. выполнить команду "docker build -t dockerfile <span style="color:red"><b>.</b></span>"
2. выполнить скрипт "<span style="color:green">/run_0.sh</span>" в терминале 1;
3. выполнить скрипт "<span style="color:green">/run_1.sh</span>" в терминале 2;
4. выполнить скрипт "<span style="color:green">/run_2.sh</span>" в терминале 3;

каждый раз когда суриката читает правило она его парсит, и если не получается, то в терминале 1 будет что то вроже <span style="color:red">"Error: <span style="color:green">detect-parse</span>: An invalid action "!alert" was given....."</span>

