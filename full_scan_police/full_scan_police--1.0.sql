-- full_scan_police--1.0.sql

DO $$
BEGIN
    EXECUTE 'CREATE SCHEMA IF NOT EXISTS full_scan_police';
END;
$$;

-- Create the table for hourly offenders
CREATE TABLE IF NOT EXISTS full_scan_police.hourly_stats (
    recorded_at TIMESTAMP DEFAULT now(),
    table_name TEXT UNIQUE,
    table_size NUMERIC,
    seq_scans BIGINT,
    total_time NUMERIC,
    total_executions BIGINT,
    mean_time NUMERIC,
    logical_io NUMERIC,
    offending_query TEXT
);

-- Create the table for daily reports
CREATE TABLE IF NOT EXISTS full_scan_police.daily_reports (
    report_date DATE PRIMARY KEY,
    report_content TEXT -- Store the HTML content
);

-- Function to collect top offenders
CREATE OR REPLACE FUNCTION full_scan_police.collect_offenders() RETURNS VOID AS $$
DECLARE
    db_size NUMERIC;
    top_tables RECORD;
    related_query RECORD;
BEGIN
    -- Calculate the total database size
    SELECT pg_database_size(current_database()) INTO db_size;

    -- Find top 10 full table scans for large tables (≥1% of database size), excluding system tables and schemas
    FOR top_tables IN
        SELECT
            c.relname AS table_name,
            pg_total_relation_size(c.oid) AS table_size,
            seq_scan,
            seq_tup_read
        FROM pg_class c
        JOIN pg_stat_all_tables s ON c.oid = s.relid
        JOIN pg_namespace n ON c.relnamespace = n.oid
        WHERE pg_total_relation_size(c.oid) >= db_size * 0.01
          AND seq_scan > 0
          AND n.nspname NOT IN ('pg_catalog', 'information_schema', 'pg_toast')
          AND n.nspname NOT LIKE 'pg_temp%'
          AND n.nspname NOT LIKE 'pg_toast_temp%'
          AND c.relname NOT LIKE 'pg_%'
        ORDER BY seq_scan DESC
        LIMIT 10
    LOOP
        -- Correlate with a top query from pg_stat_statements
        FOR related_query IN
            SELECT
                query, total_exec_time AS total_time, calls AS total_executions,
                mean_exec_time AS mean_time, shared_blks_hit AS logical_io
            FROM pg_stat_statements
            WHERE query LIKE '%' || top_tables.table_name || '%'
            ORDER BY mean_exec_time DESC
            LIMIT 1
        LOOP
            -- Use UPSERT to insert or update data
            INSERT INTO full_scan_police.hourly_stats (
                table_name, table_size, seq_scans, total_time, total_executions,
                mean_time, logical_io, offending_query, recorded_at
            )
            VALUES (
                top_tables.table_name,
                top_tables.table_size,
                top_tables.seq_scan,
                related_query.total_time,
                related_query.total_executions,
                related_query.mean_time,
                related_query.logical_io,
                related_query.query,
                now()
            )
            ON CONFLICT (table_name) DO UPDATE
            SET
                recorded_at = EXCLUDED.recorded_at,
                table_size = EXCLUDED.table_size,
                seq_scans = EXCLUDED.seq_scans,
                total_time = EXCLUDED.total_time,
                total_executions = EXCLUDED.total_executions,
                mean_time = EXCLUDED.mean_time,
                logical_io = EXCLUDED.logical_io,
                offending_query = EXCLUDED.offending_query
            WHERE (hourly_stats.seq_scans <> EXCLUDED.seq_scans
               OR hourly_stats.total_time <> EXCLUDED.total_time
               OR hourly_stats.total_executions <> EXCLUDED.total_executions
               OR hourly_stats.logical_io <> EXCLUDED.logical_io);
        END LOOP;
    END LOOP;
END;
$$ LANGUAGE plpgsql;

-- Function to generate daily HTML report
CREATE OR REPLACE FUNCTION full_scan_police.generate_daily_report() RETURNS VOID AS $$
DECLARE
    v_report_date DATE := current_date;
    report_content TEXT := '<html><head><title>Daily Offenders Report</title></head><body><h1>Daily Offenders</h1><table border="1"><tr><th>Time</th><th>Table</th><th>Size (bytes)</th><th>Seq Scans</th><th>Total Time</th><th>Executions</th><th>Mean Time</th><th>Logical IO</th><th>Query</th></tr>';
    offender RECORD;
BEGIN
    FOR offender IN
        SELECT * FROM full_scan_police.hourly_stats
        WHERE recorded_at::DATE = v_report_date
    LOOP
        report_content := report_content || format(
            '<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>',
            offender.recorded_at, offender.table_name, offender.table_size, offender.seq_scans,
            offender.total_time, offender.total_executions, offender.mean_time, offender.logical_io,
            offender.offending_query
        );
    END LOOP;

    report_content := report_content || '</table></body></html>';

    INSERT INTO full_scan_police.daily_reports (report_date, report_content)
    VALUES (v_report_date, report_content)
    ON CONFLICT (report_date) DO UPDATE
    SET report_content = excluded.report_content;
END;
$$ LANGUAGE plpgsql;

-- Utility functions to stop or drop hourly jobs
CREATE OR REPLACE FUNCTION full_scan_police.drop_collection_job() RETURNS VOID AS $$
BEGIN
    PERFORM cron.unschedule('collect_offenders');
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION full_scan_police.drop_daily_job() RETURNS VOID AS $$
BEGIN
    PERFORM cron.unschedule('generate_daily');
END;
$$ LANGUAGE plpgsql;



-- Schedule hourly offender collection using pg_cron
SELECT cron.schedule('collect_offenders', '0 * * * *', 'select full_scan_police.collect_offenders();');

-- Schedule daily report generation at midnight using pg_cron
SELECT cron.schedule('generate_daily', '0 0 * * *', 'select full_scan_police.generate_daily_report();');

UPDATE cron.job SET nodename = '' where jobname in ('collect_offenders','generate_daily');
