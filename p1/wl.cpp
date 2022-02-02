///////////////////////////////////////////////////////////////////////////////
// 
// Project Name:        Word Locator
// Student:             Zihan Zhao
// Student ID:          908 266 6315
// UW Email Address:    zzhao383@wisc.edu
// 
///////////////////////////////////////////////////////////////////////////////
// 
// This File: wl.cpp
// Main File: wl.cpp
// 
// Purpose of this file: This file implements the classes, functions, etc. that
// are declared in wl.h. In addition, the main function is written here.
//
///////////////////////////////////////////////////////////////////////////////

#include "wl.h"

int main()
{
    wl::Context context;
    wl::Command command;

    do
    {
        command.Receive();
        context.Execute(&command);
    } while (!context.Destoryed());

    return 0;
}

std::string wl::Parser::ToLower(const std::string& str) const
{
    std::string copy;
    size_t length = str.size();
    for (size_t i = 0; i < length; i++)
    {
        copy += std::tolower(str.at(i));
    }

    return copy;
}

wl::Command::Command() : op(wl::Op::EMPTY), arg_2(0) { }

bool wl::Command::IsPosNumber(const std::string* str) const
{
    size_t length = str->size();

    for (size_t i = 0; i < length; i++)
    {
        if (!std::isdigit(str[i].c_str()[0]))
        {
            return false;
        }
    }

    return std::stoi(*str) > 0;
}

bool wl::Command::IsValid(const std::string* str) const
{
    size_t length = str->size();
    for (size_t i = 0; i < length; i++)
    {
        if (!isalnum((*str)[i]))
        {
            return false;
        }
    }

    return true;
}

void wl::Command::Parse(const std::string* command, std::vector<std::string>* vec) const
{
    std::istringstream iss(*command);
    std::string p;

    while (iss >> p) vec->emplace_back(p);
}

wl::Op wl::Command::GetOperation() const
{
    return this->op;
}

const std::string& wl::Command::GetFirstArg() const
{
    return this->arg_1;
}

uint16_t wl::Command::GetSecondArg() const
{
    return this->arg_2;
}

void wl::Command::Receive()
{
    std::string command;
    std::cout << ">";
    std::getline(std::cin, command);

    std::vector<std::string> ops;
    this->Parse(&command, &ops);
    size_t ops_len = ops.size();

    if (ops_len == 0)
    {
        this->op = wl::Op::EMPTY;
    }
    else if (ops_len == 1 && this->ToLower(ops.at(0)).compare("new") == 0)
    {
        this->op = wl::Op::NEW;
    }
    else if (ops_len == 1 && this->ToLower(ops.at(0)).compare("end") == 0)
    {
        this->op = wl::Op::END;
    }
    else if (ops_len == 2 && this->ToLower(ops.at(0)).compare("load") == 0)
    {
        std::ifstream f(ops.at(1));
        if (f.is_open())
        {
            this->op = wl::Op::LOAD;
            this->arg_1 = ops.at(1);

            f.close();
        }
        else
        {
            this->op = wl::Op::INVALID;
        }
    }
    else if (ops_len == 3 && this->ToLower(ops.at(0)).compare("locate") == 0)
    {
        std::string f_arg = this->ToLower(ops.at(1)), s_arg = ops.at(2);
        if (this->IsValid(&f_arg) && this->IsPosNumber(&s_arg))
        {
            uint16_t occurrence;
            std::stringstream ss;

            ss << s_arg;
            ss >> occurrence;

            this->op = wl::Op::LOCATE;
            this->arg_1 = f_arg;
            this->arg_2 = occurrence;
        }
        else
        {
            this->op = wl::Op::INVALID;
        }
    }
    else
    {
        this->op = wl::Op::INVALID;
    }
}

wl::Dictionary::Node::Node(char ch) : ch(ch) { }

wl::Dictionary::Node::~Node()
{
    for (auto child : this->children)
    {
        delete child;
    }

    this->children.clear();
}

wl::Dictionary::Node* wl::Dictionary::Node::Next(char next_ch) const
{
    size_t length = this->children.size();
    for (size_t i = 0; i < length; i++)
    {
        Node* curr = this->children.at(i);
        if (curr->ch == next_ch)
        {
            return curr;
        }
    }

    return nullptr;
}

uint16_t wl::Dictionary::Node::Search(const std::string* word, uint16_t occurrence) const
{
    const Node* curr = this, * next = nullptr;
    size_t length = word->size();
    for (size_t i = 0; i < length; i++, curr = next)
    {
        const char next_ch = word->at(i);
        next = curr->Next(next_ch);
        if (next == nullptr)
        {
            return 0;
        }
    }

    if (occurrence <= curr->counts.size())
    {
        return curr->counts[occurrence - 1];
    }

    return 0;
}

