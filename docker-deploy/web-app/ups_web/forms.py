from django import forms
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserCreationForm


class UserRegisterForm(UserCreationForm):
    class Meta:
        model = User
        fields = ['username', 'email', 'password1', 'password2']

class UserUpdateForm(forms.ModelForm):
    email = forms.EmailField()
    class Meta:
        model = User
        fields = ['username', 'email']

class trackingQ(forms.Form):
    tracking_num = forms.IntegerField(label='Please type in your tracking number:', min_value=0)

class updateD(forms.Form):
    dest_x = forms.IntegerField(label='Please update your destination x:', min_value=0)
    dest_y = forms.IntegerField(label='Please update your destination y:', min_value=0)