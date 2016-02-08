# Irccdctl

The `irccdctl` utility let you controlling a running `irccd` instance. It uses sockets to perform any operation.

You need to define at least one transport before using `irccdctl`.

## Configuration

Configuration of irccdctl is done in the same rules than `irccd`.

### The general section

This section defines the global irccdctl parameters.

The available options:

- **verbose**: (bool) enable verbose message (Optional, default: false).

**Example**

````ini
[general]
verbose = true
````

## The connect section

The section socket permit irccdctl to connect to a specific irccd listener, only one may be defined. Just like
transports you can connect to Unix or internet sockets.

The available options:

  - **type**: (string) type of listener "ip" or "unix"

The options for **internet** type:

  - **host**: (string) host to connect,
  - **port**: (int) port number,
  - **family**: (string) internet family: ipv6 or ipv4 (Optional, default: ipv4).

The options for **unix** type:

  - **path**: (string) Required. The file path to the socket.

**Example for internet transports**

````ini
[connect]
type = "internet"
host = "localhost"
port = "9999"
family = "ipv6"
````

**Example for unix transports**

````ini
[connect]
type = "unix"
path = "/tmp/irccd.sock"
````
