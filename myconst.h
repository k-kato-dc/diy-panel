
// ---- USB CDC port ----

#ifdef WINDOWS
	#define STR_CDCDEV		"COM"
	#define NUM_CDCPORT		4
#else
	#define STR_CDCDEV		"/dev/ttyACM"
	#define NUM_CDCPORT		0
#endif

// ---- char code ----

#define WAIT_CHAR	0x95

// ---- file path ----

#ifdef WINDOWS

	#define STR_APPPATH		"C:\\Program Files (x86)"
	#define STR_SEP			"\\"
	#define STR_SHEXT		".bat"

#else

	#define STR_APPPATH		"/opt"
	#define STR_SEP			"/"
	#define STR_SHEXT		".sh"

#endif

#define STR_VENDOR		"AYOR"
#define STR_PACKAGE		"diy-panel"
#define STR_MODULE		"panel"
#define STR_FILEPREFIX	"diy-"

#define PACKAGE_PATH	STR_APPPATH STR_SEP STR_VENDOR STR_SEP STR_PACKAGE
#define MODULE_PATH	PACKAGE_PATH STR_SEP STR_MODULE

#define MODFILEPATH(module, fname)		PACKAGE_PATH STR_SEP module STR_SEP fname "\0" 
#define MODFILEPATHEX(module, subdir, fname)	PACKAGE_PATH STR_SEP module STR_SEP subdir STR_SEP fname "\0" 
#define SUBFILEPATH(subdir, fname)		MODULE_PATH STR_SEP subdir STR_SEP fname "\0" 
#define SCRFILEPATHEX(module, addext)		PACKAGE_PATH STR_SEP module STR_SEP STR_FILEPREFIX module STR_SHEXT addext"\0" 


//]EOF
