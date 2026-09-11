#include <iostream>
#include <queue>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <vector>
using namespace std;
typedef enum TokenType {
    LEFT_PAREN, 
    RIGHT_PAREN, 
    INT,       
    STRING, ERROR,   
    DOT,
    FLOAT,     
    NIL,       
    T,          
    QUOTE,
    SYMBOL,
    ERRORTOKEN
    
} TokenType;

typedef struct Token {
    TokenType type;
    string tokenInfo;
    int startLine;
    int startColumn;
    int endLine;
    int endColumn;
} Token;
// Scanner layer error
class NoClosingQuote: public runtime_error{
public:
    NoClosingQuote(Token tk): runtime_error("ERROR (no closing quote) : END-OF-LINE encountered at Line "
                                       + to_string(tk.endLine) + " Column " + to_string(tk.endColumn)) {}
};
// end Scanner layer error
// Parser layer error
class ExpectedAtomOrLeftParen: public runtime_error{
public:
    ExpectedAtomOrLeftParen(Token tk): 
    runtime_error("ERROR (unexpected token) : atom or '(' expected when token at Line "
                + to_string(tk.startLine) + " Column " + to_string(tk.startColumn) + 
            " is >>" + tk.tokenInfo + "<<"){}
};
class ExpectedRightParen: public runtime_error{
public:
    ExpectedRightParen(Token tk):
    runtime_error("ERROR (unexpected token) : ')' expected when token at Line "
                + to_string(tk.startLine) + " Column " + to_string(tk.startColumn) + 
                " is >>" + tk.tokenInfo + "<<"){}
};
class UnexpectedCharacter: public runtime_error {
public:
};
// end Parser layer error

// EOF error
class NoMoreInput: public runtime_error{
public:
    NoMoreInput(): runtime_error("ERROR (no more input) : END-OF-FILE encountered"){}
};
class UnexpectedError: public runtime_error{
public:
    UnexpectedError(): runtime_error("---something goes wrong---"){}
};
// end EOF error
class Scanner {
private:
    // 0.data member
    int currentLine;
    int currentColumn;
    queue<Token> tokensBuffer;
    bool pendingLineReset;
    // 1. get character zone
    // (Get, Peek, Skip)
    bool GetNextChar(char& ch) {
        int c = cin.get();
        if (c == EOF)
            return false;
        //if (c == '\r') return GetNextChar(ch);
        ch = static_cast<char>(c);
        currentColumn++;
        if (ch == '\n') {
            currentColumn = 0;
            if (pendingLineReset) pendingLineReset = false;
            else currentLine++;
        }
        //cout << "[CharGetterGetChar]:{"<< ch<<"}(" << currentLine << ", " << currentColumn<< ")\n"; 
        return true;
    }
    bool PeekNextChar(char& ch) {
        int c = cin.peek();
        if (c == EOF)
            return false;

        ch = static_cast<char>(c);
        return true;
    }
    void SkipWhiteSpaceAndComment() {
        while (true) {
            char c;
            if(!PeekNextChar(c)) return;
            if (c == ' ' || c == '\t' || c == '\n') {
                GetNextChar(c);
            } else if (c == ';') {
                while (GetNextChar(c) && c != '\n');
            } else break;
        }
        return;
    }
    // 2.Helper Function zone
    // (MakeToken)
    Token MakeToken(TokenType _type, string _tokenInfo, int _startLine, int _startColumn, int _endLine, int _endColumn) {
        Token token;
        token.type = _type;
        token.tokenInfo = _tokenInfo;
        token.startLine = _startLine;
        token.startColumn = _startColumn;
        token.endLine = _endLine;
        token.endColumn = _endColumn;
        return token;
    }
    // 3.Helper Function zone
    // (Token Reader function)

    Token ReadNumberOrDot(char firstChar, int startLine, int startColumn) {
        string buffer;
        buffer += firstChar;
        bool hasDot = (firstChar == '.');
        int dotCount = (firstChar == '.')? 1: 0;
        bool hasDigit = isDigit(firstChar);
        bool firstCharIsPlusOrMinus = (firstChar == '+' || firstChar == '-');
        bool hasOther = false;
        // loop
        while(true) {
            char ch;
            if(!PeekNextChar(ch)) break;
            if (isSeparator(ch)) break;  // illegal ch
            GetNextChar(ch);
            buffer += ch;
            if (isDigit(ch)) {
                hasDigit = true;
            }
            else if (ch == '.') {
                hasDot = true;
                dotCount++;
            }
            else hasOther = true;
            
        }
        // conditional return
        int endLine = currentLine;
        int endColumn = currentColumn;
        if (hasDigit && !hasDot && !hasOther) {
            return MakeToken(INT, buffer, startLine, startColumn, endLine, endColumn);
        }
        if (hasDigit && hasDot && dotCount == 1 && !hasOther) {
            return MakeToken(FLOAT, buffer, startLine, startColumn, endLine, endColumn);
        }
        if (!hasDigit && hasDot && dotCount == 1 && !hasOther && !firstCharIsPlusOrMinus) {
            return MakeToken(DOT, buffer, startLine, startColumn, endLine, endColumn);
        }
        return MakeToken(SYMBOL, buffer, startLine, startColumn, endLine, endColumn);

    }
    Token ReadTOrNil(char firstChar, int startLine, int startColumn) {
        string buffer;
        buffer += firstChar;
        while (true) {
            char ch;
            if(!PeekNextChar(ch)) break;
            if (isSeparator(ch)) break;
            GetNextChar(ch);
            buffer += ch;
        }
        int endLine = currentLine;
        int endColumn = currentColumn;
        if (buffer == "#t" || buffer == "t") {
            return MakeToken(T, buffer, startLine, startColumn, endLine, endColumn);
        }
        if (buffer == "nil" || buffer == "#f") {
            return MakeToken(NIL, buffer, startLine, startColumn, endLine, endColumn);
        }
        return MakeToken(SYMBOL, buffer, startLine, startColumn, endLine, endColumn);
    }
    Token ReadStringOrErrorToken(char firstChar, int startLine, int startColumn) {
        string buffer;
        buffer += firstChar;
        bool isString = false;
        bool hasPreviousBackSlash = false;
        
        while(true) {
            char ch;
            if(!PeekNextChar(ch)) break;
            if (ch == '\n') {
                break;
            }
            if (ch == '"') {
                GetNextChar(ch);
                buffer += ch;
                isString = true;
                break;
            }
            if (ch == '\\') {
                //if () 
                char temp = buffer[buffer.size() - 1];
                GetNextChar(ch);
                buffer += ch;
                if(!PeekNextChar(ch)) break;
                if (ch == '\\') {
                    GetNextChar(ch);
                    buffer += ch;
                    continue;
                }
                if (ch == '"') {
                    GetNextChar(ch);
                    buffer += ch;
                }
                continue;               
            }
            GetNextChar(ch); 
            buffer += ch;
        }
        int endLine = currentLine;
        int endColumn = currentColumn;
        if (isString) {
            return MakeToken(STRING, buffer, startLine, startColumn, endLine, endColumn);
        }
        endColumn++;
        return MakeToken(ERRORTOKEN, buffer, startLine, startColumn, endLine, endColumn);

    }

    Token ReadSymbol(char firstChar, int startLine, int startColumn) {
        string buffer;
        buffer += firstChar;
        while (true) {
            char ch;
            if(!PeekNextChar(ch)) break;
            if (isSeparator(ch)) break;
            GetNextChar(ch);
            buffer += ch;
        }
        int endLine = currentLine;
        int endColumn = currentColumn;
        return MakeToken(SYMBOL, buffer, startLine, startColumn, endLine, endColumn);
    }
    // 4. others
    bool isDigit(char ch) {return (ch<= '9' && ch >= '0');}
    bool isSeparator(char ch) {
        return (ch == ' ' || ch == '\t' || ch == '\n' || //ch == '\r' ||
                ch == '(' || ch == ')'  ||
                ch == '"' || ch == '\'' ||
                ch == ';');
    }
public:
    Scanner(): currentLine(1), currentColumn(0), tokensBuffer(), pendingLineReset(false){}
    Token GetToken() {
        // 0. non empty tokensBuffer case
        if (!tokensBuffer.empty()) {
            Token token = tokensBuffer.front();
            tokensBuffer.pop();
            return token;
        }

        // 1. Skip White Space And Command
        SkipWhiteSpaceAndComment();

        pendingLineReset = false;

        // 2. Read First Char
        char ch;
        if (!GetNextChar(ch)) {
            throw NoMoreInput();
        }
        int startLine = currentLine;
        int startColumn = currentColumn;
        
        // A. LEFT_PAREN, RIGHT_PAREN, QUOTE
        if (ch == '(') {
            return MakeToken(LEFT_PAREN, "(", startLine, startColumn, startLine, startColumn);            
        }
        if (ch == ')') {
            return MakeToken(RIGHT_PAREN, ")", startLine, startColumn, startLine, startColumn);
        }
        if (ch == '\'') {
            return MakeToken(QUOTE, "\'", startLine, startColumn, startLine, startColumn);
        }
        // B. INT, FLOAT, DOT, SYMBOL
        if (isDigit(ch) || ch == '+' || ch == '-' || ch == '.') {
            return ReadNumberOrDot(ch, startLine, startColumn);
        }
        // C. NIL, T, SYMBOL
        if (ch == '#' || ch == 'n' || ch == 't') {
            return ReadTOrNil(ch, startLine, startColumn);
        }
        // D. STRING, ERRORTOKEN
        if (ch == '"') {
            return ReadStringOrErrorToken(ch, startLine, startColumn);
        }
        // E. LEFT_PAREN, NIL
        //if (ch == '(') {
        //    return ReadLeftParenOrNil(ch, startLine, startColumn);
        //}
        // F. SYMBOL
        return ReadSymbol(ch, startLine, startColumn);
    }
    Token PeekToken() {
        if (!tokensBuffer.empty()) {
            return tokensBuffer.front();
        }
        Token token = GetToken();
        tokensBuffer.push(token);
        return token;
    }
    void ResetCurrentLineAndColumn(bool alreadyConsumedNewline = false) {
        currentLine = 1;
        currentColumn = 0;
        char ch;
        pendingLineReset = !alreadyConsumedNewline;
    }
    void SkipRestOfLine() {
        char ch;
        while (true) {
            if (!PeekNextChar(ch)) break;
            
            GetNextChar(ch);
            if (ch == '\n') break; 
        }
    }
    void ResetForRead() {
        currentLine = 1;
        currentColumn = 0;
        pendingLineReset = false;
        while (!tokensBuffer.empty()) tokensBuffer.pop();
    }
    int GetLine() {
        return currentLine;
    }
    int GetColumn() {
        return currentColumn;
    }
};


