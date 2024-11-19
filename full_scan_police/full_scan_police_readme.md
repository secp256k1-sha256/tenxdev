Full Scan Police: Generates a daily HTML report highlighting the top 10 large tables with sequential scans, along with their correlated SQL queries.

Prerequisite
```bash
#Install PostgreSQL and Development Tools, in case it's missing.
sudo apt update
sudo apt install -y postgresql-16 postgresql-server-dev-16 build-essential

#pg_stat_statements must be installed in the same database.
#pg_cron must be installed and cron.database should be set to same database name (as the one where extension is needed) in postgres config file. 
```

Installation
```bash
#Download and Go to the Extension Directory
mkdir full_scan_police
cd full_scan_police

#Make and Install the Extension
sudo make install

#Connect to the database and create extension
CREATE EXTENSION full_scan_police;

```

HTML Report : Get report content from below query everyday (runs at midnight by default) and paste it in an html file.

```sql
select report_date,report_content
from full_scan_police.daily_reports;
```



![FSP_Screenshot](https://github.com/user-attachments/assets/b59eae86-4784-4f73-8906-31e61036105a)
