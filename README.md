# PebbleNotes
This is a program for Pebble smartwatch which allows access to
Google Tasks service. Planned are read-only access and an ability
to mark tasks as done/undone.

## Requirements
Program must work on both Android and iOS devices, because it uses
JavaScript backend for accessing tasks.
It requires an Internet connection to be active!

You may edit tasks with your preferred client: Gmail web interface,
GTasks android app, (my favourite) NoNonsenceNotes for Android
or any iOS client. As long as your tasks are synchronized with Google
and you have active Internet connection, you will see your latest
notes/tasks.

Compatible editors:
- Android:
 - Tasks Free,
 - GTasks,
 - NoNonsenceNotes,
 - ...
- iOS:
 - GTasks,
 - GTasks HD,
 - GTasks.net,
 - ...

## ToDo
+ Google API auth
+ Persistent storage of api access token
+ GoogleTasks access
- Auto refresh access token
- App-phone communication protocol
- 1st version of app interface (tree-like)
- 2nd version of interface (saving positions, etc)
- Realtime information updates
- Android NoNonsenceNotes integration??
- Cache lists/tasks for offline access

## Watch-Phone protocol
Messages >124b?
Short IDs?
Maybe use appsync?
- Get lists count
- Get n-th list name/id
- Get list size
- Get n-th task from list ID (name, state, has-comment)
- Get comment for task M list N
- Mark task M as done/undone