typedef enum FormType{
    // special form
    FORM_QUOTE,
    FORM_DEFINE,
    FORM_IF,
    FORM_COND,
    FORM_AND,
    FORM_OR,
    FORM_BEGIN,
    FORM_LAMBDA, FORM_CLOSURE,
    FORM_LET,
    FORM_SET,
    // primitive expression
    FORM_CONS,
    FORM_LIST,
    FORM_CAR,
    FORM_CDR,
    FORM_ARITHMETIC,
    FORM_PREDICATE,
    FORM_COMPARE,
    FORM_STRING,
    FORM_EQUIV,
    FORM_NOT,
    FORM_CLEAN_ENVIRONMENT,
    FORM_EXIT,
    FORM_VERBOSE,
    FORM_VERBOSE_QUESTION,
    FORM_READ,
    FORM_CREATE_ERROR_OBJECT,
    FORM_ERROR_OBJECT_QUESTION,
    FORM_WRITE,
    FORM_DISPLAY_STRING,
    FORM_NEWLINE,
    FORM_SYMBOL_TO_STRING,
    FORM_NUMBER_TO_STRING,
    FORM_EVAL,
    
    // form default
    FORM_DEFAULT
    
}FormType;
struct Environment;
struct Node;
struct Closure {
    string functionName;
    vector<string> parameters;
    shared_ptr<Node> body;
    shared_ptr<Environment> curEnvironment;

};
typedef struct Node {
    
    shared_ptr<FormType> formtypeptr = nullptr;   
    shared_ptr<Token> tokenptr = nullptr;           
    shared_ptr<Node> car = nullptr;
    shared_ptr<Node> cdr = nullptr;
    shared_ptr<Closure> closureptr = nullptr; // <--------- to record closure 


    bool IsAtom() const {return tokenptr != nullptr;}
    bool IsPair() const {return car != nullptr;}
    bool IsNil() const {return IsAtom() && tokenptr->type == NIL;}
    bool IsClosure() const {return closureptr != nullptr;}
    bool IsEmptyNode() const{ return !formtypeptr && !tokenptr && !car && !cdr && !closureptr;}
    Node(): formtypeptr(nullptr), tokenptr(nullptr), car(nullptr), cdr(nullptr), closureptr(nullptr) {}
    Node(Token t): formtypeptr(nullptr), tokenptr(make_shared<Token>(t)), car(nullptr), cdr(nullptr) {}
    Node(Token t, FormType ft): formtypeptr(make_shared<FormType>(ft)), tokenptr(make_shared<Token>(t)), car(nullptr), cdr(nullptr) {}

} Node;

struct Environment {

    unordered_map<string, shared_ptr<Node>> table;
    shared_ptr<Environment> parent = nullptr;
    shared_ptr<Node> FindVariable(string var, shared_ptr<Environment> env) {
        
        for ( ; env!=nullptr; env = env->parent) {
            auto it = env->table.find(var);
            if (it != env->table.end())
                return it->second;
        }
        return nullptr;
    }
    void SetVariable(string variable, shared_ptr<Node> value, shared_ptr<Environment> env) {

        for (auto cur = env; cur != nullptr; cur = cur->parent) {
            auto it = cur->table.find(variable);
            if (it != cur->table.end()) {
                it->second = value;  
                return;
            }
        }
        while (env->parent != nullptr) env = env->parent;
        env->table[variable] = value;
    }
};

class Parser {
private:
    Scanner sc;
    shared_ptr<Node> root;
    shared_ptr<Node> GetQuoExp(Token token) {
        if (token.type != QUOTE) throw UnexpectedError();
        auto n = make_shared<Node>();

        Token quoteSymToken = {SYMBOL, "quote",
                               token.startLine, token.startColumn,
                               token.endLine,   token.endColumn};
        n->car = make_shared<Node>(quoteSymToken);

        auto cdrNode = make_shared<Node>();
        cdrNode->car = GetAnSExpression();
        if (!cdrNode->car) throw UnexpectedError();
   
        Token nilTk = {NIL, "nil", -1, -1, -1, -1};
        cdrNode->cdr = make_shared<Node>(nilTk);

        n->cdr = cdrNode;
        return n;

    }
    shared_ptr<Node> GetLRExp(Token token) {
        if (token.type != LEFT_PAREN)
            throw UnexpectedError();
        
        // ATOM: NIL "()"
        Token peekedToken = sc.PeekToken();
        if (peekedToken.type == RIGHT_PAREN) {
            Token token2 = sc.GetToken();
            Token nilToken = {NIL, "nil", token.startLine,
                                        token.startColumn,
                                        token2.startLine,
                                        token2.startColumn};
            return make_shared<Node>(nilToken);
        }
        // ----------------------------------------------------------
        auto n = make_shared<Node>();
    
        n->car = GetAnSExpression();
        if (!n->car) throw UnexpectedError();
        auto cur = n;
        while (true) {
            Token peekedToken = sc.PeekToken();
            // termination condition 1: LIST
            if (peekedToken.type == RIGHT_PAREN) {
                sc.GetToken();
                Token tk = {NIL, "nil", -1, -1, -1, -1};
                cur->cdr = make_shared<Node>(tk);
                return n;
            }
            // termination condition 2: DOTTED PAIR
            if (peekedToken.type == DOT) {
                // get DOT
                sc.GetToken();
                // error catch:
                Token peekedToken = sc.PeekToken();
                if (peekedToken.type == RIGHT_PAREN) {
                    throw ExpectedAtomOrLeftParen(sc.GetToken());
                }
                cur->cdr = GetAnSExpression();
                if (!cur->cdr) throw UnexpectedError();
                // error catch:
                peekedToken = sc.PeekToken();
                if (peekedToken.type != RIGHT_PAREN) {
                    throw ExpectedRightParen(sc.GetToken());
                } else sc.GetToken();
                return n;
                
            }

            // normal element
            
            cur->cdr = make_shared<Node>();
            cur = cur->cdr;
            auto nextNodePtr = GetAnSExpression();
            if (!nextNodePtr) throw UnexpectedError();
            cur->car = nextNodePtr;
            
        }
        // ----------------------------------------------------------
    }

public:
    Parser(): root(nullptr){}
    shared_ptr<Node> GetRoot() {return root;}
    shared_ptr<Node> GetAnSExpression() {
        Token peekedToken = sc.PeekToken();
        // token's error(Scanner layer)
        if (peekedToken.type == ERRORTOKEN) {
            throw NoClosingQuote(sc.GetToken());
        }
        // S-Expression type 1: ATOM
        if (peekedToken.type == SYMBOL || peekedToken.type == INT || peekedToken.type == FLOAT || 
            peekedToken.type == STRING || peekedToken.type == NIL || peekedToken.type == T) {
            
            return make_shared<Node>(sc.GetToken());
        }  // end ATOM
        // S-Expression type 2: QUOEXP
        if (peekedToken.type == QUOTE) {
            return GetQuoExp(sc.GetToken());
        }
        // S-Expression type 3: LREXP
        if (peekedToken.type == LEFT_PAREN) {
            return GetLRExp(sc.GetToken());
        }
        // syntax error(Parser layer)
        throw ExpectedAtomOrLeftParen(sc.GetToken());
    }
    void ReadAnSExpression() {
        root = nullptr;
        bool hasError = false;
        try {            
            root = this->GetAnSExpression();
        } catch (NoClosingQuote& e) {
            cout << e.what()<< "\n";
            sc.SkipRestOfLine();
            hasError = true;
        } catch (ExpectedAtomOrLeftParen& e) {
            cout << e.what()<< "\n";
            sc.SkipRestOfLine();
            hasError = true;
        } catch (ExpectedRightParen& e) {
            cout << e.what()<< "\n";
            sc.SkipRestOfLine();
            hasError = true;            
        }
        sc.ResetCurrentLineAndColumn(hasError);   
    }
    void SkipRestOfLine() {
        sc.SkipRestOfLine();
        sc.ResetCurrentLineAndColumn(true);
    }
    void ResetForRead() {
        sc.ResetForRead();
    }
    bool IsExitCommand() {
        if (!root) return false;
        if (root->car && root->car->tokenptr && root->car->tokenptr->tokenInfo == "exit" &&
            root->cdr && root->cdr->tokenptr && root->cdr->tokenptr->type == NIL) {
             return true;
        } 
        return false;
    }
};
//----------------------------------------------------------------
class Printer {
private:
    long long printStacker;

