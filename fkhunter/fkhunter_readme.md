https://github.com/user-attachments/assets/e0c66961-7a1a-4675-83e8-341ec141dd5e


1) Install PostgreSQL and Development Tools, in case it's missing.
   
sudo apt update

sudo apt install -y postgresql-16 postgresql-server-dev-16 build-essential

2) Download and Go to the Extension Directory
   
mkdir fkhunter

cd fkhunter

3) Make and Install the Extension
   
sudo make

sudo make install

4) Add to shared_preload_libraries
   
alter system set shared_preload_libraries=fkhunter;

5) Restart PostgreSQL service and create your new extension.
   
sudo systemctl restart postgresql.service

sudo -u postgres psql -c "CREATE EXTENSION fkhunter;"



