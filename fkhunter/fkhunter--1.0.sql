CREATE FUNCTION check_fk_index() RETURNS event_trigger
    LANGUAGE c
    AS 'MODULE_PATHNAME', 'check_fk_index';

CREATE EVENT TRIGGER fk_check_trigger
    ON ddl_command_start
    WHEN TAG IN ('alter table', 'create table')
    EXECUTE FUNCTION check_fk_index();