    void PrintAtom(shared_ptr<Node> cur, bool newline) {
        const string& info = cur->tokenptr->tokenInfo;
        switch (cur->tokenptr->type) {
            case NIL:    cout << "nil"; break;
            case T:      cout << "#t";  break;
            case SYMBOL: cout << info;  break;
            case INT:    cout << stoll(info); break;
            case FLOAT:  cout << fixed << setprecision(3) << stod(info); break;
            case STRING:    case ERROR:
                for (int i = 0; i < (int)info.size(); i++) {
                    if (info[i] == '\\' && i + 1 < (int)info.size()) {
                        switch (info[i+1]) {
                            case 'n':  cout << '\n'; i++; break;
                            case 't':  cout << '\t'; i++; break;
                            case '"':  cout << '"';  i++; break;
                            case '\\': cout << '\\'; i++; break;
                            default:   cout << info[i]; break; 
                        }
                    } else {
                        cout << info[i];
                    }
                }
                break;
            default: break;
        }
        if (newline) cout << "\n";
    }

    void PrintLRExp(shared_ptr<Node> cur, bool newline) {
        printStacker++;
        cout << "( ";
        this->PrettyPrint(cur->car); // print sexp
       
        while (true) {

            cur = cur->cdr;
            // termination condition1: list (nil end)
            if (cur && cur->IsNil() ) break; 
            // termination condition2: dotted pair (not nil end)
            if (cur && cur->IsAtom() && !cur->IsNil()) {
                for (int i = 0; i < printStacker; i++) cout << "  ";
                cout << ".\n";
                for (int i = 0; i < printStacker; i++) cout << "  ";
                this->PrettyPrint(cur);
            
                break;
            }

            if (cur->car != nullptr) {
                for (int i = 0; i < printStacker; i++) cout << "  ";
                this->PrettyPrint(cur->car);  // print sexp
            
            }
        }
        printStacker--;
        for (int i = 0; i < printStacker; i++) cout << "  ";
        cout << ")";
        if (newline) cout << "\n";
    }
public:
    Printer() : printStacker(0) {}

    void PrettyPrint(shared_ptr<Node> cur, bool newline = true) {
        if (!cur) throw UnexpectedError();
        if (cur->IsEmptyNode()) return;
        if (cur && cur->IsAtom()) PrintAtom(cur, newline);
        else               PrintLRExp(cur, newline);
    }
};
//----------------------------------------------------------------
// *************************Project 2 error*************************
//1
class NonList: public runtime_error {
public:
    shared_ptr<Node> root;
    NonList(shared_ptr<Node> _root): root(_root), runtime_error("ERROR (non-list) : ") {}

};
//2
class IncorrectNumberOfArguments: public runtime_error {
public:
    static string formatStr(const string str) {
        const string prefix = "#<procedure ";
        if (str.size() >= 13 && 
            str.compare(0, prefix.size(), prefix) == 0 && 
            str.back() == '>') {
            
            return str.substr(prefix.size(), str.size() - prefix.size() - 1);
        }
        return str;   
    }
    IncorrectNumberOfArguments(string str):
    runtime_error("ERROR (incorrect number of arguments) : " + formatStr(str)){}
};
//3
class WithIncorrectArgumentType: public runtime_error {
public:
    static string formatStr(const string str) {
        const string prefix = "#<procedure ";
        if (str.size() >= 13 && 
            str.compare(0, prefix.size(), prefix) == 0 && 
            str.back() == '>') {
            
            return str.substr(prefix.size(), str.size() - prefix.size() - 1);
        }
        return str;   
    }
    shared_ptr<Node> sexpStart;
    WithIncorrectArgumentType(string str, shared_ptr<Node> root):
    sexpStart(root),
    runtime_error("ERROR (" + formatStr(str) + " with incorrect argument type) : "){}
};
//4
class AttemptToApplyNonFunction: public runtime_error {
public:
    shared_ptr<Node> startNode;
    AttemptToApplyNonFunction(shared_ptr<Node> root):
    startNode(root), runtime_error("ERROR (attempt to apply non-function) : "){}
};
//5
class NoReturnValue: public runtime_error {
public:
    shared_ptr<Node> sexpStart;
    NoReturnValue(shared_ptr<Node> root):
    sexpStart(root),
    runtime_error("ERROR (no return value) : "){}
};
//6
class UnboundSymbol: public runtime_error {
public:

    UnboundSymbol(string str): 
    runtime_error("ERROR (unbound symbol) : " + str){}
};
//7
class DivisionByZero: public runtime_error {
public:
    DivisionByZero():
    runtime_error("ERROR (division by zero) : /"){}
};
//8
class DefineFormat: public runtime_error {
public:
    shared_ptr<Node> root;
    DefineFormat(shared_ptr<Node> _root):
    runtime_error("ERROR (DEFINE format) : "), root(_root){}
};
//9
class CondFormat: public runtime_error {
public:
    shared_ptr<Node> root;
    CondFormat(shared_ptr<Node> _root):
    runtime_error("ERROR (COND format) : "), root(_root){}
};

//10
class ExitOurScheme: public runtime_error {
public:
    ExitOurScheme():
    runtime_error(""){}
};

//11
class LevelOfExit: public runtime_error {
public:
    LevelOfExit():
    runtime_error("ERROR (level of EXIT)"){}
};
//12
class LevelOfDefine:public runtime_error {
public:
    LevelOfDefine():
    runtime_error("ERROR (level of DEFINE)"){}
};
//13
class LevelOfCleanEnvironment: public runtime_error {
public:
    LevelOfCleanEnvironment():
    runtime_error("ERROR (level of CLEAN-ENVIRONMENT)"){}

};
// *************************Project 2 error*************************
// *************************Project 3 error*************************
//14
class UnboundParameter: public runtime_error {
public:
    shared_ptr<Node> root;

    UnboundParameter(shared_ptr<Node> _root):
    runtime_error("ERROR (unbound parameter) : "), root(_root){}
};
//15
class UnboundTestCondition: public runtime_error {
public:
    shared_ptr<Node> root;
    UnboundTestCondition(shared_ptr<Node> _root):
    runtime_error("ERROR (unbound test-condition) : "), root(_root){}
};
//16
class UnboundCondition: public runtime_error {
public:
    shared_ptr<Node> root;
    UnboundCondition(shared_ptr<Node> _root):
    runtime_error("ERROR (unbound condition) : "), root(_root){}
};
//17
class LambdaFormat: public runtime_error {
public:
    shared_ptr<Node> root;
    LambdaFormat(shared_ptr<Node> _root):
    runtime_error("ERROR (LAMBDA format) : "), root(_root){}
};

//18
class LetFormat: public runtime_error {
public:
    shared_ptr<Node> root;
    LetFormat(shared_ptr<Node> _root):
    runtime_error("ERROR (LET format) : "), root(_root){}
};
// *************************Project 3 error*************************
// *************************Project 4 error*************************
class SetFormat: public runtime_error {
public:
    shared_ptr<Node> root;
    SetFormat(shared_ptr<Node> _root):
    runtime_error("ERROR (SET! format) : "), root(_root){}
};

// *************************Project 4 error*************************
class Evaluator {
private:
    Parser ps;
    shared_ptr<Node> root;
    Printer pt;
    shared_ptr<Environment> baseEnvironment;
    
    bool verbose = true;
    void CheckNonList(shared_ptr<Node> root) {
        for (auto cur = root; cur; cur = cur->cdr) {
            if (cur && cur->IsAtom()) {
                if (cur && !cur->IsNil()) throw NonList(root);
                return; 
            }
        }
    }
    void CheckNumberOfArguments(shared_ptr<Node> root, string type_str, int num_of_argu) const {
        shared_ptr<Node> cur = root->cdr;
        for (int i = 0; i < num_of_argu; i++) {
            if (!cur || (cur && cur->IsNil())) throw IncorrectNumberOfArguments(type_str);
            cur = cur->cdr;
        }
        if ( cur && !cur->IsNil()) throw IncorrectNumberOfArguments(type_str);
    }

    void CheckIfLessThanNArguments(shared_ptr<Node> root, string type_str, int N) const {
        if (N == 0) return;
        shared_ptr<Node> cur = root->cdr;
        for (int i = 0; i < N; i++) {
            if (!cur ||(cur && cur->IsNil())) throw IncorrectNumberOfArguments(type_str);
            cur = cur->cdr;
        }
        return;
    }
    void CheckIfMoreThanNArguments(shared_ptr<Node> root, string type_str, int N) const {
        shared_ptr<Node> cur = root->cdr;
       
        for (int i = 0; i < N; i++) {
            if (!cur ||(cur && cur->IsNil())) return; 
            cur = cur->cdr;
        }
        if (cur && !cur->IsNil()) throw IncorrectNumberOfArguments(type_str);
        
        return;
    }
    void CheckLevel(FormType ft, bool isTopLevel) const {
        if (!isTopLevel) 
            switch(ft) {
                case FORM_EXIT: throw LevelOfExit();
                case FORM_DEFINE: throw LevelOfDefine();
                case FORM_CLEAN_ENVIRONMENT: throw LevelOfCleanEnvironment();
                default: break;
            }
        return;
    }
    shared_ptr<Node> TrueNodeGenerator() {
        shared_ptr<Node> trueNode = make_shared<Node>();
        Token tk = {T, "#t", -1, -1, -1, -1};
        trueNode->tokenptr = make_shared<Token>(tk);
        return trueNode; 
    }
    shared_ptr<Node> NilNodeGenerator() {
        shared_ptr<Node> nilNode = make_shared<Node>();
        Token tk = {NIL, "nil", -1, -1, -1, -1};
        nilNode->tokenptr = make_shared<Token>(tk);
        return nilNode;
    }
    shared_ptr<Node> EmptyNodeGenerator() {
        shared_ptr<Node> emptyNode = make_shared<Node>();
        emptyNode->formtypeptr = nullptr;
        emptyNode->tokenptr = nullptr;
        emptyNode->car = nullptr;
        emptyNode->cdr = nullptr;
        emptyNode->closureptr = nullptr;
        return emptyNode;
    }
    shared_ptr<Node> MakeProcedureNode(FormType ft, const string& name) {
        auto node = make_shared<Node>();
        node->formtypeptr = make_shared<FormType>(ft);
        Token tk = {SYMBOL, "#<procedure " + name + ">", -1, -1, -1, -1};
        node->tokenptr = make_shared<Token>(tk);
        return node;
    }

