#!/bin/bash

#проверка третьего пункта и смотрим логи в оболочке запуска сурикаты
chmod o+rw /tmp/suricata.socket
psql -U postgres -f /src/sql/fun_sql.sql


