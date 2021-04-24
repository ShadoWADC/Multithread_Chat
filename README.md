# TITLE: Multithread_Chat

# Final project for Operative Systems - Sapienza University of Rome
# Teacher: Giorgio Grissetti and Irvin Aloise

# Description:
The project consists in the development of a private chat environment 
to exchange text messages between hosts. 

The project is composed by two main modules:

## Server: 
* Receives and does not store messages.
* Forwards the messages from client to client
* Notifies to the Sender Client, Delivered and Read status in their respective cases once they occur

## Client: 
* Provides a very simple interface to login or register a new user
* Once succesfully logged in the Client will show a menu with 3 options:
  * Send a message
  * Check Inbox
  * Exit

## Common errors are handled such as:
* Incorrect credentials during login
* Connection error, when connecting from the Client when the Server is not online
* User Offline notification, when sending a message to an offline user

The project has been tested with 3 users and by default could support until 5 users as specified in common.h in the constant MAX_USERS
