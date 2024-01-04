 #include "../include/Client.class.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

//might rename the variable location for something more fitting.
//has the url after the domain
//TODO:cgi location hard coded, needs to be changed!


bool	Client::checkForP(void){

	int result = false;

	if (_fileEnding == ".py") {
		result = std::system("python3 --version > temp.txt");
	} else if (_fileEnding == ".pl") {
		result = std::system("perl --version > temp.txt");
	}

	if (result == 0) {
		remove("temp.txt");
		return true;
	}
	remove("temp.txt");
	return false;
}

bool Client::validCGIextension() {
	std::vector<std::string> allowed_ending;
	allowed_ending.push_back(".py");
	allowed_ending.push_back(".pl");
	std::string	temp;
	size_t		dot;
	size_t pos = _requestedFile.find("?");

	if (pos != std::string::npos){
		this->_cgiPath = _requestedFile.substr(0, pos);
		this->_query = this->_requestedFile.substr(pos+1);
	}
	else
		this->_cgiPath = this->_requestedFile;

	dot = _cgiPath.find_last_of('.');
	if (dot == std::string::npos)
		return false;
	temp = _cgiPath.substr(dot);
	for (size_t i = 0; i < allowed_ending.size(); i++)
	{

		if (_cgiPath.size() < allowed_ending[i].size())
			continue;
		if(temp == allowed_ending[i]){
			_fileEnding = temp;
			return true;
		}
	}
	return false;
}

int Client::callCGI() {
	int pipefd[2];
	int status;

	
	if(!checkForP()) //not installed for execution
		return 500;
	if(access(_cgiPath.c_str(), F_OK)!= 0) // no such file
		return 404;
	if(access(_cgiPath.c_str(), X_OK)!= 0) // no permission
		return 403;
	if(_fileEnding != ".py" && _fileEnding != ".pl") //not supported
		return 501;

	if (_method == "POST"){
		size_t pos = _buffer.find("\r\n\r\n");
		_query = _buffer.substr(pos + 4);
	}
	_query = "QUERY_STRING=" + _query;
	char *query = (char *) _query.c_str();


	std::string exec;

	if (_fileEnding == ".py")
		exec = "python3";
	else
		exec = "perl";

	char *env[] = {query, 0};
	char *cmd = (char*)_cgiPath.c_str();
	char *argv[] = {const_cast<char *>(exec.c_str()), cmd, 0};

	int file = open("www/tempCGI", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); //TODO hardcoded. Do whatever for naming or keep it

	if (pipe(pipefd) == -1) {
		std::cerr << "Something went wrong creating the pipe!" << std::endl;
		close (file);
		remove("www/tempCgi");
		return 500;
	}

	int pid = fork();
	if (pid == -1) {
		std::cerr << "Something went wrong with fork" << std::endl;
		close (file);
		remove("www/tempCgi");
		return 500;
	}

	else if (pid == 0)
	{
		close(pipefd[0]);
		dup2(file, STDOUT_FILENO);

		alarm(_timeout/1000);
		if (_fileEnding == ".py")
			execve("/usr/bin/python3", argv, env);

		else
			execve("/usr/bin/perl", argv, env);
		close(file);
		exit(1);
	}
	else
	{
		close(pipefd[1]);
		waitpid(pid, &status, 0);
		close(pipefd[0]);
	}
	close (file);;
	if(WIFSIGNALED(status))
	{
		std::cerr << RED << "TIMEOUT" << RESET_COLOR << std::endl;
		remove("www/tempCGI");
		return 504;
	}
	if (_method == "POST"){
		_responseStr = "HTTP/1.1 201 OK \r\nContent-Type: text/html\r\n";
		readFile("www/tempCGI");
		remove("www/tempCGI");
		return 200;
	}
	_responseStr = "HTTP/1.1 200 OK \r\nContent-Type: text/html\r\n";
	readFile("www/tempCGI");
	remove("www/tempCGI");
	return 200;
	}