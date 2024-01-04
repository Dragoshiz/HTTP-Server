#include "../include/Client.class.hpp"


int Client::postMethod()
{
	if (validCGIextension())
		return callCGI();

	// if (_contentLen/ == 0)
		// return 400;

	if (_buffer.size() > (size_t)_bodySize)
		return 413;

	if (!_multipart)
		multiPart();
	else
		ongoingMultiPart();


	if (_done){
		if (_uploadFileName.empty())
			return 400;
		_responseStr = "HTTP/1.1 201 OK\r\n";
		return 201;
	}

	return 0;
}


/* HTTP requests format are dependent on the browser/client.
generally speaking there will always be a startboundary defining the body and
an endBoundary defining the end of the body.

In multipart/form-data from the browser will be another part added
defining the form field "submit". It does not contain file data, also possible that this
is OS or Hardware dependent */

void Client::multiPart() {

	const std::string startBoundary = "--" + _boundary + "\r\n";
	size_t start = _buffer.find(startBoundary);


	if (start == std::string::npos) {
		size_t bodyPos = _buffer.find("\r\n\r\n") + 4;
		if (bodyPos != std::string::npos){
		_body = _buffer.substr(bodyPos);
			if (_body.size() == (size_t)_contentLen){
				std::ofstream outfile("www/content/storage/plainFile", std::ios::binary | std::ios::trunc);
				_uploadFileName = "plainFile";
				outfile.write(_body.c_str(), _body.size());
				_done = true;
				return;
			}
		}
		_multipart = true;
		return;
	}
	_startBoundary = true;

	const std::string endBoundary = "\r\n--" + _boundary + "--";
	size_t end = _buffer.find(endBoundary);
	if (end != std::string::npos) {
		_done = true;
	}

	_uploadFileName = parseUploadFileName(_buffer);
	if (_uploadFileName.empty()) {
		return;
	}
	_uploadPath = "www/content/storage/" + _uploadFileName;

	if (access(_uploadPath.c_str(), F_OK) == 0) {
		_uploadFileName = "copy_" + _uploadFileName;
		_uploadPath = "www/content/storage/" + _uploadFileName;
	}

	if (_done) {
		_body = _buffer.substr(start + startBoundary.size());
		std::string temp(_body);
		size_t bodyPos = temp.find("\r\n\r\n") + 4; //doesn't find the body occasionally

		if (bodyPos != std::string::npos) {
			_body = temp.substr(bodyPos);
			temp = _body;
			end = temp.find(endBoundary);
			_body = temp.substr(0, end);
		}
		start = _body.find("\r\n" +startBoundary);
		if (start != std::string::npos) {
			temp = _body;
			_body = _body.substr(0, start);
		}
	}
	else {
		_body = _buffer.substr(start + startBoundary.size());
		std::string temp(_body);
		size_t bodyPos = temp.find("\r\n\r\n") + 4;
		_body = temp.substr(bodyPos);
	}
	std::ofstream outfile(_uploadPath.c_str(), std::ios::binary | std::ios::trunc);
	outfile.write(_body.c_str(), _body.size());

	if (_done) {
		return;
	}

	_multipart = true;
}

void Client::ongoingMultiPart(){
	_body.clear();

	if (!_startBoundary){

		_uploadFileName = parseUploadFileName(_buffer);
		if (_uploadFileName.empty()) {
			return;
		}
		_uploadPath = "www/content/storage/" + _uploadFileName;

		if (access(_uploadPath.c_str(), F_OK) == 0) {
			_uploadFileName = "copy_" + _uploadFileName;
			_uploadPath = "www/content/storage/" + _uploadFileName;
		}

		const std::string startBoundary = "--" + _boundary + "\r\n";
		size_t start = _buffer.find(startBoundary);
		_body = _buffer.substr(start + startBoundary.size());

		std::string temp(_body);
		
		size_t bodyPos = temp.find("\r\n\r\n") + 4;
		
		if (bodyPos != std::string::npos){
			_body = temp.substr(bodyPos);
		}
		std::ofstream outfile(_uploadPath.c_str(), std::ios::binary | std::ios::trunc);
		_startBoundary = true;
	}
	else
		_body = _buffer;


	const std::string endBoundary = "\r\n--" + _boundary + "--";
	size_t end = _buffer.find(endBoundary);

	if (end != std::string::npos) {
		_done = true;
	}

	std::ofstream outfile(_uploadPath.c_str(), std::ios::binary | std::ios::app);
	if (_done){
		end = _body.find(endBoundary);
		_body = _body.substr(0, end);
		const std::string startBoundary = "\r\n--" + _boundary;
		size_t start = _body.find(startBoundary);
		if (start != std::string::npos) {
			std::string temp(_body);
			_body = _body.substr(0, start);
		}
		outfile.write(_body.c_str(), _body.size());
	}
	else {
		outfile.write(_body.c_str(), _body.size()); 
	}
}

bool Client::UploadinProgress(){
	if (_multipart)
		return true;

	return false;
}