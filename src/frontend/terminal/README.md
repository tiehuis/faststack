FastStack-Term
==============

<a href="https://asciinema.org/a/5odjv7x273oonbmhq51sfbf4l"><img src="https://asciinema.org/a/5odjv7x273oonbmhq51sfbf4l.png" width="600"/></a>

This is a frontend that strictly utilizes a text-based environment for
output. It should require the least dependencies of any version.

Platform specific code is likely to be prevalent here, and portability across
platforms is not guaranteed.

Installation
------------

```
git clone https://github.com/tiehuis/FastStack
cd FastStack
make terminal
```

Running
-------

This requires root permission to read from the input device. To avoid requring
root permissions for every task, you can add yourself to the required input
group once so you can run with user permissions in the future.

Determine the group which the event devices are part of (typically `input`):

```
$ ls -l /dev/input | grep event | head -n5
crw-rw---- 1 root input 13, 64 Jul 31 09:46 event0
crw-rw---- 1 root input 13, 65 Jul 31 09:46 event1
crw-rw---- 1 root input 13, 74 Jul 31 09:46 event10
crw-rw---- 1 root input 13, 75 Jul 31 09:46 event11
crw-rw---- 1 root input 13, 76 Jul 31 09:46 event12
```

Determine your user name:

```
$ id -un
user
```

Add yourself to the previously discovered group:

```
$ sudo gpasswd -a user input
Adding user user to group input
```

Log out and back in (or reboot) for the changes to take effect.

You can now run this program without root permission. Note, if you want to
in the future remove yourself from the input group use the following:

```
$ sudo gpasswd -d user input
Removing user user from group input
```
