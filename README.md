Before you start our backend, please do the following in docker-deploy/web-app:

pip3 install django-crispy-forms

python3 manage.py makemigrations

python3 manage.py migrate

To open the frontend:

python3 manage.py runserver 0.0.0.0:8000

Before you start our backend, please do the following in docker-deploy/backend:

change the ip address and port number(should be 12345) of world_simulator_exec in runups.sh

change the ip address and port number pf amazon side in runups.sh


After modified configuration, run sudo docker-compose build, then sudo docker-compose up

