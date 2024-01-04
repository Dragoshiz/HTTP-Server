#include "../include/Client.class.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/types.h>

int Client::getMethod(configParser& parser){
	(void) parser;

	if(validCGIextension())
	{
		return callCGI();
	}

	if (_requestedFile.size() > 1 && _requestedFile[0] == '/') 
		_requestedFile.erase(0, 1);


	DIR *dir = opendir(_requestedFile.c_str());
	if (dir) {
		this->_dirPath = _requestedFile;
		std::string indexPath = _requestedFile + _index; // Construct path to potential _index file inside the directory
		DIR *indexx = opendir(indexPath.c_str());
		if(access(indexPath.c_str(), F_OK) == 0 && indexx == NULL) {
			// If _index file exists inside the directory, read it.
			setMimeType(indexPath);
			setContentType();
			readFile(indexPath);
			closedir(dir);
			return 200;
		} else if (_autoIndex && _requestedFile.find("favicon.ico") == std::string::npos) {
			setMimeType(_requestedFile);
			setContentType();
			directoryListing(_requestedFile);
			closedir(dir);
			closedir(indexx);
			return 200;
		}
		closedir(dir);
	} else if (access(_requestedFile.c_str(), F_OK) == 0) {
		setMimeType(_requestedFile);
		setContentType();
		readFile(_requestedFile);
		return 200;
	}
	return 404;
}



void	Client::setMimeType(std::string file){
	size_t dotPos = file.find_last_of(".");
	this->_fileType = file.substr(dotPos + 1);
}

// lists directories in storage
std::vector<std::string> directoryFiles(std::string& path){
	std::string storagePath = path;

	DIR *dir = opendir(storagePath.c_str());
	if(dir == NULL){
		std::cout << RED << "File not found." << dir << RESET_COLOR << std::endl;
		return std::vector<std::string>(); //empty vector or how should we handle
	}

	struct dirent *entity;
	std::vector<std::string> files;

	while((entity = readdir(dir)) != NULL){
		if (entity->d_type == DT_DIR){
			if (strcmp(entity->d_name, ".") == 0 || strcmp(entity->d_name, "..") == 0){
				continue;
			}
			files.push_back(std::string(entity->d_name) + "/");
		} else {
			files.push_back(entity->d_name);
		}
	}
	
	closedir(dir);
	return files;
}




int Client::directoryListing(std::string& path){
	std::vector<std::string> storageContent = directoryFiles(path);

	std::stringstream html;
	html << "<!DOCTYPE>"
		<< "<html>"
		<< "<head><title>Directory Listing</title></head>"
		<< "<body>"
		<< "<ul>";
	for (size_t i = 0; i < storageContent.size(); i++){
		html << "<li><a href=\""  << storageContent[i] << "\">" << storageContent[i] << "</a>";
		if (storageContent[i][storageContent[i].size() - 1] != '/' && _dirPath.find("/storage") != std::string::npos)
			html << "<a href=http://localhost:" << _port << "/" << _dirPath.substr(12) <<"/" << storageContent[i] << " download=\"" << storageContent[i] << "\">"  << " <-- Download this file</a>";
		html << "</li>";
	}	
	html << "</ul>"
		<< "</body>"
		<< "</html>";
	std::stringstream htmlLen;
	htmlLen << html.str().size();
	_responseStr += "Content-Length: " + htmlLen.str() + "\r\n\r\n";
	_responseStr += html.str();
	html.clear();

	return 200;
}

