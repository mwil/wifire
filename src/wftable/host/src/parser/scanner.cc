
#include "scanner.h"
#include <algorithm>
#include <stdexcept>

namespace Parser {

Scanner::Scanner() {}

Scanner::Scanner(const std::string& line, std::size_t begin, std::size_t end)
	: line(new std::string(line, begin, end - begin)), pos(0) {}

Scanner::Scanner(const std::string& line)
	: line(new std::string(line)), pos(0) {}

void Scanner::reset()
{
	pos = 0;
}

Token Scanner::pop()
{
	std::size_t startpos = line->find_first_not_of(' ', pos);
	if (startpos == std::string::npos) {
		pos = line->length();
		return Token();
	}

	if (line->at(startpos) == '"') {
		std::size_t endquote = startpos;
		bool quoted;
		try {
			do {
				endquote = line->find_first_of("\\\"", endquote + 1);
				quoted = false;
				while (line->at(endquote) == '\\') {
					quoted = !quoted;
					++endquote;
				}
			} while (line->at(endquote) != '"' && !quoted);
		} catch(std::out_of_range&) {
			pos = line->length();
			return Token(startpos, pos, line->substr(startpos + 1, pos - (startpos + 1)));
		}
		pos = endquote + 1;
		return Token(startpos, endquote + 1, line->substr(startpos + 1, endquote - startpos - 1));
	} else {
		pos = line->find_first_of(' ', startpos);
		if (pos == std::string::npos)
			pos = line->length();
		return Token(startpos, pos, line->substr(startpos, pos - startpos));
	}
}

void Scanner::prev()
{
	if (pos == std::string::npos)
		pos = line->length();
	pos = line->find_last_of(' ', pos - 1);
	if (pos == std::string::npos)
		pos = 0;
}

bool Scanner::atEnd() const
{
	return pos == line->length();
}

bool Scanner::atTokenEnd() const
{
	return pos == line->length() || line->find_first_not_of(' ', pos) == std::string::npos;
}

std::string Scanner::rest() const
{
	return line->substr(pos);
}

std::size_t Scanner::getPos() const
{
	return pos;
}

void Scanner::prettyPrint(std::ostream& stream, const std::string& text) const
{
	stream << text << " ";
	if (atEnd() || line->find_first_not_of(' ', pos) == std::string::npos) {
		stream << "at end of input" << std::endl;
	} else {
		Scanner p(*this);
		Scanner s(*this);
		p.prev();

		std::size_t tlen = text.length() + 2;
		std::size_t begin = p.pos;
		if (line->at(begin) == ' ') {
			begin = line->find_first_not_of(' ', begin);
			if (begin == std::string::npos)
				begin = line->length();
		}
		Token tk = s.pop();
		std::string prolog(tk.begin > begin ? line->substr(begin, tk.begin - begin) : "");
		std::string token(tk.token);
		std::string epilog(tk.end == line->length() ? "" : line->substr(tk.end, 5));

		if (1) { /* TODO: something better here */
			stream << "\"" << prolog << char(0x1b) << "[1;39;49m" <<
				token << char(0x1b) << "[0;39;49m" <<
				epilog << "\"" << std::endl;
		} else {
			stream << "\"" << prolog << token << epilog << "\"" << std::endl;
			stream << std::string(tlen + prolog.length(), ' ') <<
				std::string(tk.token.length(), '^') << std::endl;
		}
	}
}

bool Scanner::operator==(const Scanner& s) const
{
	if (this == &s)
		return true;
	return line == s.line && pos == s.pos;
}

bool Scanner::operator<(const Scanner& s) const
{
	return pos < s.pos;
}


}
