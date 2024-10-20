#Install PostgreSQL and Development Tools, in case it's missing.
sudo apt update
sudo apt install -y postgresql-16 postgresql-server-dev-16 build-essential

#Create the Extension Directory
mkdir fkhunter
cd fkhunter

#Compile and Install the Extension
make
sudo make install

#Load and Test the Extension
#Start PostgreSQL service and load your new extension.
sudo systemctl start postgresql
sudo -u postgres psql -c "CREATE EXTENSION fkhunter;"
