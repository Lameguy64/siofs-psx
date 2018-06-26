#ifndef SIOFS_H
#define SIOFS_H

/*!	\mainpage
 *
 *	Client library for the PlayStation to access files from a SioFS host through
 *	the serial port. Most commonly used file operations are implemented and
 *	querying directory listings.
 * 
 *	\version	0.50
 *	\author		Lameguy64
 *	\copyright	GNU General Public License
 */

/*! \addtogroup fileDefs File mode definitions
 *	\brief File mode definitions for fsOpen().
 *	\details Multiple mode attributes can be specified by simply OR'ing it together.
 *	@{
 */
#define FS_READ		0x1	//!< Read.
#define FS_WRITE	0x2	//!< Write.
#define FS_BINARY	0x4	//!< Binary mode.
/*	@}	*/

/*! \addtogroup seekDefs File seek mode definitions
 *	@{
 */
#define FS_SEEK_SET	0	//!< Set offset absolute.
#define FS_SEEK_CUR	1	//!< Set offset relative to current position.
#define FS_SEEK_END 2	//!< Seek relative to end of file.
/*	@}	*/

/*! \addtogroup errorDefs Error definitions
 *	@{
 */
#define FS_ERR_TIMEOUT		-1	//!< Communications timeout occured.
#define FS_ERR_NOTFOUND		-2	//!< File not found on host side.
#define FS_ERR_NOHANDLES	-3	//!< No file handle slots available on host.
#define FS_ERR_NOTOPEN		-4	//!< File handle not open.
#define FS_ERR_INVALID		-5	//!< Invalid file handle.
#define FS_ERR_READERROR	-6	//!< Read error on host.
#define FS_ERR_SEEKERROR	-7	//!< Seek error or invalid offset on host.
#define FS_ERR_DIRERROR		-8	//!< Directory open error.
#define FS_ERR_DIREMPTY		-9	//!< Directory is completely empty (only returned by fsFindFirst())
#define FS_ERR_NODIROPEN	-10	//!< No directory open (call fsFindFirst() first)
#define FS_ERR_DIRDONE		-11	//!< Directory query finished
/*	@}	*/

//! Directory entry struct
/*!	\details To be used with fsFindFirst(), fsFindNext() and fsListFiles().
 */
typedef struct {
	int size;					//!< File size
	struct {
		unsigned int seconds:6;	//!< Seconds (0 - 59)
		unsigned int minutes:6;	//!< Minutes (0 - 59)
		unsigned int hours:4;	//!< Hours (0 - 23)
		unsigned int day:5;		//!< Day (1 - 31)
		unsigned int month:4;	//!< Month (1 - 12)
		unsigned int year:7;	//!< Year (starts in 1980)
	} date;						//!< File date stamp
	unsigned short flags;		//!< File flags (bit 1 - Directory)
	unsigned short length;		//!< File name length
	char filename[64];			//!< File name
} FS_DIRENT;

//! File stat struct
/*!	\details To be used with fsStat(). Follows a similar structure as 
 *  FS_DIRENT minus the file name.
 */
