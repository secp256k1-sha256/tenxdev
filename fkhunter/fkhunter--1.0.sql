DO $$ 
BEGIN
    IF current_setting('shared_preload_libraries') NOT LIKE '%fkhunter%' THEN
        RAISE EXCEPTION 'fkhunter must be loaded via shared_preload_libraries. Add fkhunter to shared_preload_libraries in postgresql.conf and restart PostgreSQL.';
    END IF;
END $$;
