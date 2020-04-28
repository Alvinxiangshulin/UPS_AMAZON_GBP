from django.shortcuts import render, redirect
from django.contrib.auth.forms import UserCreationForm
from django.contrib import messages
from .forms import UserRegisterForm, UserUpdateForm, trackingQ, updateD
from django.core.mail import send_mail, send_mass_mail
import math
# Create your views here.
from .models import packages, items, truck_pool, package_history
from django.template.defaulttags import register

truck_status = {
    0: 'idle',
    1: 'traveling',
    2: 'arrive_warehouse',
    3: 'loading',
    4: 'delivering'
}

package_status = {
    0: 'to_delivery',
    1: 'in transition',
    2: 'arrived'
}


@register.filter(name='lookup')
def lookup(dict, key):
    return dict.get(key)


def register(request):
    if request.method == 'POST':
        form = UserRegisterForm(request.POST)
        if form.is_valid():
            form.save()
            messages.success(request, f'Account created!')
            email = form.cleaned_data.get('email')
            datatuple = (
                ('Welcome!', 'Thanks for regiest UPS', email, [email]),
            )
            send_mass_mail(datatuple);
            return redirect('login')
    else:
        form = UserRegisterForm()
    return render(request, 'users/register.html', {'form': form})


def home(request):
    return render(request, 'homepage/base.html')


def profile(request):
    user = request.user
    return render(request, 'users/profile.html')


def tracking(request):
    if request.method == 'POST':
        form = trackingQ(request.POST)
        if form.is_valid():
            pkgFind = packages.objects.all().filter(package_id=form.cleaned_data['tracking_num'])
            return render(request, 'homepage/trackingResult.html', {'pkgs': pkgFind, 'pkgS': package_status})
    else:
        form = trackingQ()
    return render(request, 'homepage/tracking.html', {'form': form})


def trackResult(request):
    return render(request, 'homepage/trackingResult.html')


def allPackages(request):
    pkgs = packages.objects.all().filter(account=request.user.username)
    return render(request, 'homepage/allPackages.html', {'pkgs': pkgs, 'pkgS': package_status, 'user': request.user})


def itemDetail(request):
    if request.method == 'POST':
        pkgid = int(request.POST['pkgid'])
        iteminPkg = items.objects.all().filter(package_id=pkgid)
        pkg = packages.objects.all().get(package_id=pkgid)
        truckforPkg = truck_pool.objects.all().get(truck_id=pkg.truck_id)
        dist = math.sqrt((pkg.dest_x - truckforPkg.dest_x) * (pkg.dest_x - truckforPkg.dest_x) + (pkg.dest_y - truckforPkg.dest_y) * (pkg.dest_y - truckforPkg.dest_y))
        dist = round(dist, 2)
        pkghistory = package_history.objects.all().get(package_id=pkgid)
        return render(request, 'homepage/itemDetail.html', {'items': iteminPkg, 'truck': truckforPkg, 'pkg': pkg, 'dist': dist, 'history': pkghistory})
    return render(request, 'homepage/itemDetail.html')


def updateDest(request):
    pkgid = int(request.POST['pkgid'])
    if request.method == 'POST':
        form = updateD(request.POST)
        if form.is_valid():
            packageToUpdate = packages.objects.all().get(package_id=pkgid)
            if packageToUpdate.status == 0:
                packageToUpdate.dest_x = form.cleaned_data['dest_x']
                packageToUpdate.dest_y = form.cleaned_data['dest_y']
                packageToUpdate.save()
            return render(request, 'homepage/updateDestResult.html', {'pkg': packageToUpdate})
    else:
        form = updateD()
    return render(request, 'homepage/updateDest.html', {'newDestForm': form, 'pkgid': pkgid})


def updateDestResult(request):
    return render(request, 'homepage/updateDestResult.html')
