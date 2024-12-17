#include "postgres.h"
#include "executor/spi.h"       
#include "commands/trigger.h"   
#include "utils/rel.h"          
#include <fmgr.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

// #ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
// #endif
#define TTDUMMY_INFINITY	999999
PG_FUNCTION_INFO_V1(insert_rule);
// #define DEBUG 
#if defined(DEBUG)
 	#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
 	#define DEBUG_PRINT(fmt, args...) 
#endif
#define PIPE_NAME "/tmp/suricata.rules"
#define PLENG 8192
#if defined (PLENG)
	#define PIPE_SIZE PLENG
#else
	#define PIPE_SIZE PIPE_BUF 
#endif

#define SOCKET_TARGET "/tmp/suricata.socket"
#define VERSION_MSG "{ \"version\": \"0.1\" }"

bool reload()
{
	
	int ret;
	char buffer[512];

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) 
			ereport(NOTICE, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Can not create socket\n")));

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_TARGET, sizeof(addr.sun_path));
	//strncpy(addr.sun_path, "/home/lunix/suricat/DEBUG/suricata.socket", sizeof(addr.sun_path));
	addr.sun_path[sizeof(addr.sun_path) - 1] = 0;
	ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Connect to socket: %s\n", er)));
	}

	ret = send(sockfd, VERSION_MSG, strlen(VERSION_MSG), 0);
	if (ret == -1) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Can't send error: %s\n", er)));
	}
	else if (ret < strlen(VERSION_MSG)) 
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Error send version\n")));

	memset(buffer, 0, sizeof(buffer));
	ret = read(sockfd, buffer, sizeof(buffer));
	if (ret == -1) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Can't read answer: %s\n", er)));
	}

	DEBUG_PRINT("Buffer: %s.\n", buffer);

	// начинаем отправлять команду 
	memset(buffer, 0, sizeof(buffer));
	ret = snprintf(buffer, sizeof(buffer) - 1, "{ \"command\": \"reload-rules\" }");
	if (ret < 0) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Can't create message: %s\n", er)));
	}

	ret = send(sockfd, buffer, sizeof(buffer), 0);
	if (ret == -1) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Can't send error: %s\n", er)));
	}
	else if (ret < strlen(VERSION_MSG)) 
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Error send version\n")));

	// usleep(1000);
	memset(buffer, 0, sizeof(buffer));
	ret = read(sockfd, buffer, sizeof(buffer));
	if (ret == -1) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Can not read answer: %s\n", er)));
	}

	DEBUG_PRINT("Buffer: %s.\n", buffer);
	return true;
}

void write_rule(char const * rule_str)
{
    // создать PIPE
	// unlink(PIPE_NAME); // закрыть перед созданием и открытием
	if ( mkfifo(PIPE_NAME, 0777)) 
	{
		char * er = strerror(errno);
		ereport(NOTICE, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("create mkfifo error: %s\n", er)));
	}
	
	int fd_fifo_r=0;
	// открыть PIPE r
    if ( (fd_fifo_r = open(PIPE_NAME, O_RDONLY  | O_NONBLOCK, 0777)) <= 0 ) 
	{
		char * er = strerror(errno);
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("open fifo for read error: %s\n", er)));
	}

	// открыть PIPE w
	int fd_fifo_w = 0;
    if ( (fd_fifo_w = open(PIPE_NAME, O_WRONLY, 0777)) <= 0 ) 
	{
		if (errno == ENXIO) 
			ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("open fifo for write error, open the other side for reading\n")));
		else 
		{
			char * er = strerror(errno);
			ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("open fifo for write error : %s\n", er)));
		}
	}

    DEBUG_PRINT("%s is opened\n", PIPE_NAME);

	// отрпавляем правило
	char buf[PIPE_SIZE];
	memset(buf, '\0', PIPE_SIZE);
	memcpy( buf, rule_str, strlen(rule_str) );
	buf[strlen(buf)]='\n';
 	int ret_write = write(fd_fifo_w, rule_str, strlen(rule_str)+1);
	if (ret_write < strlen(rule_str)+1)
	{
		if (errno == ENXIO) 
			ereport(NOTICE, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("WRITE %d byte, open FIFO the other side for reading ", ret_write)));
		else 
		{
			char * er = strerror(errno);
			ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("open fifo for write error : %s\n", er)));
		}
	}

	// команда на обновление
	reload();

	close(fd_fifo_r);	
	close(fd_fifo_w);
}

int GetCurrentTimestamp();

Datum insert_rule(PG_FUNCTION_ARGS)
{
    TriggerData *trigdata = (TriggerData *) fcinfo->context;
	HeapTuple	rettuple = NULL;
	
	if (!CALLED_AS_TRIGGER(fcinfo))
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("ERROR: function was not triggered by trigger")));
	
	if (!TRIGGER_FIRED_FOR_ROW(trigdata->tg_event))
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("ERROR: trigger is not for row")));

	if (TRIGGER_FIRED_BY_INSERT(trigdata->tg_event))
		rettuple = trigdata->tg_trigtuple;
	else if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
		rettuple = trigdata->tg_newtuple;
	else
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("ERROR: trigger for the deletion operation")));

	Relation	rel;			/* triggered relation */
	rel = trigdata->tg_relation;

	TupleDesc	tupdesc;		/* tuple description */
	tupdesc = rel->rd_att;

	int	col_num;
	col_num = SPI_fnumber(tupdesc, "rule"); // номер столбца
	if (col_num == SPI_ERROR_NOATTRIBUTE)
		ereport(ERROR, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("ERROR: field with name %s is missing", "rule")));
	
	// get char* rule
	char *valbuf;
	valbuf = (char*) SPI_getvalue(rettuple, tupdesc, col_num);
	if (valbuf) 
	{
		write_rule(valbuf);
		ereport(NOTICE, (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION), errmsg("Write: %s", valbuf)));
	}
	else 
	{
		bool newnull = false;
		Datum newval;	
		newval = GetCurrentTimestamp();
		rettuple = heap_modify_tuple_by_cols(rettuple, tupdesc, 1, &col_num, &newval, &newnull);		
	}

	return PointerGetDatum(rettuple);
}

