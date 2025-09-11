import socket

# Define server address and port
HOST = '127.0.0.1'  # Localhost
PORT = 65432        # Port to listen on

# Create a socket object
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.bind((HOST, PORT))  # Bind the socket to the address and port
    server_socket.listen()            # Start listening for connections
    print(f"Server is listening on {HOST}:{PORT}")

    while True:
        conn, addr = server_socket.accept()  # Accept a new connection
        with conn:
            print(f"Connected by {addr}")
            while True:
                data = conn.recv(1024)  # Receive data from the client
                if not data:
                    break
                print(f"Received: {data.decode()}")
                conn.sendall(b"OK")  # Send "OK" response