    FormType IdentifyForm(const shared_ptr<Node>& root) {
        if (!root->tokenptr) throw UnexpectedError();
        // special form
        if (root->tokenptr->tokenInfo == "quote") return FORM_QUOTE;
        if (root->tokenptr->tokenInfo == "define") return FORM_DEFINE;
        if (root->tokenptr->tokenInfo == "if") return FORM_IF;
        if (root->tokenptr->tokenInfo == "cond") return FORM_COND;
        if (root->tokenptr->tokenInfo == "and") return FORM_AND;
        if (root->tokenptr->tokenInfo == "or") return FORM_OR;
        if (root->tokenptr->tokenInfo == "begin") return FORM_BEGIN;
        if (root->tokenptr->tokenInfo == "lambda" && !root->IsClosure()) return FORM_LAMBDA;
        if (root->tokenptr->tokenInfo == "lambda" && root->IsClosure()) return FORM_CLOSURE;
        if (root->tokenptr->tokenInfo == "let") return FORM_LET;
        // primitive expression
        if (root->tokenptr->tokenInfo == "cons") return FORM_CONS;
        if (root->tokenptr->tokenInfo == "list") return FORM_LIST;
        if (root->tokenptr->tokenInfo == "car") return FORM_CAR;
        if (root->tokenptr->tokenInfo == "cdr") return FORM_CDR;
        if (root->tokenptr->tokenInfo == "+" || root->tokenptr->tokenInfo == "-" || root->tokenptr->tokenInfo == "*" || root->tokenptr->tokenInfo == "/")
            return FORM_ARITHMETIC;
        if (root->tokenptr->tokenInfo == "atom?" ||root->tokenptr->tokenInfo == "pair?" ||root->tokenptr->tokenInfo == "list?" ||
            root->tokenptr->tokenInfo == "null?" ||root->tokenptr->tokenInfo == "integer?" ||root->tokenptr->tokenInfo == "real?" ||
            root->tokenptr->tokenInfo == "number?" ||root->tokenptr->tokenInfo == "string?" ||root->tokenptr->tokenInfo == "boolean?" ||
            root->tokenptr->tokenInfo == "symbol?")
            return FORM_PREDICATE;
        if (root->tokenptr->tokenInfo == "=" || root->tokenptr->tokenInfo == ">" || root->tokenptr->tokenInfo == "<" ||
            root->tokenptr->tokenInfo == ">=" || root->tokenptr->tokenInfo == "<=")
            return FORM_COMPARE;
        if (root->tokenptr->tokenInfo == "string-append" || root->tokenptr->tokenInfo == "string>?" ||
            root->tokenptr->tokenInfo == "string<?" || root->tokenptr->tokenInfo == "string=?")
            return FORM_STRING;
        if (root->tokenptr->tokenInfo == "eqv?" || root->tokenptr->tokenInfo == "equal?")
            return FORM_EQUIV;
        if (root->tokenptr->tokenInfo == "not")
            return FORM_NOT;
        if (root->tokenptr->tokenInfo == "clean-environment")
            return FORM_CLEAN_ENVIRONMENT;
        if (root->tokenptr->tokenInfo == "exit")
            return FORM_EXIT;
        if (root->tokenptr->tokenInfo == "verbose")
            return FORM_VERBOSE;
        if (root->tokenptr->tokenInfo == "verbose?")
            return FORM_VERBOSE_QUESTION;
        if (root->tokenptr->tokenInfo == "read")
            return FORM_READ;
        if (root->tokenptr->tokenInfo == "create-error-object")
            return FORM_CREATE_ERROR_OBJECT;
        if (root->tokenptr->tokenInfo == "error-object?")
            return FORM_ERROR_OBJECT_QUESTION;
        if (root->tokenptr->tokenInfo == "write")
            return FORM_WRITE;
        if (root->tokenptr->tokenInfo == "display-string")
            return FORM_DISPLAY_STRING;
        if (root->tokenptr->tokenInfo == "newline")
            return FORM_NEWLINE;
        if (root->tokenptr->tokenInfo == "symbol->string")
            return FORM_SYMBOL_TO_STRING;
        if (root->tokenptr->tokenInfo == "number->string")
            return FORM_NUMBER_TO_STRING;
        if (root->tokenptr->tokenInfo == "eval")
            return FORM_EVAL;
        if (root->tokenptr->tokenInfo == "set!")
            return FORM_SET;
        // default
        return FORM_DEFAULT;
    }
    bool CompareTwoParsingTrees(shared_ptr<Node> sexp1, shared_ptr<Node> sexp2) const {
        if (!sexp1 || !sexp2)
            return sexp1 == sexp2;
        if (sexp1->IsAtom() != sexp2->IsAtom())
            return false;
        if (sexp1->IsAtom() && sexp2->IsAtom()) {
            if (!sexp1->tokenptr || !sexp2->tokenptr)
                return false;
            TokenType t1 = sexp1->tokenptr->type;
            TokenType t2 = sexp2->tokenptr->type;
            // tier1: T, NIL
            if (t1 == T && t1 == t2) {
                return true;
            }
            if (t1 == NIL && t1 == t2) {
                return true;
            }  
            // tier2: INT, FLOAT
            if ((t1 == INT || t1 == FLOAT) &&
                (t2 == INT || t2 == FLOAT)) {
                return stod(sexp1->tokenptr->tokenInfo) ==
                    stod(sexp2->tokenptr->tokenInfo);
            }
            // tier3: STRING==ERROR
            if ((t1 == STRING || t1 == ERROR) &&
                (t2 == STRING || t2 == ERROR)) {
                return sexp1->tokenptr->tokenInfo == sexp2->tokenptr->tokenInfo;
            }        
            return t1 == t2 &&
                sexp1->tokenptr->tokenInfo ==
                sexp2->tokenptr->tokenInfo;
        }

        // recursive compare pair/list
        return CompareTwoParsingTrees(sexp1->car, sexp2->car) &&
            CompareTwoParsingTrees(sexp1->cdr, sexp2->cdr);
    }

    shared_ptr<Node> EvalDefault(shared_ptr<Node> root, shared_ptr<Environment> env) {
        shared_ptr<Node> newRoot = EvalSexp(root->car, env);
        if (!newRoot->IsAtom()) throw AttemptToApplyNonFunction(newRoot);
        FormType ft = IdentifyForm(newRoot);
        throw AttemptToApplyNonFunction(newRoot);
    }

    shared_ptr<Node> EvalExit(shared_ptr<Node> root, shared_ptr<Environment> env, bool isTopLevel) {
        CheckLevel(FORM_EXIT, isTopLevel);
        // argument: 0
        CheckNumberOfArguments(root, "exit", 0);
        throw ExitOurScheme();
    }
    
