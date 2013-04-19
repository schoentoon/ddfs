Dumb Distributed File System
============================

Ddfs works with a master-slave model. The server will simply monitor a folder for new files (using inotify) and write the new files out to the clients. These clients will then simply write out the files.

Due to the server using inotify the server is very Linux only. The client however should be able to compile on other operating systems as well (I didn't test this, feel free to let me know if it works).

To build both the server and the client simply run $ make

To just build the server use
> $ make server

To just build the client use
>$ make client

If you want to build the client for windows use
> $ make windows

in the client folder. To build the client and server without openssl uncomment the #define NO_OPENSSL line in global/defines.h.

To generate the required ssl files use the following commands:
> openssl genrsa -out pkey 2048<br />
> openssl req -new -key pkey -out cert.req<br />
> openssl x509 -req -days 365 -in cert.req -signkey pkey -out cert