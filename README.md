# CIS525 Project 6
Group Members: 
- Teegan Swanson
- Ethan Hulen
- Nathan Fleming

How to Compile and Run: make; ./directoryServer5 &; ./chatServer5 <portNumber> <serverTopic>; ./chatClient5

Limitations: 
1. Upon each new user's first message into the chat, there is an extra blank line above it with the nickname attached. 
2. Every time the directory server is restarted, SERV_TCP_PORT and the chat Server port need to be incremented
3. Our implementation requires clients stay connected
4. Server must contain topic KSUFootball
5. Messages & names must be shorter than 100 characters
6. When running chatServer, must include a port number and server topic in that order, will seg fault otherwise
7. Must be run on cougar


Sample Walkthrough:
1. run make
2. run ./directoryServer5 &
3. run ./chatServer5 <portNumber> <serverTopic>
4. run ./chatClient5
6. choose server to run on
7. Type username into client [note: if server topic is not "KSUFootball", error message is displayed and program exits]
8. Type message into client