    shared_ptr<Node> EvalQuote(shared_ptr<Node> root, shared_ptr<Environment> env) {
        // argument: 1
        CheckNumberOfArguments(root, "quote", 1);
        return root->cdr->car;
    }
    shared_ptr<Node> EvalCons(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "cons", 2);
        auto result = make_shared<Node>();
        result->car = EvalSexp(root->cdr->car, env);
        result->cdr = EvalSexp(root->cdr->cdr->car, env);
        if (!result->car) throw UnboundParameter(root->cdr->car);
        if (!result->cdr) throw UnboundParameter(root->cdr->cdr->car);
        return result;
    }

    shared_ptr<Node> EvalList(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (root->cdr && root->cdr->IsNil()) {
            shared_ptr<Node> newRoot = make_shared<Node>();
            Token tk = {NIL, "nil", -1, -1, -1, -1};
            newRoot->tokenptr = make_shared<Token>(tk);
            return newRoot;
        }
        shared_ptr<Node> newRoot = make_shared<Node>();
        newRoot->tokenptr = nullptr;
        shared_ptr<Node> newCur = newRoot;
        for (shared_ptr<Node> cur = root->cdr; 
            cur && !cur->IsNil() && newCur && !newCur->IsNil(); 
            cur = cur->cdr, newCur = newCur->cdr) {
            newCur->car = EvalSexp(cur->car, env);
            if (!newCur->car) throw UnboundParameter(cur->car);
            if (cur->cdr && !cur->cdr->IsNil()) newCur->cdr = make_shared<Node>();
            else {
                Token nilToken = {NIL, "nil", -1, -1, -1, -1};
                newCur->cdr = make_shared<Node>(nilToken);
            }
        }
        return newRoot;
        
    }

    shared_ptr<Node> EvalCar(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "car", 1);
        auto arg = EvalSexp(root->cdr->car, env);
        if (!arg) throw UnboundParameter(root->cdr->car);
        if (!arg->IsPair()) throw WithIncorrectArgumentType("car", arg);
        return arg->car;    
    }

    shared_ptr<Node> EvalCdr(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "cdr", 1);
        auto arg = EvalSexp(root->cdr->car, env);
        if (!arg) throw UnboundParameter(root->cdr->car);
        if (!arg->IsPair()) throw WithIncorrectArgumentType("cdr", arg);
        return arg->cdr;
    }

    shared_ptr<Node> EvalDefine(shared_ptr<Node> root, shared_ptr<Environment> env, bool isTopLevel) {
        CheckLevel(FORM_DEFINE, isTopLevel);
        shared_ptr<Node> cur = root->cdr;
        if (cur && cur->IsNil()) throw DefineFormat(root);
        if (cur->car->IsAtom()) {
            if ((cur->cdr && cur->cdr->IsNil()) || (cur->cdr->cdr && !cur->cdr->cdr->IsNil())) throw DefineFormat(root);
            if (!cur->car->IsAtom() ||(cur->car->IsAtom() && cur->car->tokenptr->type != SYMBOL)) throw DefineFormat(root);

            FormType ft = IdentifyForm(cur->car);
            if (ft == FORM_DEFAULT) {
                Token variable = {cur->car->tokenptr->type, cur->car->tokenptr->tokenInfo, -1, -1, -1, -1};
                shared_ptr<Node> newRoot;
                
                newRoot = EvalSexp(cur->cdr->car, env);
                if (!newRoot) throw NoReturnValue(cur->cdr->car);
                env->table[variable.tokenInfo] = newRoot;
                if (verbose)  cout << variable.tokenInfo << " defined\n";

                
                return EmptyNodeGenerator();
            }
            //throw DefineFormat(root);
        } else {
            string functionName;  // functionName<-----------------------------------
            vector<string> parametersVec;  // parameters<-----------------------------------
            bool first = true;
            if (!cur->cdr || cur->cdr->IsNil()) throw DefineFormat(root);
            for (auto cur2 = cur->car; cur2 && !cur2->IsNil(); cur2 = cur2->cdr) {
                if (!cur2->car->IsAtom() || cur2->car->tokenptr->type != SYMBOL) throw DefineFormat(root); 
                FormType ft = IdentifyForm(cur2->car);
                if (ft != FORM_DEFAULT && ft != FORM_CLOSURE) throw DefineFormat(root);  
                string parameter;
                parameter = cur2->car->tokenptr->tokenInfo;
                if (!first) parametersVec.push_back(parameter);
                else functionName = parameter;
                first = false;
            }
            shared_ptr<Node> functionNode = MakeProcedureNode(FORM_CLOSURE, functionName);
            functionNode->closureptr = make_shared<Closure>();
            functionNode->closureptr->functionName = functionName;
            functionNode->closureptr->parameters = parametersVec;
            functionNode->closureptr->body = cur->cdr;
            functionNode->closureptr->curEnvironment = baseEnvironment;

            env->table[functionName] = functionNode;
            
            if (verbose)  cout << functionName << " defined\n";

            return EmptyNodeGenerator();

        }
        throw DefineFormat(root);

    }
    shared_ptr<Node> EvalArithmetic(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string arith = temp->tokenptr->tokenInfo;
        CheckIfLessThanNArguments(root, arith, 2);
        double ftotal = 0;
        bool hasFloat = false;

        shared_ptr<Node> numNode = EvalSexp(root->cdr->car, env);
        if (!numNode) throw UnboundParameter(root->cdr->car);
        if (!numNode->IsAtom() || 
            ( numNode->IsAtom() && numNode->tokenptr->type != INT && numNode->tokenptr->type != FLOAT)) 
            throw WithIncorrectArgumentType(arith, numNode);
        ftotal += stod(numNode->tokenptr->tokenInfo);

        if (numNode->tokenptr && numNode->tokenptr->type == FLOAT) hasFloat = true; 

        for (shared_ptr<Node> cur = root->cdr->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
            shared_ptr<Node> numNode = EvalSexp(cur->car, env);
             if (!numNode) throw UnboundParameter(cur->car);
             if (!numNode->IsAtom() || 
                ( numNode->IsAtom() && numNode->tokenptr->type != INT && numNode->tokenptr->type != FLOAT)) 
                throw WithIncorrectArgumentType(arith, numNode);
            double num = stod(numNode->tokenptr->tokenInfo);
            if (numNode->tokenptr && numNode->tokenptr->type == FLOAT) hasFloat = true; 
            if (arith == "#<procedure +>") ftotal += num;
            if (arith == "#<procedure ->")  ftotal -= num;
            if (arith == "#<procedure *>") ftotal *= num;
            if (arith == "#<procedure />") {
                if (num == 0.0f) throw DivisionByZero();
                ftotal /= num;
                if (numNode->tokenptr && numNode->tokenptr->type == INT && !hasFloat)
                    ftotal = static_cast<double>((long long)ftotal);
                else if (numNode->tokenptr && numNode->tokenptr->type == FLOAT)
                    hasFloat = true;
            }

        }
        
        shared_ptr<Node> resultNode = make_shared<Node>();
       
        Token tk = {FLOAT, "", -1, -1, -1, -1};
        resultNode->tokenptr = make_shared<Token>(tk);
        if (!hasFloat) {
            resultNode->tokenptr->type = INT;
            int inum = static_cast<long long>(ftotal);
            stringstream ss;
            ss << inum;
            resultNode->tokenptr->tokenInfo = ss.str();
        }else {
            resultNode->tokenptr->type = FLOAT;
            stringstream ss;
            ss << fixed << setprecision(3) << ftotal;
            resultNode->tokenptr->tokenInfo = ss.str();
        }
        return resultNode;

    }

    shared_ptr<Node> EvalPredicate(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string predicate = temp->tokenptr->tokenInfo;
        CheckNumberOfArguments(root, predicate, 1);
        shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot) throw UnboundParameter(root->cdr->car);
        if (predicate == "#<procedure null?>") {
            if (newRoot->IsAtom() &&  newRoot->tokenptr && newRoot->tokenptr->type == NIL) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//1
        if (predicate == "#<procedure atom?>") {
            if (newRoot->IsAtom()) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//7
        if (predicate == "#<procedure pair?>") {
            if (!newRoot->IsAtom()) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//2
        if (predicate == "#<procedure number?>" || predicate == "#<procedure real?>") {
            if (newRoot->IsAtom() && newRoot->tokenptr && (newRoot->tokenptr->type == INT || newRoot->tokenptr->type == FLOAT)) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//3

        if (predicate == "#<procedure symbol?>") {
            if (newRoot->IsAtom() && newRoot->tokenptr && 
                newRoot->tokenptr->type == SYMBOL && !newRoot->formtypeptr) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//4
        if (predicate == "#<procedure string?>") {

            if (newRoot->IsAtom() && newRoot->tokenptr && (newRoot->tokenptr->type == STRING || newRoot->tokenptr->type == ERROR)) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//5
        if (predicate == "#<procedure boolean?>") {
            if (newRoot->IsAtom() && newRoot->tokenptr &&(newRoot->tokenptr->type == T || newRoot->tokenptr->type == NIL)) {
                return TrueNodeGenerator();
            }
            return NilNodeGenerator();
        }//6

        if (predicate == "#<procedure list?>") {
            if (newRoot->IsAtom() && newRoot->tokenptr && newRoot->tokenptr->type != NIL) return NilNodeGenerator();
            shared_ptr<Node> cur;
            for (cur = newRoot; cur && !cur->IsNil(); cur = cur->cdr);
            if (cur && cur->IsNil()) 
                return TrueNodeGenerator();
            return NilNodeGenerator();
        }//8
        if (predicate == "#<procedure integer?>") {
            if (newRoot->IsAtom() && newRoot->tokenptr && newRoot->tokenptr->type == INT) return TrueNodeGenerator();
            return NilNodeGenerator();
        }//9


        throw UnexpectedError();
    }

    shared_ptr<Node> EvalCompare(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string com = temp->tokenptr->tokenInfo;
        CheckIfLessThanNArguments(root, com, 2);
        
        shared_ptr<Node> preNum = nullptr;
        shared_ptr<Node> curNum = EvalSexp(root->cdr->car, env);
        if (!curNum) throw UnboundParameter(root->cdr->car);
        for (shared_ptr<Node> cur = root->cdr; cur&& !cur->IsNil(); cur = cur->cdr) {
            preNum = curNum;
            curNum = EvalSexp(cur->car, env);
            if (!curNum) throw UnboundParameter(cur->car);
            if ((curNum && !curNum->IsAtom()) || 
                (curNum->tokenptr && curNum->tokenptr->type != INT && curNum->tokenptr->type != FLOAT)) {
                    throw WithIncorrectArgumentType(com, curNum);
            }
        }
        preNum = nullptr;
        curNum = EvalSexp(root->cdr->car , env);
        if (!curNum) throw UnboundParameter(root->cdr->car);  
        for (shared_ptr<Node> cur = root->cdr->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
            preNum = curNum;
            curNum = EvalSexp(cur->car, env);
            if (!curNum) throw UnboundParameter(cur->car);
            double prenum = stod(preNum->tokenptr->tokenInfo);
            double curnum = stod(curNum->tokenptr->tokenInfo);       
            if (com == "#<procedure >>" && prenum <= curnum) return NilNodeGenerator();
            if (com == "#<procedure >=>" && prenum < curnum) return NilNodeGenerator();
            if (com == "#<procedure =>" && prenum != curnum) return NilNodeGenerator();
            if (com == "#<procedure <=>" && prenum > curnum) return NilNodeGenerator();
            if (com == "#<procedure <>" && prenum >= curnum) return NilNodeGenerator();
        }
        return TrueNodeGenerator();
    }
    shared_ptr<Node> EvalNot(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string noT = temp->tokenptr->tokenInfo;
        CheckNumberOfArguments(root, noT, 1);
        shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot) throw UnboundParameter(root->cdr->car);
        if (newRoot->IsAtom() && newRoot->tokenptr && newRoot->tokenptr->type == NIL) return TrueNodeGenerator();
        return NilNodeGenerator();
        throw UnexpectedError(); 
    }
    shared_ptr<Node> EvalAnd(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string und = temp->tokenptr->tokenInfo;
        shared_ptr<Node> newRoot;
        CheckIfLessThanNArguments(root, und, 2);
        for (shared_ptr<Node> cur = root->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
            newRoot = EvalSexp(cur->car, env);
            if (!newRoot) throw UnboundCondition(cur->car);
            if (newRoot && newRoot->IsAtom() && (newRoot->tokenptr && newRoot->tokenptr->type == NIL))
                return NilNodeGenerator();
        }
        return newRoot;    
    }

    shared_ptr<Node> EvalOr(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string oder = temp->tokenptr->tokenInfo;
        shared_ptr<Node> newRoot;
        CheckIfLessThanNArguments(root, oder, 2);
        for (shared_ptr<Node> cur = root->cdr; cur&& !cur->IsNil(); cur = cur->cdr) {
            newRoot = EvalSexp(cur->car, env);
            if(!newRoot) throw UnboundCondition(cur->car);
            if (!newRoot->IsAtom() || newRoot->tokenptr->type != NIL)
                return newRoot;
        }
        return NilNodeGenerator();
    }
    void StringTransformer(string& str) {
        stringstream ss;
        for (int i = 0; i < str.size(); i++) {
            if (str[i] == '\\' && 
                (str[i + 1] == 'n' || str[i + 1] == 't' ||
                str[i + 1] == '\"' || str[i + 1] == '\\')) {
                if (str[i + 1] == 'n') ss << "\n";
                if (str[i + 1] == 't') ss << "\t";
                if (str[i + 1] == '\"') ss << "\"";
                if (str[i + 1] == '\\') ss << "\\";
                i++;
                continue;
            }
            ss << str[i];
        }
        str = ss.str();
    }
    shared_ptr<Node> EvalString(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string str = temp->tokenptr->tokenInfo;
        shared_ptr<Node> newRoot;
        CheckIfLessThanNArguments(root, str, 2);

        if (str == "#<procedure string-append>") {

            
            shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
            if (!newRoot) throw UnboundParameter(root->cdr->car);
            if (!newRoot->IsAtom() || (newRoot->tokenptr && newRoot->tokenptr->type != STRING && newRoot->tokenptr->type != ERROR)) {
                throw WithIncorrectArgumentType(str, newRoot);
            }
            string returnString = newRoot->tokenptr->tokenInfo.substr(0, newRoot->tokenptr->tokenInfo.length() - 1);
            for (shared_ptr<Node> cur = root->cdr->cdr; cur&& !cur->IsNil(); cur = cur->cdr) {
                shared_ptr<Node> newRoot = EvalSexp(cur->car, env);
                if (!newRoot) throw UnboundParameter(cur->car);
                if (!newRoot->IsAtom() || (newRoot->tokenptr && newRoot->tokenptr->type != STRING && newRoot->tokenptr->type != ERROR)) {
                    throw WithIncorrectArgumentType(str, newRoot);
                }
                returnString += newRoot->tokenptr->tokenInfo.substr(1, newRoot->tokenptr->tokenInfo.length() - 2);
            }
            returnString += "\"";
            shared_ptr<Node> returnNode = make_shared<Node>();
           
            Token tk = {STRING, returnString, -1, -1, -1, -1};
            returnNode->tokenptr = make_shared<Token>(tk);
            return returnNode;
            
        } 
        else if (str == "#<procedure string>?>" || str == "#<procedure string=?>" || str == "#<procedure string<?>") {
            
            shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
            if (!newRoot) throw UnboundParameter(root->cdr->car);
            if (!newRoot->IsAtom() || (newRoot->tokenptr && newRoot->tokenptr->type != STRING && newRoot->tokenptr->type != ERROR)) {
                throw WithIncorrectArgumentType(str, newRoot);
            }
            string prestr;
            string curstr = newRoot->tokenptr->tokenInfo.substr(1, newRoot->tokenptr->tokenInfo.length() - 2);
            StringTransformer(curstr);
            bool shouldReturnNil = false;
            
            for (shared_ptr<Node> cur = root->cdr->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
                
                shared_ptr<Node> newRoot = EvalSexp(cur->car, env);
                if (!newRoot) throw UnboundParameter(cur->car);
                if (!newRoot->IsAtom() || ( newRoot->tokenptr && newRoot->tokenptr->type != STRING && newRoot->tokenptr->type != ERROR)) {
                    
                    throw WithIncorrectArgumentType(str, newRoot);
                }
                prestr = curstr;
                curstr = newRoot->tokenptr->tokenInfo.substr(1, newRoot->tokenptr->tokenInfo.length() - 2);
                
                
                StringTransformer(curstr);
                //cout << "[" << prestr << ", " << curstr<< "]\n";
                if (str == "#<procedure string>?>" && prestr <= curstr) shouldReturnNil = true;
                if (str == "#<procedure string=?>" && prestr != curstr) shouldReturnNil = true;
                if (str == "#<procedure string<?>" && prestr >= curstr) shouldReturnNil = true;     

            }
            if (shouldReturnNil) return NilNodeGenerator();
            return TrueNodeGenerator();
            
        }
        throw UnexpectedError();
    }

    shared_ptr<Node> EvalEquiv(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> eqNode = EvalSexp(root->car, env);
        if (!eqNode) throw NoReturnValue(root->car);
        string eq = eqNode->tokenptr->tokenInfo;
        CheckNumberOfArguments(root, eq, 2);
        shared_ptr<Node> sexp1 = EvalSexp(root->cdr->car, env);
        if (!sexp1) throw UnboundParameter(root->cdr->car);
        shared_ptr<Node> sexp2 = EvalSexp(root->cdr->cdr->car, env);
        if (!sexp2) throw UnboundParameter(root->cdr->cdr->car);

        if (eq == "#<procedure eqv?>") {
            if ((sexp1->IsAtom() && sexp2->IsAtom()) && 
                (sexp1->tokenptr->type != STRING && sexp1->tokenptr->type != ERROR && sexp2->tokenptr->type != STRING && sexp2->tokenptr->type != ERROR)) {
                
                // tier1: T, NIL
                if (sexp1->tokenptr->type == T && sexp1->tokenptr->type == sexp2->tokenptr->type) 
                    return TrueNodeGenerator();
                if (sexp1->tokenptr->type == NIL && sexp1->tokenptr->type == sexp2->tokenptr->type)
                    return TrueNodeGenerator();
                // tier2: SYMBOL
                if (sexp1->tokenptr->type == SYMBOL && sexp1->tokenptr->type == sexp2->tokenptr->type && 
                    sexp1->tokenptr->tokenInfo == sexp2->tokenptr->tokenInfo)
                    return TrueNodeGenerator();
                // tier3: INT FLOAT
                if ((sexp1->tokenptr->type == INT || sexp1->tokenptr->type == FLOAT) &&
                    (sexp2->tokenptr->type == INT || sexp2->tokenptr->type == FLOAT) &&
                    stod(sexp1->tokenptr->tokenInfo) == stod(sexp2->tokenptr->tokenInfo)) {
                    return TrueNodeGenerator();
                }

                
            } else if (sexp1 == sexp2) {
                return TrueNodeGenerator();
            } 
            return NilNodeGenerator();
        } else if (eq == "#<procedure equal?>") {
            if (CompareTwoParsingTrees(sexp1, sexp2)) return TrueNodeGenerator();
            return NilNodeGenerator();
        }

        throw UnexpectedError();

    }
    shared_ptr<Node> EvalIf(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        if (!temp) throw NoReturnValue(root->car);
        string iF = temp->tokenptr->tokenInfo;
       
        CheckIfLessThanNArguments(root, iF, 2);
        CheckIfMoreThanNArguments(root, iF, 3);
        shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot) throw UnboundTestCondition(root->cdr->car);
        shared_ptr<Node> conclusion = nullptr;
        shared_ptr<Node> conclusion2 = nullptr;



        if (!newRoot->tokenptr || newRoot->tokenptr->type != NIL) {
            conclusion = EvalSexp(root->cdr->cdr->car, env);
        } else {
            if (root->cdr->cdr->cdr && root->cdr->cdr->cdr->IsNil()) return nullptr;
            conclusion = EvalSexp(root->cdr->cdr->cdr->car, env);
        }
            
        if (!conclusion) return nullptr;
        return conclusion;
    }


    shared_ptr<Node> EvalElse(const shared_ptr<Node>& original_root, shared_ptr<Node> root, shared_ptr<Environment> env) {
        shared_ptr<Node> cur;
        for (cur = root; cur&& !cur->IsNil(); cur = cur->cdr); 
        if (!cur) throw CondFormat(original_root);

        shared_ptr<Node> newRoot;
        for (shared_ptr<Node> cur = root->cdr; cur&& !cur->IsNil(); cur = cur->cdr) {
            newRoot = EvalSexp(cur->car, env);
        }
        
        return newRoot;
    }

    shared_ptr<Node> EvalCond(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (root->cdr && root->cdr->IsNil()) throw CondFormat(root);

        for (shared_ptr<Node> cur = root->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
            if (cur->car->IsAtom()) throw CondFormat(root);
            if (cur->car->cdr && cur->car->cdr->IsNil()) throw CondFormat(root);
            shared_ptr<Node> c;
            for (c = cur->car; c && !c->IsNil(); c = c->cdr);
            if (!c) throw CondFormat(root);
        }
        shared_ptr<Node> newRoot = nullptr;
     
 
        for(shared_ptr<Node> cur = root->cdr; cur&& !cur->IsNil(); cur = cur->cdr) {
            if (cur->car->IsAtom()) throw CondFormat(root);
            // else condition
            if ( cur->cdr && cur->cdr->IsNil() && cur->car && cur->car->car &&
                cur->car->car->IsAtom() &&             
                cur->car->car->tokenptr->tokenInfo == "else" ) {
                if (cur->cdr  && !cur->cdr->IsNil()) throw CondFormat(root);
                newRoot = EvalElse(root, cur->car, env);
                if (newRoot) return newRoot;   
                return nullptr;
            } 
            shared_ptr<Node> condTest = EvalSexp(cur->car->car, env);
            if (!condTest) throw UnboundTestCondition(cur->car->car);
            if (!condTest->tokenptr || condTest->tokenptr->type != NIL) {
                shared_ptr<Node> bodyResult;
                for (auto body = cur->car->cdr; body && !body->IsNil(); body = body->cdr) {
                    bodyResult = EvalSexp(body->car, env);
                }
                return bodyResult;  
            }
        
            
        }
        if (!newRoot) 
            return nullptr;
        return newRoot;
    }


    shared_ptr<Node> EvalBegin(shared_ptr<Node> root, shared_ptr<Environment> env) {
        if (!root->car) throw UnexpectedError();
        shared_ptr<Node> temp = EvalSexp(root->car, env);
        string beg = temp->tokenptr->tokenInfo;
        shared_ptr<Node> newRoot;
        CheckIfLessThanNArguments(root, beg, 1);
        
        for (shared_ptr<Node> cur = root->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
            newRoot = EvalSexp(cur->car, env);
        }
        return newRoot;
    }


    shared_ptr<Node> EvalCleanEnvironment(shared_ptr<Node> root, shared_ptr<Environment> env, bool isTopLevel) {
        CheckLevel(FORM_CLEAN_ENVIRONMENT, isTopLevel);
        // argument: 0
        CheckNumberOfArguments(root, "clean-environment", 0);

        baseEnvironment->table.clear();
        if (verbose) cout << "environment cleaned\n";
        return EmptyNodeGenerator();
    }


    // engine to run lambda
    shared_ptr<Node> EvalClosure(shared_ptr<Node> root, shared_ptr<Environment> env) {
        
        // 1. eval closure node from root->car
        auto closureNode = EvalSexp(root->car, env);
        if (!closureNode) throw NoReturnValue(root->car);
        if (!closureNode->IsAtom()) throw UnexpectedError();
        // 2. count parameters number
        vector<string> parametersVec = closureNode->closureptr->parameters;
        int parametersNum = closureNode->closureptr->parameters.size();
        
        CheckNumberOfArguments(root, closureNode->closureptr->functionName, parametersNum);

        // 3. build new environment
        auto newEnv = make_shared<Environment>();
        newEnv->parent = closureNode->closureptr->curEnvironment;
        auto cur = root->cdr;
        // 4. bind parameters
        for (const string& parameter: parametersVec) {
            newEnv->table[parameter] = EvalSexp(cur->car, env);
            if (!newEnv->table[parameter]) throw UnboundParameter(cur->car);
            cur = cur->cdr;
        }
        // 5. execute body
        shared_ptr<Node> newRoot;
        for (auto cur = closureNode->closureptr->body; cur && !cur->IsNil(); cur = cur->cdr) {
            newRoot = EvalSexp(cur->car, newEnv);
        }
        
        return newRoot;
    }
    vector<string> CheckLambdaParameterAndReturnParameters(shared_ptr<Node>& original_root, shared_ptr<Node> root) {
        if (root->IsAtom() && !root->IsNil()) throw LambdaFormat(original_root);
        
        vector<string> vec;
        for (auto cur = root; cur && !cur->IsNil(); cur = cur->cdr) {
            auto parameter = cur->car;
            if (!parameter->IsAtom() || (parameter->tokenptr && parameter->tokenptr->type != SYMBOL)) throw LambdaFormat(original_root);
            // ------is Symbol but is command-------------
            string parameterString = parameter->tokenptr->tokenInfo;
            FormType ft = IdentifyForm(parameter);
            // ------------------------------------------
            if (ft != FORM_DEFAULT) throw LambdaFormat(original_root);
            vec.push_back(parameter->tokenptr->tokenInfo);
        }
        return vec;
    }
    shared_ptr<Node> EvalLambda(shared_ptr<Node> root, shared_ptr<Environment> env) {
        // (lambda ({symbol}) sexp{sexp} )
        if (!root->cdr || root->cdr->IsNil()) throw LambdaFormat(root);
        if (!root->cdr->cdr || root->cdr->cdr->IsNil()) throw LambdaFormat(root);
        vector<string> parameters = CheckLambdaParameterAndReturnParameters(root, root->cdr->car);
        shared_ptr<Node> returnFunctionNode; // must be an ATOM and closureptr !=nullptr
                                            // [check]: parameter, body, curEnvironment
        returnFunctionNode = MakeProcedureNode(FORM_CLOSURE, "lambda");
        returnFunctionNode->closureptr = make_shared<Closure>();
        returnFunctionNode->closureptr->functionName = "lambda";
        returnFunctionNode->closureptr->parameters = parameters;
        returnFunctionNode->closureptr->body = root->cdr->cdr;
        returnFunctionNode->closureptr->curEnvironment = baseEnvironment; // ***important***: [env] or [baseEnvironment]
        return returnFunctionNode;
    }
    shared_ptr<Node> EvalLet(shared_ptr<Node> root, shared_ptr<Environment> env) {
        // (let ({sexp})  sexp{sexp} )
        // check let format
        if (!root->cdr || root->cdr->IsNil()) throw LetFormat(root);
        if (!root->cdr->cdr || root->cdr->cdr->IsNil()) throw LetFormat(root);
        shared_ptr<Node> bindParameters = root->cdr->car;
        if (!bindParameters->IsNil() && !bindParameters->IsPair()) throw LetFormat(root);
        for (shared_ptr<Node> cur = bindParameters; cur && !cur->IsNil(); cur = cur->cdr) {
            shared_ptr<Node> base = cur->car;
            if (!base->IsPair()) throw LetFormat(root);
            if (!base->car || !base->car->IsAtom() || base->car->tokenptr->type != SYMBOL) 
                throw LetFormat(root);
            FormType ft = IdentifyForm(base->car);
            if (ft != FORM_DEFAULT) throw LetFormat(root);
            if (!base->cdr || base->cdr->IsNil()) throw LetFormat(root);
            if (!base->cdr->cdr || !base->cdr->cdr->IsNil()) throw LetFormat(root);
        }
        shared_ptr<Environment> newEnvironment = make_shared<Environment>();
        newEnvironment->parent = env;
        for (shared_ptr<Node> cur = bindParameters; cur && !cur->IsNil(); cur = cur->cdr) {
            shared_ptr<Node> base = cur->car;
            shared_ptr<Node> val;
            val = EvalSexp(base->cdr->car, env);
            if (!val) throw NoReturnValue(base->cdr->car);
            newEnvironment->table[base->car->tokenptr->tokenInfo] = val;
        }
        shared_ptr<Node> result;
        for (auto cur = root->cdr->cdr; cur && !cur->IsNil(); cur = cur->cdr) {
            result = EvalSexp(cur->car, newEnvironment);
        }
        return result;
    }
    shared_ptr<Node> EvalVerbose(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "verbose", 1);
        shared_ptr<Node> newRoot;
        newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot) throw UnboundParameter(root->cdr->car);
        if (newRoot->IsAtom() && newRoot->tokenptr->type == NIL) {
            verbose = false;
            return NilNodeGenerator();
        }
        verbose = true;
        return TrueNodeGenerator();
    }

    shared_ptr<Node> EvalVerboseQuestion(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "verbose?", 0);
        if (verbose) return TrueNodeGenerator();
        return NilNodeGenerator();
    }
    shared_ptr<Node> EvalSet(shared_ptr<Node> root, shared_ptr<Environment> env) {
        shared_ptr<Node> cur = root->cdr;
        if (cur && cur->IsNil()) throw SetFormat(root);
        if (cur->car->IsAtom()) {
            if ((cur->cdr && cur->cdr->IsNil()) || (cur->cdr->cdr && !cur->cdr->cdr->IsNil()))
                throw SetFormat(root);
            if (!cur->car->IsAtom() || cur->car->tokenptr->type != SYMBOL)
                throw SetFormat(root);

            FormType ft = IdentifyForm(cur->car);
            if (ft == FORM_DEFAULT) {
                shared_ptr<Node> newRoot = EvalSexp(cur->cdr->car, env);
                if (!newRoot) throw NoReturnValue(cur->cdr->car);
                env->SetVariable(cur->car->tokenptr->tokenInfo, newRoot, env);

                return newRoot;
            }
        }
        throw SetFormat(root);
    }
    shared_ptr<Node> EvalRead(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "read", 0);
        shared_ptr<Node> newRoot;
        shared_ptr<Node> errorobj;
        ps.ResetForRead();
        try {
            newRoot = ps.GetAnSExpression();
        } catch(NoClosingQuote& e) {
            string errMsg = e.what();
            string str = '\"' + errMsg + '\"';
            Token tk = {ERROR,  str, -1, -1, -1, -1};
            errorobj = make_shared<Node>(tk);
            ps.SkipRestOfLine();
        } catch (ExpectedAtomOrLeftParen& e) {
            string errMsg = e.what();
            string str = '\"' + errMsg + '\"';
            Token tk = {ERROR,  str, -1, -1, -1, -1};
            errorobj = make_shared<Node>(tk);
            ps.SkipRestOfLine();
        } catch (ExpectedRightParen& e) {
            string errMsg = e.what();
            string str = '\"' + errMsg + '\"';
            Token tk = {ERROR,  str, -1, -1, -1, -1};
            errorobj = make_shared<Node>(tk);
            ps.SkipRestOfLine();
        }  catch (NoMoreInput& e) {
            string errMsg = e.what();
            string str = '\"' + errMsg + '\"';
            Token tk = {ERROR,  str, -1, -1, -1, -1};
            errorobj = make_shared<Node>(tk);
           
        }
        
        if (newRoot) return newRoot;
        return errorobj;
        
    }

    shared_ptr<Node> EvalCreateErrorObject(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "create-error-object", 1);
        shared_ptr<Node> node = EvalSexp(root->cdr->car, env);
        if (!node)throw UnboundParameter(root->cdr->car);
        if (!node->IsAtom() || node->tokenptr->type != STRING) throw WithIncorrectArgumentType("create-error-object", node);
        shared_ptr<Node> newNode = make_shared<Node>();
        Token tk = {
            ERROR, node->tokenptr->tokenInfo,  -1, -1, -1, -1
        };
        newNode->tokenptr = make_shared<Token>(tk);
        return newNode;
    }
    shared_ptr<Node> EvalErrorObjectQuestion(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "error-object?", 1);
        shared_ptr<Node> node = EvalSexp(root->cdr->car, env);
        if (!node)throw UnboundParameter(root->cdr->car);
        if (!node->IsAtom() ||  node->tokenptr->type != ERROR) return NilNodeGenerator();
        return TrueNodeGenerator();
    }
    shared_ptr<Node> EvalWrite(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "write", 1);
        shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot)throw UnboundParameter(root->cdr->car);
        pt.PrettyPrint(newRoot, false);
        return newRoot; 
    }
    shared_ptr<Node> EvalDisplayString(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "display-string", 1);
        shared_ptr<Node> newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot)throw UnboundParameter(root->cdr->car);
        if (!newRoot->IsAtom() || (newRoot->tokenptr->type != STRING && newRoot->tokenptr->type != ERROR)) 
            throw WithIncorrectArgumentType("display-string", root->cdr->car);
        shared_ptr<Node> stringNode = make_shared<Node>();
        Token tk = {STRING, newRoot->tokenptr->tokenInfo.substr(1, newRoot->tokenptr->tokenInfo.size() - 2), -1, -1, -1, -1};
        stringNode->tokenptr = make_shared<Token>(tk);
        pt.PrettyPrint(stringNode, false);
        return newRoot;
    }

    shared_ptr<Node> EvalNewline(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "newline", 0);
        cout << "\n";
        return NilNodeGenerator();
    }
    shared_ptr<Node> EvalSymbolToString(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "symbol->string", 1);
        auto newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot) throw UnboundParameter(root->cdr->car);
        if (!newRoot->IsAtom() || newRoot->tokenptr->type != SYMBOL) throw WithIncorrectArgumentType("symbol->string", newRoot);
        auto stringNode = make_shared<Node>();
        string str = '\"' + newRoot->tokenptr->tokenInfo + '\"';
        Token tk = {STRING, str, -1, -1, -1, -1};
        stringNode->tokenptr = make_shared<Token>(tk);
        return stringNode;

    }
    shared_ptr<Node> EvalNumberToString(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root, "number->string", 1);
        auto newRoot = EvalSexp(root->cdr->car, env);
        if (!newRoot) throw UnboundParameter(root->cdr->car);
        if (!newRoot->IsAtom() ||
            (newRoot->tokenptr->type != INT && newRoot->tokenptr->type != FLOAT))
            throw WithIncorrectArgumentType("number->string", newRoot);
        double val = stod(newRoot->tokenptr->tokenInfo);  
        stringstream ss;
        if (newRoot->tokenptr->type == INT)
            ss << (long long)val; 
        else
            ss << fixed << setprecision(3) << val;
        string str = "\"" + ss.str() + "\"";
        Token tk = {STRING, str, -1, -1, -1, -1};
        auto stringNode = make_shared<Node>();
        stringNode->tokenptr = make_shared<Token>(tk);
        return stringNode;
    }
    shared_ptr<Node> EvalEval(shared_ptr<Node> root, shared_ptr<Environment> env) {
        CheckNumberOfArguments(root,"eval", 1);
        auto newRoot = EvalSexp(root->cdr->car, env, false);
        if (!newRoot) return nullptr;
        return EvalSexp(newRoot, baseEnvironment, true);
    }
    shared_ptr<Node> EvalSexp(shared_ptr<Node> root = nullptr, shared_ptr<Environment> env = nullptr, bool isTopLevel = false) {
        if (env == nullptr)
            env = baseEnvironment;
        if (!root) throw UnexpectedError();
        // 1. Leaf node
        if (root->IsAtom()) {
            if (root->tokenptr->type != SYMBOL) return root; 
            FormType ft = IdentifyForm(root);
            if (ft != FORM_DEFAULT) return MakeProcedureNode(ft, root->tokenptr->tokenInfo);

            auto val = env->FindVariable(root->tokenptr->tokenInfo, env);
            if (!val) throw UnboundSymbol(root->tokenptr->tokenInfo);
            return val;
        }
        // 2. Pair Expression
        CheckNonList(root);
        shared_ptr<Node> op;
        op = EvalSexp(root->car, env);
        if (!op)  throw NoReturnValue(root->car);
        FormType ft = op->formtypeptr ? *op->formtypeptr : FORM_DEFAULT;
        switch (ft) {
            case FORM_DEFINE:               return EvalDefine(root, env, isTopLevel);
            case FORM_CLEAN_ENVIRONMENT:    return EvalCleanEnvironment(root, env, isTopLevel);
            case FORM_EXIT:                 return EvalExit(root, env, isTopLevel);
            case FORM_DEFAULT:              return EvalDefault(root, env);
            case FORM_QUOTE:                return EvalQuote(root, env);
            case FORM_IF:                   return EvalIf( root, env);
            case FORM_COND:                 return EvalCond( root, env);
            case FORM_AND:                  return EvalAnd(root, env);
            case FORM_OR:                   return EvalOr(root, env);
            case FORM_BEGIN:                return EvalBegin(root, env);
            case FORM_CONS:                 return EvalCons(root, env);
            case FORM_LIST:                 return EvalList(root, env);
            case FORM_CAR:                  return EvalCar(root, env);
            case FORM_CDR:                  return EvalCdr(root, env);
            case FORM_ARITHMETIC:           return EvalArithmetic(root, env);
            case FORM_PREDICATE:            return EvalPredicate(root, env);
            case FORM_COMPARE:              return EvalCompare(root, env);
            case FORM_STRING:               return EvalString(root, env);
            case FORM_EQUIV:                return EvalEquiv(root, env);
            case FORM_NOT:                  return EvalNot(root, env);
            // proj3
            case FORM_CLOSURE:              return EvalClosure(root, env);
            case FORM_LAMBDA:               return EvalLambda(root, env);
            case FORM_LET:                  return EvalLet(root, env);
            case FORM_VERBOSE:              return EvalVerbose(root, env);
            case FORM_VERBOSE_QUESTION:     return EvalVerboseQuestion(root, env);
            // proj4
            case FORM_SET:                  return EvalSet(root, env);
            case FORM_READ:                 return EvalRead(root, env);
            case FORM_CREATE_ERROR_OBJECT:  return EvalCreateErrorObject(root, env);
            case FORM_ERROR_OBJECT_QUESTION:return EvalErrorObjectQuestion(root, env);
            case FORM_WRITE:                return EvalWrite(root, env);
            case FORM_DISPLAY_STRING:       return EvalDisplayString(root, env);
            case FORM_NEWLINE:              return EvalNewline(root, env);
            case FORM_SYMBOL_TO_STRING:     return EvalSymbolToString(root, env);
            case FORM_NUMBER_TO_STRING:     return EvalNumberToString(root, env);
            case FORM_EVAL:                 return EvalEval(root, env);
            default:                    throw UnexpectedError();
        }
    }
