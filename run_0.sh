#!/bin/bash
#������ ���������
pg_ctlcluster 16 main start 

#����� �������� � ������������� ������
suricata -c /suricata.yaml --unix-socket=/tmp/suricata.socket -vvv 