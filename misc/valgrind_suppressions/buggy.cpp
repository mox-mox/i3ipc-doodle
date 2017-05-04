#include <string>
#include <iostream>


std::string* hello = new std::string("Hello world!");

int main(int argc, char * argv[] )
{
	std::cout<<*hello<<std::endl;
    return 0;
}