typedef struct {
	int size;					//!< File size
	struct {
		unsigned int seconds:6;	//!< Seconds (0 - 59)
		unsigned int minutes:6;	//!< Minutes (0 - 59)
		unsigned int hours:4;	//!< Hours (0 - 23)
		unsigned int day:5;		//!< Day (1 - 31)
		unsigned int month:4;	//!< Month (1 - 12)
		unsigned int year:7;	//!< Year (starts in 1980)
	} date;						//!< File date stamp
	unsigned short flags;		//!< File flags (bit 1 - Directory)
	unsigned short pad;			//!< Padding
} FS_STAT;

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup fileFunctions File access functions
 *	\brief Functions for reading and writing files.
 *	@{
 */
	
//! Initializes the serial interface and resets the SioFS host
/*!
 *	\details Configures the serial interface and probes for the presence of a
 *	SioFS host and returns the protocol version. It also resets the host's
 *	working directory and clears all previously opened file handles.
 * 
 *  \note If you wish to use AddSIO() to direct console output to serial, it
 *	is recommended to call it before fsInit().
 *
 *	\param[in]	baud	Serial baud rate (must match host's rate).
 * 
 *  \returns Non-zero value containing the protocol version. FS_ERR_TIMEOUT if
 *	no SioFS host is detected.
 */
int fsInit(int baud);

//! Open a file
/*!
 *	\details Opens a file on the SioFS host and returns a handle number if
 *	successful.
 *
 *  \param[in]	filename	File name (max 64 characters).
 *	\param[in]	flags		File mode flags (see fileDefs).
 * 
 *  \returns File handle as a non-negative number starting at zero. Returns
 *	FS_ERR_NOTFOUND if file not found or FS_ERR_NOHANDLES if too many files are
 *	opened.
 */
int fsOpen(const char* filename, int flags);

//! Close an open file handle
/*!
 *	\param[in]	fd			File handle.
 * 
 *	\returns Zero if successful otherwise returns FS_ERR_NOTOPEN or 
 *	FS_ERR_INVALID.
 */
int fsClose(int fd);

//! Write data to an open file handle
/*!
 *	\param[in]	fd		File handle.
 *	\param[in]	*data	Buffer to write.
 *	\param[in]	size	Bytes to write.
 * 
 *	\returns Number of bytes written or errorDefs if negative.
 */
int fsWrite(int fd, u_char* data, int size);

//! Put a string to an open file handle
/*!
 *	\param[in]	fd		File handle.
 *	\param[in]	*text	String to put.
 * 
 *	\returns Number of bytes written or one of Error definitions if negative.
 */
int fsPuts(int fd, char* text);

//! Get a string from an open file handle
/*!
 *	\param[out]	*text	Buffer to store string to.
 *	\param[in]	length	Buffer length.
 *	\param[in]	fd		File handle.
 * 
 *	\returns Number of bytes read or errorDefs if negative.
 */
int fsGets(char* text, int length, int fd);

//! Read data from an open file handle
/*!
 *	\param[in]	fd		File handle.
 *	\param[out]	*data	Buffer to load data to.
 *	\param[in]	size	Bytes to read from file.
 * 
 *	\returns Number of bytes read from file or errorDefs if negative.
 */
int fsRead(int fd, u_char* data, int size);

//! Set file position of an open file handle
/*!
 *	\param[in]	fd		File handle.
 *	\param[in]	pos		Offset.
 *	\param[in]	mode	Seek mode (see seekDefs)
 * 
 *	\returns Zero if successful or errorDefs if negative.
 */
int fsSeek(int fd, int pos, int mode);

//! Get end of file status from last read.
/*!
 *	\returns Non zero if EOF is reached.
 */
int fsEof();

//! Read data from a file
/*! 
 *	\details Read contents of a file without opening it
 * 
 *	\param[in]	*filename	File name.
 *	\param[out]	*buffer		Buffer for read data.
 *	\param[in]	length		Number of bytes to read.
 *	\param[in]	offset		File offset to read from.
 * 
 *	\returns Number of bytes read or errorDefs if negative.
 */
int fsQuickRead(char* file, char* buffer, int length, int offset);

/*!	@} */

/*! \addtogroup dirFunctions Directory query functions
 *	\brief Functions for querying directory contents.
 *	@{
 */

//!	Find first file on current directory
/*!
 *	\details Retrieves the first file found on the host with the matching
 *	wildcard pattern.
 * 
 *	\param[in]	wildcard	Wildcard pattern.
 *	\param[out]	*dir		Pointer to an FS_DIRENT object.
 * 
 *	\returns Zero if succeeds. Non-zero on error.
 */
int fsFindFirst(char* wildcard, FS_DIRENT* dir);

//! Find the next file on current directory
/*!
 *	\details Retrieves the next file found on the host with the matching
 *	wildcard pattern from the fsFindFirst() call.
 * 
 *	\param[out]	*dir		Pointer to an FS_DIRENT object.
 * 
 *	\returns Zero if succeeds. Non-zero on error. FS_ERR_DIRDONE is returned
 *	when no more files follow or FS_ERR_NODIROPEN when fsFindFirst() was not
 *  called first.
 */
int fsFindNext(FS_DIRENT* dir);

//! List files on current directory
/*!
 *  \details Retrieves a listing of files on the host with the matching
 *	wildcard pattern.
 * 
 *	\param[in]	wildcard	Wildcard pattern (multiple patterns can be specified with ;).
 *	\param[in]	items		Maximum number of items to retrieve.
 *	\param[in]	offset		Directory list offset.
 *	\param[out]	*dir		Array of FS_DIRENT entries.
 *
 *	\returns Number of files found.
 */
int fsListFiles(char* wildcard, int items, int offset, FS_DIRENT* dir);

//! Get total number of files from previous listing
/*!
 *	\details To be called immediately after an fsListFiles() call. Returns the
 *	total number of files queried.
 * 
 * \returns Total number of files from last fsListFiles() call.
 */
int fsListTotal();

//! Stat a file
/*!
 *	\param[in]	filename	Name of file to stat.
 *	\param[out]	st			File attributes
 * 
 *	\returns Zero if succeeds. Non-zero on error.
 */
int fsStat(char* filename, FS_STAT* st);

//! Change current directory
/*!
 *	\param[in]	path		Path to change directory to.
 * 
 *	\returns Zero if succeeds. Non-zero on error.
 */
int fsChdir(char* path);

//! Get current working directory path
/*!
 *	\param[out]	path		Current directory path.
 * 
 *	\returns Zero if succeeds. Non-zero on error.
 */
int fsGetcwd(char* path);

/*!	@} */

#ifdef __cplusplus
}
#endif

#endif /* SIOFS_H */