void wl::Dictionary::Node::Insert(const std::string* word, uint16_t count)
{
    Node* curr = this, * next = nullptr;
    size_t length = word->size();
    for (size_t i = 0; i < length; i++, curr = next)
    {
        const char next_ch = word->at(i);
        next = curr->Next(next_ch);
        if (next == nullptr)
        {
            next = new Node(next_ch);
            curr->children.emplace_back(next);
        }
    }

    curr->counts.emplace_back(count);
}

wl::Dictionary::Dictionary() : word_list(new Node(0)), is_loadable(true) { }

wl::Dictionary::~Dictionary()
{
    if (this->word_list != nullptr)
    {
        delete this->word_list;
        this->word_list = nullptr;
    }
}

void wl::Dictionary::Parse(const std::string* line, std::vector<std::string>* vec) const
{
    std::string word;
    size_t s_index = 0, e_index = 0, length = line->size();

    while (s_index < length && e_index < length)
    {
        char ch = line->at(e_index);
        if (isalnum(ch) || ch == '\'')
        {
            e_index++;
        }
        else
        {
            if (s_index != e_index)
            {
                word = line->substr(s_index, e_index - s_index);
                vec->emplace_back(this->ToLower(word));
            }

            bool found = false;
            for (size_t i = e_index + 1; i < length; i++)
            {
                ch = line->at(i);
                if (isalnum(ch) || ch == '\'')
                {
                    s_index = i;
                    e_index = i;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                s_index = e_index;
                break;
            }
        }
    }

    if (s_index != e_index)
    {
        word = line->substr(s_index, e_index - s_index);
        vec->emplace_back(this->ToLower(word));
    }
}

bool wl::Dictionary::IsLodable() const
{
    return this->is_loadable;
}

void wl::Dictionary::SetLodable(bool is_loadable)
{
    this->is_loadable = is_loadable;
}

void wl::Dictionary::New()
{
    if (this->word_list != nullptr)
    {
        delete this->word_list;
    }

    this->word_list = new Node(0);
    this->is_loadable = true;
}

void wl::Dictionary::Load(const std::string path)
{
    if (!this->is_loadable) return;

    std::ifstream f(path);
    std::string line;
    std::vector<std::string> words;
    words.reserve(20);  // an estimated max. number of words in a line
    uint16_t total_count = 0;
    if (f.is_open())
    {
        while (std::getline(f, line))
        {
            this->Parse(&line, &words);
            size_t length = words.size();
            for (size_t i = 0; i < length; i++)
            {
                this->word_list->Insert(&words[i], ++total_count);
            }

            words.clear();
        }

        f.close();

        this->is_loadable = false;
    }
}

uint16_t wl::Dictionary::Locate(const std::string word, uint16_t occurrence) const
{
    return this->word_list->Search(&word, occurrence);
}

wl::Context::Context() 
    : dictionary(new Dictionary()), result(-2), destoryed(false), prev_ops{ wl::Op::EMPTY } { }

wl::Context::~Context()
{
    if (this->dictionary != nullptr)
    {
        delete this->dictionary;
    }
}

void wl::Context::PrintResult()
{
    if (this->result == -1)
    {
        std::cout << "ERROR: Invalid command" << std::endl;
    }
    else if (this->result == 0)
    {
        std::cout << "No matching entry" << std::endl;
    }
    else if (this->result >= 1)
    {
        std::cout << this->result << std::endl;
    }

    this->result = -2;  // reset
}

bool wl::Context::Destoryed() const
{
    return this->destoryed;
}

void wl::Context::Execute(const Command* command)
{
    switch (command->GetOperation())
    {
    case wl::Op::END:
        this->destoryed = true;
        this->prev_ops[0] = this->prev_ops[1];
        this->prev_ops[1] = wl::Op::END;
        break;

    case wl::Op::NEW:
        this->dictionary->New();
        this->prev_ops[0] = this->prev_ops[1];
        this->prev_ops[1] = wl::Op::NEW;
        break;

    case wl::Op::LOAD:
        if (this->prev_ops[1] == wl::Op::LOAD && this->prev_ops[0] != wl::Op::LOAD)
        {
            this->dictionary->New();
        }

        if (this->dictionary->IsLodable())
        {
            this->dictionary->Load(command->GetFirstArg());
            this->prev_ops[0] = this->prev_ops[1];
            this->prev_ops[1] = wl::Op::LOAD;
        }
        else
        {
            this->result = -1;
        }
        break;
    

    case wl::Op::LOCATE:
        this->result = this->dictionary->Locate(command->GetFirstArg(), command->GetSecondArg());
        this->prev_ops[0] = this->prev_ops[1];
        this->prev_ops[1] = wl::Op::LOCATE;
        break;

    case wl::Op::INVALID:
        this->result = -1;
        break;

    case wl::Op::EMPTY:
    default:
        break;
    }

    this->PrintResult();
}