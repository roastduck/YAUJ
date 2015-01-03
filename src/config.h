// run
#define	DEFAULT_WALLCLOCK_LIMIT			60000		// overall time. in ms
//#define	DEFAULT_CPU_LIMIT				1000			// cpu time. in ms
//#define	DEFAULT_MEMORY_LIMIT			268435456		// in bytes
#define	DEFAULT_OUTPUT_LIMIT			33554432		// disk usage limit. in bytes
//#define	RUN_PATH						"/home/rd/gadget/yauj/test/"

// compile
#define	DEFAULT_O2					true
#define	DEFAULT_DEFINE					"ONLINE_JUDGE"

// diff
#define DEFAULT_W_MODE					1

// buffer
#define	COMMAND_BUFF_MAX				256
#define	PIPE_READ_BUFF_MAX				65536
#define	DIFF_FILE_BUFF_MAX				DEFAULT_OUTPUT_LIMIT
#define	FUNC_READ_BUFF_MAX				DEFAULT_OUTPUT_LIMIT
