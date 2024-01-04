#include "../include/configParser.hpp"

#define PORT 8080
#define HOST "127.0.0.1"
#define URL "/upload/"

int main(int argc, char **argv)
{
	configParser parser;
	if (!parser.validConfig(argc, argv))
		return 1;

	parser.setData(URL, HOST, PORT); // TODO für den check immer bis zum nächsten slash oder ende der url  -> /upload/images/ oder /upload/images


	std::cout
	<< "getURL " << RED << parser.getUrl() << RESET_COLOR
	<< "\ngetAutoindex " << RED << parser.getAutoIndex() << RESET_COLOR
	<< "\nCurrentRoute: " << BOLDBLUE << parser.getCurrentRoute() << RESET_COLOR
	<< "\ngetIndexFile " << RED << parser.getIndexFile() << RESET_COLOR
	<< "\ngetPostAllowed " << RED << parser.getPostAllowed() << RESET_COLOR
	<< "\ngetDeleteAllowed " << RED << parser.getDeleteAllowed() << RESET_COLOR
	<< "\ngetGetAllowed " << RED << parser.getGetAllowed() << RESET_COLOR
	<< "\ngetBodySize " << RED << parser.getBodySize(PORT) << RESET_COLOR
	<< "\ngetHasRedirection " << RED << parser.getHasRedirection() << RESET_COLOR
	<< std::endl;

	std::cout << "\nPortVector ";
	IntVector::iterator it;
	for (it = parser.getPortVector().begin(); it != parser.getPortVector().end(); ++it)
		std::cout << RED << *it << " ";
	std::cout << RESET_COLOR << std::endl;

	std::cout << "\nCGIVector ";
	StringVector::iterator cgi_it;
	for (cgi_it = parser.getCgiExtensions().begin(); cgi_it != parser.getCgiExtensions().end(); ++cgi_it)
		std::cout << RED << *cgi_it << " ";
	std::cout << RESET_COLOR << std::endl;

	IntStringMap errormap = parser.getErrorMap();
	IntStringMap::iterator error_it;
	std::cout << "\nErrormap\n";
  	for (error_it = errormap.begin(); error_it != errormap.end(); ++error_it)
    	std::cout << error_it->first << " => " << error_it->second << '\n';
	return 0;
}
