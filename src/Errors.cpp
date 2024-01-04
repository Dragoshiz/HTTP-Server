#include "../include/Client.class.hpp"
#include <map>

int Client::readError(int statuscode, std::string info){
	std::stringstream status;
	std::map<int, std::string>::iterator it;
	std::string path = "data/status_codes/";

	it = _errorMap.find(statuscode);
	if (it != _errorMap.end())
		path = it->second;
	else
		std::cout << "Error: status code not found" << std::endl;

	status << statuscode;

	std::stringstream buffer_html;
	std::ifstream htmlFile(path.c_str());

	if (!htmlFile.is_open())
	{
		std::cerr << "Couldn't access " << path << std::endl;
	}
	buffer_html << htmlFile.rdbuf();
	htmlFile.close();

	std::string htmlContent(buffer_html.str());

	int htmlContentLen = htmlContent.length();
	std::stringstream ss;
	ss << "Content-Length: " << htmlContentLen << "\r\n";
	this -> _responseStr = "HTTP/1.1 " + status.str() + " " + info 
					+ "\r\n" "Content-Type: text/html\r\n" + ss.str()+ "\r\n" + htmlContent;
	return statuscode;
}