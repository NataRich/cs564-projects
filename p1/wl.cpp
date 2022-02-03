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
    } while (!context.Destroyed());

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// 
// Below is the impementation of Parser::ToLower()
// 
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// 
// Below is the impementation of Command class
// 
///////////////////////////////////////////////////////////////////////////////

wl::Command::Command() : op(wl::Op::EMPTY), arg_2(0) { }

void wl::Command::Parse(const std::string* command, std::vector<std::string>* vec) const
{
    // Checks if the given command is an empty command
    if (command->empty()) return;

    std::regex re_new("^(new)\\s*$", std::regex_constants::icase);
    std::regex re_end("^(end)\\s*$", std::regex_constants::icase);
    std::regex re_load(
        "^(load)\\s+(\"(.+\\.[a-zA-Z]+)\"|(\\S+\\.[a-zA-Z]+))\\s*$",
        std::regex_constants::icase);
    std::regex re_locate(
        "^(locate)\\s+([0-9a-zA-Z']+)+\\s+([1-9][0-9]*)\\s*$",
        std::regex_constants::icase
    );

    const char* c_command = command->c_str();
    std::cmatch match;

    // Matches "new" command
    std::regex_match(c_command, match, re_new);
    if (match.size() != 0)
    {
        vec->emplace_back(this->ToLower(match.str(1)));
        return;
    }

    // Matches "end" command
    std::regex_match(c_command, match, re_end);
    if (match.size() != 0)
    {
        vec->emplace_back(this->ToLower(match.str(1)));
        return;
    }

    // Matches "load <filepath>" command
    std::regex_match(c_command, match, re_load);
    if (match.size() != 0)
    {
        vec->emplace_back(this->ToLower(match.str(1)));
        {
            match.str(3).empty() ?
                vec->emplace_back(match.str(4))
                :
                vec->emplace_back(match.str(3));
        }
        return;
    }

    // Matches "locate <word> <n>" command
    std::regex_match(c_command, match, re_locate);
    if (match.size() != 0)
    {
        vec->emplace_back(this->ToLower(match.str(1)));
        vec->emplace_back(this->ToLower(match.str(2)));
        vec->emplace_back(match.str(3));
        return;
    }

    // Indicates an invalid command if no match
    vec->emplace_back("INVALID");
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
    else if (ops_len == 1 && ops.at(0).compare("new") == 0)
    {
        this->op = wl::Op::NEW;
    }
    else if (ops_len == 1 && ops.at(0).compare("end") == 0)
    {
        this->op = wl::Op::END;
    }
    else if (ops_len == 2)
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
    else if (ops_len == 3)
    {
        uint16_t occurrence;
        std::stringstream ss;

        ss << ops.at(2);
        ss >> occurrence;

        this->op = wl::Op::LOCATE;
        this->arg_1 = ops.at(1);
        this->arg_2 = occurrence;
    }
    else
    {
        this->op = wl::Op::INVALID;
    }
}

///////////////////////////////////////////////////////////////////////////////
// 
// Below is the impementation of Dictionary::Node class
// 
///////////////////////////////////////////////////////////////////////////////

wl::Dictionary::Node::Node(std::string prefix = "") : prefix(prefix) { }

wl::Dictionary::Node::~Node()
{
    for (auto child : this->children)
    {
        delete child;
    }

    this->children.clear();
}

int wl::Dictionary::Node::Diff(std::string str1, std::string str2) const
{
    size_t len_1 = str1.size(), len_2 = str2.size();
    size_t max_length = std::min(len_1, len_2);
    size_t i_diff = -1;
    for (size_t j = 1; j < max_length; j++)
    {
        if (str1.at(j) != str2.at(j))
        {
            i_diff = j;
            break;
        }
    }

    return i_diff;
}

wl::Dictionary::Node* wl::Dictionary::Node::Next(char next_ch) const
{
    size_t length = this->children.size();
    for (size_t i = 0; i < length; i++)
    {
        Node* curr = this->children.at(i);
        if (curr->prefix.front() == next_ch)
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
        std::string sub = word->substr(i);
        next = curr->Next(sub.front());
        if (next == nullptr)
        {
            return 0;
        }
        else
        {
            size_t pre_size = next->prefix.size();
            size_t sub_size = sub.size();
            if (pre_size == sub_size && next->prefix.compare(sub) == 0)
            {
                i += sub_size;
            }
            else if (pre_size < sub_size && next->prefix.compare(sub.substr(0, pre_size)) == 0)
            {
                i += pre_size - 1;
            }
            else
            {
                return 0;
            }
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
        std::string sub = word->substr(i);
        next = curr->Next(sub.front());
        if (next == nullptr)
        {
            next = new Node(sub);
            next->counts.emplace_back(count);
            curr->children.emplace_back(next);

            break;
        }
        else
        {
            int i_diff = this->Diff(next->prefix, sub);
            size_t sub_size = sub.size();
            size_t pre_size = next->prefix.size();
            if (i_diff == -1)
            {
                if (sub_size > pre_size)
                {
                    i += pre_size - 1;
                }
                else if (sub_size < pre_size)
                {
                    Node* child = new Node(next->prefix.substr(sub_size));
                    child->children.swap(next->children);
                    child->counts.swap(next->counts);

                    next->children.emplace_back(child);
                    next->counts.emplace_back(count);
                    next->prefix = next->prefix.substr(0, sub_size);

                    break;
                }
                else
                {
                    next->counts.emplace_back(count);

                    break;
                }
            }
            else
            {
                Node* org_child = new Node(next->prefix.substr(i_diff));
                org_child->children.swap(next->children);
                org_child->counts.swap(next->counts);

                Node* new_child = new Node(sub.substr(i_diff));
                new_child->counts.emplace_back(count);

                next->children.emplace_back(org_child);
                next->children.emplace_back(new_child);
                next->prefix = next->prefix.substr(0, i_diff);

                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// 
// Below is the impementation of Dictionary class
// 
///////////////////////////////////////////////////////////////////////////////

wl::Dictionary::Dictionary() : word_list(new Node()), is_loadable(true) { }

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

    this->word_list = new Node();
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

///////////////////////////////////////////////////////////////////////////////
// 
// Below is the impementation of Context class
// 
///////////////////////////////////////////////////////////////////////////////

wl::Context::Context()
    : dictionary(new Dictionary()), result(-2), destroyed(false), prev_ops{ wl::Op::EMPTY } { }

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

bool wl::Context::Destroyed() const
{
    return this->destroyed;
}

void wl::Context::Execute(const Command* command)
{
    switch (command->GetOperation())
    {
    case wl::Op::END:
        this->destroyed = true;
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