public:
    Evaluator()  
    {baseEnvironment = make_shared<Environment>();}
    void Set(shared_ptr<Node> _root) {
        root = _root;
    }
    void Eval(shared_ptr<Node> _root) {
        Set(_root);
        try {
            shared_ptr<Node> outputStructure = EvalSexp(root, baseEnvironment, true);
            if (outputStructure) pt.PrettyPrint(outputStructure);
            else throw NoReturnValue(root);
        } catch(NonList& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        } catch(IncorrectNumberOfArguments& e) {
            cout << e.what()<< "\n";
        } catch(WithIncorrectArgumentType& e) {
            cout << e.what(); 
            pt.PrettyPrint(e.sexpStart);
        } catch(AttemptToApplyNonFunction& e) {
            cout << e.what();
            pt.PrettyPrint(e.startNode);
        } catch(NoReturnValue& e) {
            cout << e.what(); 
            pt.PrettyPrint(e.sexpStart);
        } catch(UnboundSymbol& e) {
            cout << e.what()<< "\n";
        } catch(DivisionByZero& e) {
            cout << e.what()<< "\n";
        } catch(DefineFormat& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        } catch(CondFormat& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);    
        } catch(LevelOfExit& e) {
            cout << e.what()<< "\n";
        } catch(LevelOfDefine& e) {
            cout << e.what()<< "\n";
        } catch(LevelOfCleanEnvironment& e) {
            cout << e.what() << "\n";
        } catch (LetFormat& e) {
            cout << e.what();
            pt.PrettyPrint(e.root); 
        } catch (LambdaFormat& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        } catch (UnboundParameter& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        } catch (UnboundCondition& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        } catch (UnboundTestCondition& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        } catch (SetFormat& e) {
            cout << e.what();
            pt.PrettyPrint(e.root);
        }
    }
};
int main() {
    shared_ptr<Node> root;
    Parser ps;
    Printer pt;
    Evaluator ev;
    string buffer;
    getline(cin, buffer); 
    cout<< "Welcome to OurScheme!\n";
    try {
        cout << "\n> ";
        while (true) {
            ps.ReadAnSExpression();
            root = ps.GetRoot();
            if (root) ev.Eval(root);
            cout << "\n> ";
        } 
    }  catch(NoMoreInput& e) {
        cout << e.what();
    }  catch (ExitOurScheme& e) {
    } 
    cout<< "\nThanks for using OurScheme!";
    return 0;
}