#!/bin/bash
#запуск постгреса
pg_ctlcluster 16 main start 

#запук сурикаты в интерактивном режиме
suricata -c /suricata.yaml --unix-socket=/tmp/suricata.socket -vvv 