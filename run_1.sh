#!/bin/bash

#инициализация базы с правилами
psql -U postgres -f /src/sql/fun_sql.sql

#проверка второго пункта
/src/sql/bin/main

