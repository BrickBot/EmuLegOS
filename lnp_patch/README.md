Emulegos currently uses lnp by compiling in the lnp lib source files directly.
The files require a small patch to compile under g++ rather then gcc.  Simply
apply the supplied patch file to the stub.c file in your lnp/liblnp directory.
The modified stub.c file is still compatible with gcc and can be used to build
lnp if needed

Also you will need to patch the lnp/lnpd/daemon.c with daemon.c.patch.
This allows for communication between pc clients, and thus between multiple
virtual RCXs.  As with the stub.c modification LNPD will continue to function
normally, can the modified version can be used all the time

$ patch stub.c stub.c.patch
$ patch daemon.c daemon.c.patch
