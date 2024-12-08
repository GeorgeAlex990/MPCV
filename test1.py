import tkinter as tk
from pynput import keyboard
import threading
import requests
import socket

UDP_IP = "192.168.10.151" # The IP that is printed in the serial monitor from the ESP32
SHARED_UDP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Internet  # UDP
sock.connect((UDP_IP, SHARED_UDP_PORT))

# Global variables
current_key = None
keylogger_active = False

# Function to handle key press
def on_press(key):
    global current_key
    try:
        current_key = key.char.upper()
    except AttributeError:
        current_key = f'[{key.name.upper()}]'
    
    update_display()

# Function to handle key release
def on_release(key):
    global current_key
    current_key = None
    update_display()

# Update the display label with the current key
def update_display():
    if current_key:
        display_label.config(text=current_key)
        sock.send(current_key.encode())
    else:
        display_label.config(text="")
        sock.send(' '.encode())

# Function to start the keylogger
def start_keylogger():
    global keylogger_active
    if not keylogger_active:
        keylogger_active = True
        listener = keyboard.Listener(on_press=on_press, on_release=on_release)
        listener.start()
        start_button.config(state=tk.DISABLED)
        stop_button.config(state=tk.NORMAL)

# Function to stop the keylogger
def stop_keylogger():
    global keylogger_active
    if keylogger_active:
        keylogger_active = False
        start_button.config(state=tk.NORMAL)
        stop_button.config(state=tk.DISABLED)

# Function to handle closing the window
def on_closing():
    if keylogger_active:
        stop_keylogger()
    root.destroy()

# GUI setup
root = tk.Tk()
root.title("Keylogger GUI")

display_label = tk.Label(root, text="", font=("Helvetica", 48))
display_label.pack(pady=20)

start_button = tk.Button(root, text="Start Keylogger", command=start_keylogger)
start_button.pack(pady=10)

stop_button = tk.Button(root, text="Stop Keylogger", command=stop_keylogger, state=tk.DISABLED)
stop_button.pack(pady=10)

root.protocol("WM_DELETE_WINDOW", on_closing)

root.mainloop()
