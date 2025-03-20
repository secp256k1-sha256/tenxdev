DEMO (On Ubuntu)

https://github.com/user-attachments/assets/e0c66961-7a1a-4675-83e8-341ec141dd5e



Ideal use case would be in non-production environment to prevent developers from creating schema with unindexed foreign key. It's perfect to be installed on a CI/CD server via seperate script so extension itself does not go on production but prevents unindexed foreign key's from going freely on Prod.



Pre-requisite 
```
***Install PostgreSQL and Development Tools, initdb, start Postgres and check pg_config is correct ***
sudo apt update
sudo apt install -y postgresql-17 postgresql-server-dev-17 build-essential
```

Installation
```
***Git clone code and Go to the Extension Directory***
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

***Works as expected on Rocky Linux 9.5 (Blue Onyx)***

![success2_rockylinux_pg17](https://github.com/user-attachments/assets/72468643-329b-46b3-bbd6-bc0075c90ca1)

![success3_rockylinux_pg17](https://github.com/user-attachments/assets/15219828-6c75-475a-9db4-901c5ac498a1)
