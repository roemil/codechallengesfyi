# Simple IRC Client
[Repo](https://github.com/roemil/codechallengesfyi/tree/master/ccirc)

## Screenshot
![image](irc_screenshot.png)

## Compiling
* Check out repo
* mkdir build
* run 'cd build && cmake .. && make'

## Usage
* Compile and run the ccirc binary. 
* Join channels with command "/join #NAME"
* Leave channels with command "/part #NAME"
* Type message and send them by pressing enter.

## Limitations
* Only one channel at a time
* No private messages between users
* a lot of the IRC protocol is not implemented yet.

## Known bugs/TODO
* Add a quit command to close down the application gracefully
* User list is not updated when users join or leave channel
* Support multiple channels
* Support messages between users
* Fuzz testing on input

