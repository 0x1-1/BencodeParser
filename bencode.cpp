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
	if(!m_items.empty())
	WToFile();

}

void Bencode::Parser::Print()
{
	if (m_items.empty())
		std::cout << "NO DATA COULD BE DECODED! CHECK DATA AGAIN." << std::endl;
	
		for (auto& element : m_items)
		{
			std::cout << element.raw_value << std::endl;
			m_holder += element.raw_value + "\n";
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
		m_holder += ToHex(hash) + "\n";       
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

void Bencode::Parser::WToFile()
{
	std::cout << "\tDo you want to save the decoded info in a text file? Enter Y to continue.\t" << std::endl;
	char ch;
	std::cin >> ch;
	if (toupper(ch) == 'Y')
	{
		std::cout << "Enter the name for the text file." << std::endl << std::endl;
		
		OPENFILENAMEW file = { sizeof(OPENFILENAMEW) };
		file.lpstrFilter = L"Text Files (*txt)\0*.txt\0All Files\0*.*\0\0";
		wchar_t buff[MAX_PATH] = {};
		file.nMaxFile = sizeof(buff);
		file.lpstrFile = buff;
		file.lpstrDefExt = L"txt";
		file.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		BOOL f_save = GetSaveFileNameW(&file);
		if (f_save != 0)
		{
			std::wcout << "The file will be saved in directory: " << buff << std::endl;
		}
		LPCWSTR f_Name = buff;
		HANDLE hFile = CreateFileW
		(
			f_Name,
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			std::cout << "Failed to open or create file" << std::endl;
			std::cout << GetLastError();
			return;
		}
		const char* msg = m_holder.data();
		DWORD bytesWritten;
		BOOL wFile = WriteFile(
			hFile,
			msg,
			strlen(msg),
			&bytesWritten,
			NULL
		);

		if (wFile != 0)
		{
			std::cout << "Written to file successfully!" << std::endl;
		}

		else if (wFile == 0)
		{
			std::cout << "Could not write to file!" << std::endl;
			std::cout << GetLastError();
		}

		CloseHandle(hFile);

		std::cout << "Thank you for using the program." << std::endl;
	}

	else
		std::cout << "Thank you for using the program." << std::endl;
	
	
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
				temp = c;
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
			{
				m_state = State::Length;
				temp = c;
				break;
			}
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
			{ 
				m_state = State::Length;
				temp = c;
				break;
			}
				
		}
	}
	break;
	}
}
