#Install PostgreSQL and Development Tools, in case it's missing.
sudo apt update
sudo apt install -y postgresql-16 postgresql-server-dev-16 build-essential

#Create the Extension Directory
mkdir fkhunter
cd fkhunter

#Compile and Install the Extension
make
sudo make install

#Add to shared_preload_libraries 
alter system set shared_preload_libraries=fkhunter;

#Restart PostgreSQL service and create your new extension.
sudo systemctl restart postgresql.service

sudo -u postgres psql -c "CREATE EXTENSION fkhunter;"
