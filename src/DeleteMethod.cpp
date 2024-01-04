#include "../include/Client.class.hpp"

int Client::deleteMethod(){
	std::string path = _requestedFile;

	size_t pos = path.find("/storage");
	if(pos == std::string::npos){
		std::cout << RED << "Restricting to delete outside of storage" << RESET_COLOR << std::endl;
		return (readError(403, "Forbidden"));
	}

	if (std::remove(path.c_str()) != 0)
		return (readError(404, "Not Found"));

	return (readError(200, "OK"));
}