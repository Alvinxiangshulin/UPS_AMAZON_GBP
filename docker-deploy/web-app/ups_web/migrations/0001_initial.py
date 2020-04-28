# Generated by Django 3.0.5 on 2020-04-24 17:10

from django.conf import settings
from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    initial = True

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
    ]

    operations = [
        migrations.CreateModel(
            name='items',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('package_id', models.PositiveIntegerField()),
                ('product', models.CharField(max_length=50)),
                ('count', models.PositiveIntegerField()),
            ],
        ),
        migrations.CreateModel(
            name='package_history',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('package_id', models.PositiveIntegerField()),
                ('topickup', models.CharField(max_length=150, null=True)),
                ('toload', models.CharField(max_length=150, null=True)),
                ('todeliver', models.CharField(max_length=150, null=True)),
                ('delivered', models.CharField(max_length=150, null=True)),
            ],
        ),
        migrations.CreateModel(
            name='packages',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('package_id', models.PositiveIntegerField()),
                ('account', models.CharField(max_length=50, null=True)),
                ('dest_x', models.PositiveIntegerField(null=True)),
                ('dest_y', models.PositiveIntegerField(null=True)),
                ('whid', models.PositiveIntegerField()),
                ('truck_id', models.PositiveIntegerField(null=True)),
                ('status', models.PositiveIntegerField()),
            ],
        ),
        migrations.CreateModel(
            name='truck_pool',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('truck_id', models.PositiveIntegerField()),
                ('truck_status', models.PositiveIntegerField()),
                ('dest_x', models.PositiveIntegerField(default=0)),
                ('dest_y', models.PositiveIntegerField(default=0)),
            ],
        ),
        migrations.CreateModel(
            name='Profile',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('user', models.OneToOneField(on_delete=django.db.models.deletion.CASCADE, to=settings.AUTH_USER_MODEL)),
            ],
        ),
    ]