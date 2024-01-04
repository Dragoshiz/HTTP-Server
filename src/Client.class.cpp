#include "../include/Client.class.hpp"
#include "../include/common_header.h"
#include "../include/common_header.h"
#include <fstream>
#include <iostream>
#include <string>



Client::Client(int sock_fd, int port): _fd(sock_fd), _port(port),
 _bytesSent(0) ,_bytesRemaining(0), _position(0), _body(""), _startBoundary(false), _multipart(false) ,_done(false),_filePos(0){};


Client::~Client(){}

bool Client::operator==(const Client& obj){
	if (this == &obj)
		return true;
	return false;
}

int Client::checkMethod(configParser& parser)
{
	setConfig(parser);
	std::string path;

	// std::cout << _requestUrl << std::endl;

	if (_requestUrl == "/" || _requestUrl == "")
		_requestedFile = "www/content/index.html";
	else{
		if (_requestUrl[0] == '/' )
			_requestUrl = _requestUrl.substr(1);
		_requestedFile = "www/content/" + _requestUrl;
	}

	if (_method == "GET")
	{
		if (_allowGet)
			return getMethod(parser);
		else
			return 403;
	}

	else if (_method == "POST")
	{
		if (_allowPost)
			return postMethod();
		else
			return 403;
	}

	else if (_method == "DELETE")
	{
		if (_allowDelete)
			return deleteMethod();
		else
			return 403;
	}

	return 501;
}

ssize_t Client::readData(){
	size_t BUFFER_SIZE = CHUNK_SIZE;
	char tmpBuff[BUFFER_SIZE];
	memset(tmpBuff,0,BUFFER_SIZE);

	ssize_t bytesRead = recv(_fd, tmpBuff, sizeof(tmpBuff), MSG_DONTWAIT);

	if (bytesRead > 0) {
		std::string tmpstr(tmpBuff, (bytesRead));
		_buffer = tmpstr;
	}

	return bytesRead;
}


ssize_t Client::sendData(){
	 if (_responseStr.empty()) {
		return 0;
	}

	ssize_t bytesSent = send(_fd, _responseStr.data(), _responseStr.size(), MSG_DONTWAIT);

	if (bytesSent == static_cast<ssize_t>(_responseStr.size())) {
		_responseStr.clear();
	}
	
	else if (bytesSent > 0) {
		_responseStr.erase(_responseStr.begin(), _responseStr.begin() + bytesSent);
	}
	
	else if (bytesSent < 0) {
		std::cerr << "Error sending data: " << strerror(errno) << std::endl;
		return -1;
	}

	return bytesSent;
}

void Client::closeConnection(void){
	close(_fd);
}

std::string Client::getClientRequest(void){
	return _buffer;
}

int Client::getFd(void)
{
	return _fd;
}

std::string Client::getResponse(){
	 return _responseStr;
}

int Client::getPort() const
{
	return _port;
}

std::string Client::getHost() const
{
	return _host;
}

std::string Client::getBuffer()const{
	return _buffer;
}

ssize_t Client::getBytesRemaining(void){
	return _bytesRemaining;
}

std::string Client::getRequestedFile(void){
	return _requestedFile;
}

void Client::setConfig(configParser& parser){
	parser.setData(_requestUrl, "127.0.0.1", _port);

	this->_redirect = parser.getHasRedirection();
	this->_requestUrl = parser.getUrl();
	this->_index = parser.getIndexFile();
	this->_allowGet = parser.getGetAllowed();
	this->_allowPost = parser.getPostAllowed();
	this->_allowDelete = parser.getDeleteAllowed();
	this->_autoIndex = parser.getAutoIndex();
	this->_root = ROOT;
	this->_errorMap = parser.getErrorMap();
	this->_bodySize = parser.getBodySize(_port);
	this->_timeout = parser.get_timeout();
	this->_urlPath = parser.getUrl();

}

