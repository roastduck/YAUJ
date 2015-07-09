// define webServer, port, dataPath runPath & sourcePath in /etc/yauj/daemon. the path should be full path without tailing slash
// you HAVE TO define webServer as "127.0.0.1" or "localhost" when running in web server.
// when running on web server, yauj_daemon should be run by www-data to access /tmp

#define	PIPE_READ_BUFF_MAX				33553408
#define	CONFIG_BUFF_MAX				65536

#define	MAX_RUN						2
