Pingd
-----

A daemonized ping utility for determining internet protocol latency across a network.

Pingd uses crafted ICMP ECHO packets to determine round trip time.

Originally pingd used ICMP TIMESTAMP packets to attempt to split the rtt in rx and tx but
distributed clock offset added technical complications that may or may not be addressed in
the future. But, it was deteremined such a solution would be a cheap estimate off clock
offset, and a utility built with ICMP TIMESTAMP may be used to monitor NTP performance.
