#ifndef CLIENT_CLASS_CPP
#define CLIENT_CLASS_CPP
#include "common_header.h"

#include "configParser.hpp"
#include <ios>
#include <sstream>
#include <string>
#include <sys/types.h>


class configParser;

class Client{

	public:

		Client(int sock_fd, int port);
		~Client();

		Client& operator=(const Client&);
		bool operator==(const Client&);

		std::string	getClientRequest(void);
		int			getFd(void);
		int			getPort() const;
		std::string	getHost() const;
		std::string	getResponse(void);
		std::string	getBuffer()const;
		std::string getRequestedFile(void);
		ssize_t		getBytesRemaining(void);

		ssize_t		readData(void);
		ssize_t		sendData();
		void		closeConnection(void);

		void		parseRequest();
		std::string	parseUploadFileName(std::string&);

		int			processRequest(configParser& parser);
		void		setConfig(configParser& parser);
		void		setMimeType(std::string file);
		void		setContentType();
		ssize_t		readFile(std::string path);
		int			readError(int statuscode, std::string info);

		std::vector<std::string>	ls(std::string folder);
		void		getDirectoryList(std::string file);

		int			checkMethod(configParser &parser);
		int			getMethod(configParser& parser);
		int			postMethod();
		int			deleteMethod();
		void		printInternal();


		//post
		void		printPostinfo();
		void		multiPart();
		void		ongoingMultiPart();
		bool		UploadinProgress();
		std::string	encodeHtml(const std::string& input);

		//cgi
		bool		validCGIextension();
		int			callCGI();
		bool 		checkForP();
		int			directoryListing(std::string& path);


	private:

		// infos through first ping
		int							_fd;
		int							_port;

		//from config
		std::string					_host;
		std::string					_method;
		std::string					_requestUrl;
		std::string					_urlPath;
		std::string					_fileType;
		std::string					_index;
		std::string					_root;
		int							_bodySize;
		bool						_redirect;
		std::string					_requestedFile;
		std::string					_dirPath;
		std::vector<std::string>	_dirFiles;


		bool						_autoIndex;
		std::map<int,std::string>	_errorMap;

		bool						_allowGet;
		bool						_allowPost;
		bool						_allowDelete;

		//for now variables for chunking
		size_t						_bytesSent;
		size_t						_bytesRemaining;
		size_t						_fileSize;
		std::streampos				_position;

		std::string 				_cgiPath;
		int							_timeout;
		std::string 				_query;
		std::string					_fileEnding;

		// POST
		ssize_t						_contentLen;
		ssize_t						_remainingBytes;//todelete
		std::string 				_contentType;
		std::string					_boundary;
		std::string 				_uploadFileName;
		std::string 				_uploadContentType;
		std::string 				_uploadPath;
		std::string					_body;
		bool						_startBoundary;
		bool						_endBoundary;
		bool						_multipart;
		bool						_done;
	
		// infos through response header
		std::string					_buffer;
		std::streampos				_filePos;



		std::string 	_responseStr;
};

#endif