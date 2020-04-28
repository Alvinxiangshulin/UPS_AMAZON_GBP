from django.db import models
from django.contrib.auth.models import User
from django.urls import reverse

# Create your models here.

PACKAGE_STATUS = (
    (0, 'Preparing for delivery'),
    (1, 'In transit'),
    (2, 'Delivered')
)

Truck_STATUS = (
    (0, 'idle'),
    (1, 'traveling'),
    (2, 'arrive warehouse'),
    (3, 'loading'),
    (4, 'delivering')
)


class Profile(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE)
    def __str__(self):
        return f'{self.user.username} Profile'

class truck_pool(models.Model):
    truck_id = models.PositiveIntegerField()
    truck_status = models.PositiveIntegerField()
    dest_x = models.PositiveIntegerField(default=0)
    dest_y = models.PositiveIntegerField(default=0)
    def __str__(self):
        return f'{self.truck_id}, {self.truck_status}'


class packages(models.Model):
    package_id = models.PositiveIntegerField()
    account = models.CharField(max_length=50, null=True)
    dest_x = models.PositiveIntegerField(null=True)
    dest_y = models.PositiveIntegerField(null=True)
    whid = models.PositiveIntegerField()
    truck_id = models.PositiveIntegerField(null=True)
    status = models.PositiveIntegerField()
    def __str__(self):
        return f'{self.package_id},{self.status}'

class items(models.Model):
    package_id = models.PositiveIntegerField()
    product = models.CharField(max_length=50)
    count = models.PositiveIntegerField()
    def __str__(self):
        return f'{self.package_id},{self.product},{self.count}'

class package_history(models.Model):
    package_id = models.PositiveIntegerField()
    topickup = models.CharField(max_length=150, null=True)
    toload = models.CharField(max_length=150, null=True)
    todeliver = models.CharField(max_length=150, null=True)
    delivered = models.CharField(max_length=150, null=True)