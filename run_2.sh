#!/bin/bash

#�������� �������� ������ � ������� ���� � �������� ������� ��������
chmod o+rw /tmp/suricata.socket
psql -U postgres -f /src/sql/fun_sql.sql

