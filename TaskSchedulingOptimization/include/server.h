/*
 * Module: HTTP Server
 * Description: Minimal pure-C HTTP server on port 9090.
 *   Serves static files from web/ and JSON API endpoints.
 */
#ifndef SERVER_H
#define SERVER_H

/* Start the HTTP server on the given port. Blocks forever. */
void server_start(int port);

#endif /* SERVER_H */