// sets proper response to the Multimedia Type
void	Client::setContentType()
{
	const char* mimeSubtypes[] = {
	"plain", "html", "css", "javascript",
	"jpeg", "png", "gif", "svg+xml",
	"mpeg", "ogg", "wav",
	"mp4", "ogg", "webm",
	"json", "xml", "pdf", "zip", "octet-stream","ico", "html"
};
	int i = 0;
	while(i <= 19 && mimeSubtypes[i] != _fileType){
		i++;
	}
	_responseStr.clear();// pls remove this and remove clients properly after a request
	_responseStr ="HTTP/1.1 200 OK\r\n";

	switch (i)
	{
	case 0:
		_responseStr += "Content-Type: text/plain\r\n";
		break;
	case 1:
		_responseStr += "Content-Type: text/html\r\n";
		break;

	case 2:
		_responseStr += "Content-Type: text/CSS\r\n";
		break;

	case 3:
		_responseStr +="Content-Type: application/javascript\r\n";
		break;

	case 4:
		_responseStr +="Content-Type: image/jpeg\r\n";
		break;

	case 5:
		_responseStr +="Content-Type: image/PNG\r\n";
		break;

	case 6:
		_responseStr +="Content-Type: image/gif\r\n";
		break;

	case 7:
		_responseStr +="Content-Type: image/svg+xml\r\n";
		break;

	case 8:
		_responseStr +="Content-Type: audio/mpeg\r\n";
		break;
	case 9:
		_responseStr += "Content-Type: audio/ogg\r\n";

		break;
	case 10:
		_responseStr += "Content-Type: audio/wav\r\n";

		break;
	case 11:
		_responseStr += "Content-Type: video/mp4\r\n";

		break;
	case 12:
		_responseStr += "Content-Type: video/ogg\r\n";

		break;
	case 13:
		_responseStr += "Content-Type: video/webm\r\n";

		break;
	case 14:
		_responseStr += "Content-Type: application/json\r\n";

		break;
	case 15:
		_responseStr += "Content-Type: application/xml\r\n";

		break;
	case 16:
		_responseStr += "Content-Type: application/pdf\r\n";

		break;
	case 17:
		_responseStr += "Content-Type: application/zip\r\n";

		break;
	case 18:
		_responseStr += "Content-Type: application/octet-stream\r\n";

		break;
	case 19:
		_responseStr +="Content-Type: image/x-icon\r\n";
			break;
	case 20:
		_responseStr +="Content-Type: text/html\r\n";
	}
}

ssize_t Client::readFile(std::string path) {
	std::ifstream file;

	file.open(path.c_str(), std::ios::binary | std::ios::ate);
	if (!file)
	{
		std::cerr << "Couldn't access " << path << std::endl;
		return 0;
	}

	if (_position == 0){
		_fileSize = file.tellg();
		std::stringstream ss;
		ss << _fileSize;
		file.seekg(0, std::ios::beg);
		_bytesRemaining = _fileSize;
		_responseStr += "Content-Lenght: " + ss.str() + "\r\n\r\n";
	}

	//read from file
	std::vector<char> buffer(CHUNK_SIZE);
	file.seekg(_position);
	file.read(buffer.data(), std::min(CHUNK_SIZE, static_cast<int>(_bytesRemaining)));	

	//update remaining bytes from file
	size_t bytesRead = file.gcount();
	_position += bytesRead;
	_bytesRemaining -= bytesRead;

	_responseStr += std::string(buffer.data(), bytesRead);

	if (_bytesRemaining == 0)
		_position = 0;

	file.close();
	return bytesRead;
}


std::vector<std::string> Client::ls(std::string folder){
	std::string path = folder;
	DIR *dir = opendir(path.c_str());
	if (dir == NULL)
		std::cerr << "something wrong happened"; // TODO handle the error
	struct dirent *entity;
	entity = readdir(dir);
	std::string name;
	while (entity != NULL){
		if (entity->d_type == 4){
			name = ".";
			if (entity->d_name == name || entity->d_name == name + "."){
				entity = readdir(dir);
				continue;
			}
			name = "/";
			_dirFiles.push_back(entity->d_name + name);
		}
		else{
			_dirFiles.push_back(entity->d_name);
		}
		entity = readdir(dir);
	}

	closedir(dir);
	return _dirFiles;
}
