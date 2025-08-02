# part 1

**main:**

Listening on given port and accept new connect from client and handle request in a infinite loop

**handle_request:**

first parse request `GET http://www.cmu.edu/hub/index.html HTTP/1.1` to get `method` `host` `path` and `version`,

then build new request send to server: `GET /hub/index.html HTTP/1.0,` ps: pay attention to these request headers `Host`, `User-agent`, `Connection`, `Proxy-connection`,

finally read the response from server and forward to client


# part 2

# part 3
