DO $$ 
BEGIN
    IF current_setting('shared_preload_libraries') NOT LIKE '%safe_delete_shield%' THEN
        RAISE EXCEPTION 'safe_delete_shield must be loaded via shared_preload_libraries. Add safe_delete_shield to shared_preload_libraries in postgresql.conf and restart PostgreSQL.';
    END IF;
END $$;

