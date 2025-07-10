#include "bencode.h"



void Bencode::Parser::AddString(const std::string& ref, std::ostream& os)
{

	for (auto c : ref)
	{
		ProcessChar(c);
	}
	Reset();
	GetHash();
	Print();
	PrintPieces();
}

void Bencode::Parser::Print()
{

	for (auto& element : m_items)
	{
		std::cout << element.raw_value << std::endl;
	}
}

inline std::string ToHex(const std::string& binary)
{
    std::ostringstream oss;
    for (unsigned char c : binary)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    return oss.str();
}

void Bencode::Parser::GetHash()
{
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (it->raw_value == "pieces") {
            auto next = std::next(it);
            if (next != m_items.end()) {
                m_hash = next->raw_value;
                m_items.erase(next);
            }
            break;
        }
    }
}

void Bencode::Parser::PrintPieces()
{
    constexpr size_t SHA1_HASH_SIZE = 20;
    if (m_hash.size() % SHA1_HASH_SIZE != 0)
        throw Exception("Each SHA-1 hash needs to be 20 bytes!");
    size_t hashAmount = m_hash.size() / SHA1_HASH_SIZE;
    for (size_t i = 0; i < hashAmount; ++i)
    {
        std::string hash = m_hash.substr(i * SHA1_HASH_SIZE, SHA1_HASH_SIZE);
        std::cout << "Piece " << (i + 1) << ":\t" << ToHex(hash) << std::endl;
        Sleep(200);
    }
}


void Bencode::Parser::AddFile(const std::filesystem::path& path)
{
    std::ifstream fileIn(path, std::ios::binary);
    if (!fileIn.is_open())
        throw Exception(std::format("File cannot be opened at {}", path.string()));
    fileIn.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(fileIn.tellg());
    fileIn.seekg(0, std::ios::beg);
    std::string fileData(fileSize, '\0');
    if (!fileIn.read(fileData.data(), fileSize))
        throw Exception("File read error!");
    AddString(fileData);
}

void Bencode::Parser::Reset()
{
	m_string = "";
	m_int = "";
	i = 0;
}




void Bencode::Parser::ProcessChar(const char& c)
{

	switch (m_state)
	{
	case State::ReadyForData:
	{

		Reset();
		switch (iscntrl(c))
		{
			return;
		}
		switch (c)
		{
		case 'i':
			m_state = State::Int;
			break;
		case 'l':
			m_state = State::List;
			break;
		case 'd':
			m_state = State::Dict;
			break;
		case 'e':
			break;
		default:
			if (isDigit(c))
			{
				m_state = State::Length;
				break;
			}
		}
	}
	break;
	case State::Int:
	{
		switch (c)
		{
		case 'e':
			m_items.push_back(BItem(m_int, 1));
			Reset();
			m_state = State::ReadyForData;
			break;
		default:
			if (isdigit(c))
				m_int += c;
			else
				throw Exception("Containing non-digit characters");
		}

	}
	break;
	case State::Length:
	{
		if (isdigit(c))
		{
			temp += c;
			break;
		}
		else if (c == ':')
			m_state = State::Str;
		else
			m_state = State::ReadyForData;
	}
	break;
	case State::Str:
	{

		for (; i < std::stoi(GetTemp());)
		{
			m_string += c;
			i++;
			break;
		}

		if (i == std::stoi(GetTemp()))
		{
			m_items.push_back(BItem(m_string, 0));
			m_state = State::ReadyForData;
		}

		break;

	}

	case State::List:
	{
		switch (c)
		{
		case 'e':
			m_state = State::ReadyForData;
			break;
		case 'i':
			m_state = State::Int;
		default:
			if (isDigit(c))
				break;
		}
	}
	break;
	case State::Dict:
	{
		switch (c)
		{
		case 'e':
			m_state = State::ReadyForData;
			break;
		case 'i':
			m_state = State::Int;
		default:
			if (isDigit(c))
				break;
		}
	}
	break;
	}
}
