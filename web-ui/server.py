#!/usr/bin/env python3
 
import http.server
import http.server
import cgitb
cgitb.enable()  ## This line enables CGI errorreporting

name = ""
port = 80

server = http.server.HTTPServer
handler = http.server.CGIHTTPRequestHandler
server_address = (name, port)
handler.cgi_directories = ["/pad1", "/pad2", "/cgi-bin"]

if name == "":
  name = "localhost"
print("Staring server. Visit http://%s:%s" % (name, port))
 
httpd = server(server_address, handler)
httpd.serve_forever()
