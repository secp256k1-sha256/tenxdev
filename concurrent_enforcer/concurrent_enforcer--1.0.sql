DO $$ 
BEGIN
    IF current_setting('shared_preload_libraries') NOT LIKE '%concurrent_enforcer%' THEN
        RAISE EXCEPTION 'concurrent_enforcer must be loaded via shared_preload_libraries. Add concurrent_enforcer to shared_preload_libraries in postgresql.conf and restart PostgreSQL.';
    END IF;
END $$;

