DEMO

https://github.com/user-attachments/assets/e0c66961-7a1a-4675-83e8-341ec141dd5e



Ideal use case would be in non-production environment to prevent developers from creating schema with unindexed foreign key. It's perfect to be installed on a CI/CD server via seperate script so extension itself does not go on production but prevents unindexed foreign key's from going freely on Prod.



Pre-requisite 
```
***Install PostgreSQL and Development Tools, in case it's missing.***
sudo apt update
sudo apt install -y postgresql-17 postgresql-server-dev-17 build-essential
```

Installation
```
***Download and Go to the Extension Directory***
mkdir fkhunter
cd fkhunter

***Make and Install the Extension***
sudo make
sudo make install

***Add to shared_preload_libraries***
alter system set shared_preload_libraries=fkhunter;

***Restart PostgreSQL service and create your new extension***
sudo systemctl restart postgresql.service
sudo -u postgres psql -c "CREATE EXTENSION fkhunter;"
```


