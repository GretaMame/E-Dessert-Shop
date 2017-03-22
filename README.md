# E-Desserts-Shop
A simple client-server program using TCP/IP sockets.

# How to run it?
On mac or Linux:
1. Open terminal
2. Go to the directory where the files are
3. Compile the files:

    gcc server.c -o server
    
    gcc client.c -o client
4. Run the server:

    ./server 1111
5. Run the client in a separate terminal window(you can run multiple clients at a time):

    ./client 0 1111
