How many times have you seen a new deployment pipeline break production due to index creation? 
How many incidents have occurred because someone forgot to include the CONCURRENTLY keyword in CREATE INDEX or REINDEX operations? 
The concurrent_enforcer extension helps you avoid lock-related issues on critical production databases by implementing stricter control. It ensures that all index creation and reindexing operations use the CONCURRENTLY keyword, safeguarding your deployments and minimizing downtime.

Demo
![Screenshot 2024-11-16 132408](https://github.com/user-attachments/assets/7e7d57ae-724c-4f20-a446-d66e061fd0eb)





Installation Steps:

#Install PostgreSQL and Development Tools, in case it's missing.

sudo apt update

sudo apt install -y postgresql-16 postgresql-server-dev-16 build-essential

#Download and Go to the Extension Directory

mkdir concurrent_enforcer

cd concurrent_enforcer

#Make and Install the Extension

sudo make

sudo make install

#Add to shared_preload_libraries

alter system set shared_preload_libraries=concurrent_enforcer;

#Restart PostgreSQL service and create your new extension.

sudo systemctl restart postgresql.service

sudo -u postgres psql -c "CREATE EXTENSION concurrent_enforcer;"

