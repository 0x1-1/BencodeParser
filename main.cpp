#include "bencode.h"


int main()
{
	using namespace Bencode;
	Parser parser;
	parser.AddString("d7:meaningi42e4:wiki7:bencodee");
}