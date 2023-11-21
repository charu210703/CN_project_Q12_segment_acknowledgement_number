# CN_project_Q12_TCP Segment Communication Simulation

This project focuses on the design and implementation of a simplified Transmission Control Protocol (TCP) Segment Communication Simulation. TCP is a widely used transport layer protocol responsible for reliable data transmission over a network. In this simulation, we emphasize the allocation of segment numbers based on the number of bytes transmitted and acknowledgment numbers.

## Requirements
C compiler (e.g., GCC)<br/>
Operating System with Socket Support (Linux)<br/>
## Clone the repository:

```
git clone [repository-url]
```
## Navigate to the project directory:

```
cd [project-folder]
```
Alternatively, download the files locally and follow the steps below.

## Compile the client and server programs:

```
gcc <server-filename>.c -o server
gcc <client-filename>.c -o client
```
## Run the server:

```
./server 
```
## Run the client:

```
./client serveraddress
```
## Example Usage
Open a terminal and navigate to the project directory.<br/>
Compile the server and client programs.<br/>
Start the server by running ./server [Port Number].<br/>
In a separate terminal, run the client using ./client [Port Number].<br/>
## Notes
Ensure that you have the necessary permissions to execute the compiled files.<br/>
Customize the port number based on your requirements.<br/>
This simulation is designed for a Linux operating system with socket support.<br/>
Feel free to explore the code and adapt it to your specific needs. If you encounter any issues or have questions, please don't hesitate to reach out. Happy simulating!
