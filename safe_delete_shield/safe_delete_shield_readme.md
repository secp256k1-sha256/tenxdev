Safe Delete Shield: Intercepts DELETEs without WHERE clauses to protect data from accidental full-table wipes.

PS: Truncates are not protected as we aim to prevent only accidental or an ORM spawned unintentional deletion of data.


![Screenshot 2024-11-16 132408](https://github.com/user-attachments/assets/80b975c0-93d8-40b8-9a56-7997eed6e353)

Pre-requisite:
```
#Install PostgreSQL and Development Tools, in case it's missing.
sudo apt update
sudo apt install -y postgresql-17 postgresql-server-dev-17 build-essential
```

Installation:
```
#Download and Go to the Extension Directory
mkdir safe_delete_shield
cd safe_delete_shield

#Make and Install the Extension
sudo make
sudo make install

#Add to shared_preload_libraries
alter system set shared_preload_libraries=safe_delete_shield;

#Restart PostgreSQL service and create your new extension.
sudo systemctl restart postgresql.service
sudo -u postgres psql -c "CREATE EXTENSION safe_delete_shield;"
