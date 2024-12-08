import tkinter as tk
from pynput import keyboard
from threading import Thread
import requests
import socket

UDP_IP = "192.168.10.151" # The IP that is printed in the serial monitor from the ESP32
SHARED_UDP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Internet  # UDP
sock.connect((UDP_IP, SHARED_UDP_PORT))

# Function to handle key press event
def on_key_press(event):
    key = event.keysym.upper()  # Get the key pressed and convert to uppercase
    print(f"{key} pressed")
    sock.send(key.encode())
    if key in key_labels:
        key_labels[key].config(bg="black")  # Change the label background to black (lit up)

# Function to handle key release event
def on_key_release(event):
    key = event.keysym.upper()
    print(f"{key} released")
    sock.send('B'.encode())
    if key in key_labels:
        key_labels[key].config(bg="SystemButtonFace")  # Change the label background back to default

# Function to listen for incoming messages from ESP32
def listen_for_messages():
    while True:
        try:
            data, _ = sock.recvfrom(1024)  # Buffer size is 1024 bytes
            message = data.decode('utf-8')
            # Update the UI with the message from ESP32
            root.after(0, update_label, message)
        except Exception as e:
            print("Error receiving data:", e)
            break

# Function to update the label with the received message
def update_label(message):
    test_label.config(text=message)

# Create the main window
root = tk.Tk()
root.title("WASD Keyboard")
root.geometry("640x480")
root.configure(bg="grey")

# Create a frame to hold the keys
frame = tk.Frame(root, bg="grey")
frame.pack(expand=True)

# Create the labels for "W", "A", "S", and "D"
key_labels = {}
key_labels["W"] = tk.Label(frame, text="W", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace")
key_labels["A"] = tk.Label(frame, text="A", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace")
key_labels["S"] = tk.Label(frame, text="S", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace")
key_labels["D"] = tk.Label(frame, text="D", font=("Arial", 24), width=3, height=1, relief="raised", bg="SystemButtonFace")

test_label = tk.Label(frame, text="Waiting for message...", font=("Arial", 18), relief="raised", bg="SystemButtonFace")
test_label.grid(row=2, column=1, columnspan=3, pady=20)

# Layout the labels like a keyboard
key_labels["W"].grid(row=0, column=1, padx=5, pady=5)
key_labels["A"].grid(row=1, column=0, padx=5, pady=5)
key_labels["S"].grid(row=1, column=1, padx=5, pady=5)
key_labels["D"].grid(row=1, column=2, padx=5, pady=5)

# Bind the key press and key release events to the root window
root.bind("<KeyPress>", on_key_press)
root.bind("<KeyRelease>", on_key_release)

listener_thread = Thread(target=listen_for_messages, daemon=True)
listener_thread.start()

# Start the main loop
root.mainloop()
