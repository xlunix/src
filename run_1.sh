#!/bin/bash

#������������� ���� � ���������
psql -U postgres -f /src/sql/fun_sql.sql

#�������� ������� ������
/src/sql/bin/main

