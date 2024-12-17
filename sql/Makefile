.PHONY : all
CFLAGS= -Wall  -Wimplicit-function-declaration -Wunused-variable
DEBUG=-g3 # chmod +x gdb_root_VSCODE.sh
BIN =bin
I_DIR= -I$(shell pg_config --includedir) \
		-I$(shell pg_config --includedir)/16/server 
L_DIR= -L$(shell pg_config --libdir)
P_PKG= $(shell pg_config --pkglibdir)
ECPG = ecpg


all : BIN_DIR suricata_module main

BIN_DIR: 
	mkdir -p $(BIN)

main: main.c
	gcc -o $(BIN)/main  $(BIN)/main.c $(DEBUG)  $(CFLAGS) -DDEBUG $(I_DIR) -L${L_DIR}   -lpq -lecpg

main.c: main.pgc
	$(ECPG) -o $(BIN)/main.c main.pgc 

suricata_module: suricata_module.c
	cc -o $(BIN)/suricata_module.o -g3 -c suricata_module.c $(CFLAGS) $(I_DIR) -fPIC 
	cc -o $(BIN)/suricata_module.so $(BIN)/suricata_module.o -shared ${L_DIR} -lpq 

install:
	ln -s /src/sql/$(BIN)/suricata_module.so $(P_PKG)/suricata_module.so
                                                      
# cp suricata_module.so $(P_PKG)
#systemctl restart postgresql.service
	
clean: 
	rm -rf $(BIN)
	rm -f $(BIN)/main $(BIN)/main.c $(BIN)/suricata_module.o $(BIN)/suricata_module.so
	rm -f $(P_PKG)/suricata_module.so
