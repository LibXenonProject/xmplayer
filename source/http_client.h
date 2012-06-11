/*
 * File:   http_client.h
 * Author: cc
 *
 * Created on 7 juin 2012, 20:00
 */

#ifndef HTTP_CLIENT_H
#define	HTTP_CLIENT_H

#ifdef	__cplusplus
extern "C" {
#endif

	typedef struct {
		// -> must be set by user

		// hostname
		char hostname[32];
		// url
		char url[256];
		// ip adress
		struct ip_addr ipaddr;
		// called when all data fetched
		void (*callback)(void*);

		// -> read only for user

		// buffer for request
		char request[512];

		// response data
		void * data;
		// size of response data
		size_t len;

		// response status code (200/404 etc ...)
		unsigned short status_code;

		// used for speed calculation
		uint64_t start;
		uint64_t end;
	} xHttpRequest;

	void http_get_files(xHttpRequest * request);


#ifdef	__cplusplus
}
#endif

#endif	/* HTTP_CLIENT_H */

