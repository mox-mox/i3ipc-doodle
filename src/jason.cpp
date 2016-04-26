#include "jason.hpp"
#include <json/json.h>
#include <iostream>

void write_json()
{
	//Json::Value vec(Json::arrayValue);
	//vec.append(Json::Value(1));
	//vec.append(Json::Value(2));
	//vec.append(Json::Value(3));
	//std::cout << vec;








// ---- create from scratch ----

Json::Value fromScratch;
Json::Value array;
array.append("hello");
array.append("world");
fromScratch["hello"] = "world";
fromScratch["number"] = 2;
fromScratch["array"] = array;
fromScratch["object"]["hello"] = "world";

//std::cout<<fromScratch<<std::endl;

//output(fromScratch);
//
// write in a nice readible way
Json::StyledWriter styledWriter;
std::cout << styledWriter.write(fromScratch);

// ---- parse from string ----

// write in a compact way
Json::FastWriter fastWriter;
std::string jsonMessage = fastWriter.write(fromScratch);

Json::Value parsedFromString;
Json::Reader reader;
bool parsingSuccessful = reader.parse(jsonMessage, parsedFromString);
if (parsingSuccessful)
{
    std::cout << styledWriter.write(parsedFromString) << std::endl;
}





}
