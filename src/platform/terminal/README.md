# FastStack-Term

This is a frontend that strictly utilizes a text-based environment for
output. It should require the least dependencies of any version.

Platform specific code is likely to be prevalent here, and portability across
platforms is not guaranteed.

## Running

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

You can now run this program without root permission. Note, if you want to
in the future remove yourself from the input group use the following:

```
$ sudo gpasswd -d user input
Removing user user from group input
```

## Linux Specific

## Input

The linux specific engine utilizes direct access to the input device. This
allows us to not be dependent on any graphic server running and as such gives
an opportunity to play directly on the framebuffer if wanted.

## Graphics

We also assume the prescence of a VT100-compatible terminal. This is required
so we can draw with more freedom to the screen. We do not use ncurses since
it strictly isn't required. Whilst it does provide better support across
legacy screens, these are not really targeted anyway.

This IS portable across most different POSIX systems. This will not work on
Windows, but who wants to play in a terminal in Windows? If so, you can always
use Cygwin.

### Sound

Sound is not a required component, however, if `asoundlib` is present on the
machine we do support rudimentary sounds.

## Other

For other systems, we require code to talk to input, and also sound, if wanted.
We suggest however an alternative method where an X-Server is running, and
instead utilizing the X11 environment to query keystate of the specified terminal.
This has a number of benefits (windows focus managed correctly) but is slightly
hard to do (from experience).
