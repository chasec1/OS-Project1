Test name: Driver2.c

It is supposed to fork a child process and have the parent and child processes run through for loops where
they should be conflicting with each other simultaneously, first they both add mailboxes at intervals that
should cause some some conflicts but the conflicts should be handled properly and errors returned. Next each
process adds to different mailboxes on different intervals followed by receiving on different intervals. Finally,
the child process tries to get the count and length from every 4 nodes while the parent tries to delete every fourth
node. However, none of these tests are successful. Had I more time I would have implemented tests make sure my ACL
implementation works as intended. I would do this by having processes that should not have access to mailboxes try
to gain access as well as try to give themselves access, and after all of these situations are handled correct
hopefully, I would give the process' access and have them attempt to read and write, then I would remove them and
make sure they no longer have access.