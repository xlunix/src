DROP DATABASE IF EXISTS suricata_db; 
DROP TABLE IF EXISTS maintable;

CREATE DATABASE suricata_db; 
\c suricata_db
CREATE TABLE maintable (
    rule_id bigint GENERATED ALWAYS AS IDENTITY,
    rule varchar(5000) NOT NULL
);


-- загрузка модуля
-- LOAD '/usr/lib/postgresql/16/lib/suricata_module';
LOAD 'suricata_module.so';

-- удалить тригер если есть
DROP TRIGGER IF EXISTS insert_rule ON maintable ;

-- загружаю функцию из модуля
CREATE OR REPLACE FUNCTION  insert_rule() RETURNS trigger
    AS 'suricata_module.so', 'insert_rule'
    LANGUAGE C;

-- связываю функцию с тригером
CREATE OR REPLACE TRIGGER insert_rule 
    AFTER INSERT OR UPDATE ON maintable
    FOR EACH ROW 
    EXECUTE PROCEDURE 
    insert_rule();

INSERT INTO maintable (rule) VALUES ('sdalert ip any any -> any any (msg:"SURICATA Applayer Mismatch protocol both directions"; flow:established; app-layer-event:applayer_mismatch_protocol_both_directions; flowint:applayer.anomaly.count,+,1; classtype:protocol-command-decode; sid:2260000; rev:1;)') ;
INSERT INTO maintable (rule) VALUES ('!alert ip any any -> any any (msg:"SURICATA Applayer Detect protocol only one direction"; flow:established; app-layer-event:applayer_detect_protocol_only_one_direction; flowint:applayer.anomaly.count,+,1; classtype:protocol-command-decode; sid:2260002; rev:1;)');
INSERT INTO maintable (rule) VALUES ('alert ip any any -> any any (msg:"SURICATA Applayer Protocol detection skipped"; flow:established; app-layer-event:applayer_proto_detection_skipped; flowint:applayer.anomaly.count,+,1; classtype:protocol-command-decode; sid:2260003; rev:1;)');
INSERT INTO maintable (rule) VALUES (' # alert if STARTTLS was not followed by actual SSL/TLS ');
INSERT INTO maintable (rule) VALUES ('alert tcp any any -> any any (msg:"SURICATA Applayer No TLS after STARTTLS"; flow:established; app-layer-event:applayer_no_tls_after_starttls; flowint:applayer.anomaly.count,+,1; classtype:protocol-command-decode; sid:2260004; rev:2;)');
INSERT INTO maintable (rule) VALUES ('alert ip any any -> any any (msg:"SURICATA Applayer Mismatch protocol both directions"; flow:established; app-layer-event:applayer_mismatch_protocol_both_directions; flowint:applayer.anomaly.count,+,1; classtype:protocol-command-decode; sid:2260000; rev:1;)');   
INSERT INTO maintable (rule) VALUES ('alert ip any any -> any any (msg:"SURICATA Applayer Mismatch protocol both directions"; flow:established; app-layer-event:applayer_mismatch_protocol_both_directions; flowint:applayer.anomaly.count,+,1; classtype:protocol-command-decode; sid:2260000; rev:1;)');
-- ДЛЯ ОТЛАДКИ
    
    -- посмотреть созданные тригеры
    -- select relname as table_with_trigger from pg_class where pg_class.oid in ( select tgrelid from pg_trigger );

    -- посмотреть PID для gdb ,  
    -- $ sudo -u postgres psql -c "SELECT pg_backend_pid();"
    