int Client::processRequest(configParser& parser){

	if(_method.empty())
		parseRequest();

	int check = checkMethod(parser);
	if (check != 200)
	{
		switch (check)
		{
			case 400:
				return readError(400, "Bad Request");
			case 403:
				return readError(403, "Forbidden");
			case 404:
				return readError(404, "Not Found");
			case 413:
				return readError(413, "Request Entity Too Large");
			case 500:
				return readError(500, "Internal Server Error");
			case 501:
				return readError(501, "Not Implemented");
			case 504:
				return readError(504, "Gateway timeout");
			case 201:
				return readFile(PATH_FILE_SAVED);
			case 0:
				return 0; //we continue in POLLIN
		}
	}
	return 200;
}

void Client::parseRequest(){
	std::string request(_buffer);
	this->_method = request.substr(0, request.find(" "));
	std::string host;
	size_t pos = request.find("Host: ");

	if(pos != std::string::npos)
		host = request.substr(pos + 6);

	if (!host.empty())
	{
		_host = host.substr(0, host.find(':'));
	}
	else
		_host = "localhost";

	request = request.substr(request.find(" ") + 1);
	_requestUrl = request.substr(1, (request.find(" ")-1));
	if (_method == "GET")

	{
		return;
	}
	else if (_method == "POST")
	{
		std::string contentType = request.substr(request.find("Content-Type: ") + 14);
		std::string contentLen = request.substr(request.find("Content-Length: ") + 16);

		contentLen = contentLen.substr(0,contentLen.find("\r\n"));
		std::stringstream ss(contentLen);
		ss >> _contentLen;

		_remainingBytes = _contentLen; 
		_contentType = contentType.substr(0,contentType.find(";"));

		if (_contentType == "multipart/form-data")
		{
			std::string boundary = request.substr(request.find("boundary=") + 9);
			boundary = boundary.substr(0,boundary.find("\r\n"));
			this->_boundary = boundary;
		}
	}
}

std::string	Client::parseUploadFileName(std::string& temp)
{
	std::string contentKey = "Content-Disposition: form-data;";
	size_t pos = temp.find(contentKey);

	std::string filenameKey = "filename=\"";
	pos = temp.find(filenameKey, pos);
	
	if (pos == std::string::npos)
		return "";
		
	size_t start = pos + filenameKey.size();
	size_t end = temp.find("\"", start);

	if (end == std::string::npos)
		return "";

	return encodeHtml(temp.substr(start, end - start));
}

void Client::printInternal(){
	std::cout << "\n!Checking internal client parameters!\n" << std::endl;

	std::cout << "fd = " << _fd << std::endl;
	std::cout << "port = " << _port << std::endl;
	std::cout << "host = " << _host << std::endl;
	std::cout << "url = " << _urlPath<< std::endl;
	std::cout << "method = " << _method << std::endl;
	std::cout << "index = " << _index << std::endl;
	std::cout << std::boolalpha;
	std::cout << "redirect = " << _redirect << "\n";
	std::cout << "autoIndex = " << _autoIndex << "\n";
	std::cout << "allowGet = " << _allowGet << "\n";
	std::cout << "allowPost = " << _allowPost <<  "\n";
	std::cout << "allowDelete = " << _allowDelete << std::endl;
}

//milfs and Piotrs Sister

void Client::printPostinfo()
{
	std::cout << "\n\nPrinting Post info"<< std::endl;
	std::cout << "Content Length: " << _contentLen << std::endl;
	std::cout <<"Cnntent-Type: " << _contentType << std::endl;
	std::cout << std::boolalpha;
	std::cout << "Multipart: " << _multipart << std::endl;
	std::cout <<"Boundary: " << _boundary << std::endl;
	std::cout << "Filename: " <<_uploadFileName << "<<"<< std::endl;
	std::cout << "Filetype: " << _uploadContentType<< "<<" << std::endl;
}

std::string Client::encodeHtml(const std::string& input) {
	std::string result;
	for (size_t i = 0; i < input.size(); ++i) {
		if (input[i] == ' ') {
			result += "%20";
		} else {
			result += input[i];
		}
	}
	return result;
}
