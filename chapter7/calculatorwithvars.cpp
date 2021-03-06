
/*
	calculator08buggy.cpp

	grammer:
	statement:
		expression
		declaration
		assignment

	declaration:
		let name = expression

	assignment
		name = expression

	expression:
		term
		term + expression
		term - expression
	term:
		primary
		term * primary
		term / primary
	primary:
		number
		name
		'(' expression ')'
		'sqrt(' expression ')'
		- primary
		+ primary

*/

#include "../../std_lib_facilities.h"


struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) 
		:kind(ch), value(0) { }
	Token(char ch, double val) 
		:kind(ch), value(val) { }
	Token(char ch, string n) 
		:kind(ch), value(0), name(n) { } //first error
};

class Token_stream {
public:
	//Token_stream() :full(false), buffer(0) { }

	Token get(istream& input);
	void putback(Token t) { buffer.push_back(t); }
	void ignore(char c);
private:
	vector<Token> buffer;
};

const char let = 'L';
const char con = 'C';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';
const char square = 's';
const string declkey = "let";
const string constkey = "const";
const string quitkey = "quit";
const string squarekey = "sqrt";

Token Token_stream::get(istream& input)
{
	if (!buffer.empty()) {
		Token t = buffer.back();
		buffer.pop_back();
		return t;
	}
	char ch;
	input >> ch;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	input.unget();
	double val;
	input >> val;
	return Token(number, val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while (input.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch; //2nd error compilor won'catch
			input.unget();
			if (s == squarekey) return Token(square); 
			if (s == declkey) return Token(let); //another error? yes
			if (s == constkey) return Token(con);
			if (s == quitkey) return Token(quit); //another error
			return Token(name, s);
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c)
{
	while (!buffer.empty() && buffer.back().kind != c)
		buffer.pop_back();
	// Buffer contains a c kind token
	if (!buffer.empty()) return;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}

class Variable {
public:
	string name;
	double value;
	bool constant;
	//const bool isconst;
	Variable(string n, double v, bool c) :name(n), value(v), constant(c){ }
};

class Symbol_table {
public:
	double get(string s);
	void set(string s, double d);
	bool is_declared(string s);
	void declare(string s, double d, bool c);
private:
	vector<Variable> var_tabel;
};

double Symbol_table::get(string s)
{
	for (Variable e : var_tabel)
		if (e.name == s) return e.value;
	error("get: undefined name ", s);
}

void Symbol_table::set(string s, double d)
{
	for (Variable e : var_tabel)
		if (e.name == s) {
			if (e.constant) {
				error("variable you are trying to set is constant");
			}
			e.value = d;
			return;
		}
	error("set: undefined name ", s);
}

bool Symbol_table::is_declared(string s)
{
	for (Variable e : var_tabel)
		if (e.name == s) return true;
	return false;
}

void Symbol_table::declare(string s, double d, bool c)
{
	var_tabel.push_back(Variable(s, d, c));
}

Token_stream ts;
Symbol_table table;
istream& input = std::cin;

double expression();

double primary()
{
	Token t = ts.get(input);
	switch (t.kind) {
	case '(':
	{	double d = expression();
		t = ts.get(input);
		if (t.kind != ')') error("')' expected");
		return d; //error no retun
	}
	case '-':
		return -primary();
	case number:
		return t.value;
	case name:
		return table.get(t.name);
	case square:
	{
		t = ts.get(input);
		if (t.kind != '(') error("no opening parentheses after sqrt");
		double d = expression();
		t = ts.get(input);
		if (t.kind != ')') error("unbalanced parentheses after sqrt");
		if (d < 0) error("trying to take sqrt of negative expression");
		return sqrt(d);
	}
	default:
		error("primary expected");
	}
}

double term()
{
	double left = primary();
	while (true) {
		Token t = ts.get(input);
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
		if (d == 0) error("divide by zero");
		left /= d;
		break;
		}
		default:
			ts.putback(t);
			return left;
		}
	}
}

double expression()
{
	double left = term();
	while (true) {
		Token t = ts.get(input);
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.putback(t);
			return left;
		}
	}
}

double assignment()
{
	// We get there by knowing that a name and an '=' come next.
	Token t = ts.get(input);
	string var_name = t.name;
	if (!table.is_declared(var_name)) error(var_name, " has not been declared");
	ts.get(input); // Get rid of the '='
	double d = expression();
	table.set(var_name, d);
	return d;
}

double declaration()
{
	Token t1 = ts.get(input); //let or const
	Token t2 = ts.get(input); //name of var
	if (t2.kind != name) error("name expected in declaration");
	string name = t2.name;
	if (table.is_declared(name)) error(name, " declared twice");
	Token t3 = ts.get(input);
	if (t3.kind != '=') error("= missing in declaration of ", name);
	double d = expression();
	switch(t1.kind) {
	case let:
		table.declare(name, d, false);
	case con:
		table.declare(name, d, true);
	}
	return d;
}

double statement()
{
	Token t = ts.get(input);
	switch (t.kind) {
	case let: case con:
		ts.putback(t);
		return declaration();
	case name:
	{
		Token t2 = ts.get(input);
		// Whatever t2 is, we have to rollback
		ts.putback(t2);
		ts.putback(t);
		if (t2.kind == '=') {
			return assignment();
		}
		// We could move the putback(t) inside the if-statement and let this
		// fallback to the default case. I have my doubts that something like
		// that won't give us problems on the future. The current solution also
		// will be problematic if something else than an expression is
		// introduced as a statement.
		return expression();
	}
	default:
		ts.putback(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while (true) try {
		cout << prompt;
		Token t = ts.get(input);
		while (t.kind == print) t = ts.get(input);
		if (t.kind == quit) return;
		ts.putback(t);
		cout << result << statement() << endl;
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()

try {
	calculate();
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}