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
        context.Execute(command);
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

// This function identifies commands through regular expressions.
// 
// Following are some examples of allowed formats:
// > new
// >    new
// > end
// >       end
// > load sixpence
// >    load    sixpence
// >     load sixpence.txt
// > load   sixpence.txt.asfas.asdf.
// >  load   sixpence.txt.asfas.asdf.
// > load "path with whitespaces\to\file"
// > load "path with whitespaces\to\file.txt"
// > load "path with more whitespaces   \to  \file.txt        "
// > locate song  1
// >   locate  song  999
// 
// Following are disallowed:
// > new somestring
// > end somestring
// > load
// > load sixpence somestring
// > load no quotes sorrounded\file.txt
// > locate song 0
// > locate song -1
// > locate "song" 1
void wl::Command::Parse(const std::string& command, std::vector<std::string>& vec) const
{
    // Checks if the given command is an empty command
    if (command.empty()) return;

    std::regex re_new("^\\s*(new)\\s*$", std::regex_constants::icase);
    std::regex re_end("^\\s*(end)\\s*$", std::regex_constants::icase);
    std::regex re_load(
        "^\\s*(load)\\s+(\"(.*)\"|(\\S+))\\s*$",
        std::regex_constants::icase);
    std::regex re_locate(
        "^\\s*(locate)\\s+([0-9a-zA-Z']+)+\\s+([1-9][0-9]*)\\s*$",
        std::regex_constants::icase
    );

    const char* c_command = command.c_str();
    std::cmatch match;

    // Matches "new" command
    std::regex_match(c_command, match, re_new);
    if (match.size() != 0)
    {
        vec.emplace_back(this->ToLower(match.str(1)));
        return;
    }

    // Matches "end" command
    std::regex_match(c_command, match, re_end);
    if (match.size() != 0)
    {
        vec.emplace_back(this->ToLower(match.str(1)));
        return;
    }

    // Matches "load <filepath>" command
    std::regex_match(c_command, match, re_load);
    if (match.size() != 0)
    {
        vec.emplace_back(this->ToLower(match.str(1)));
        {
            match.str(3).empty() ?
                vec.emplace_back(match.str(4))
                :
                vec.emplace_back(match.str(3));
        }
        return;
    }

    // Matches "locate <word> <n>" command
    std::regex_match(c_command, match, re_locate);
    if (match.size() != 0)
    {
        vec.emplace_back(this->ToLower(match.str(1)));
        vec.emplace_back(this->ToLower(match.str(2)));
        vec.emplace_back(match.str(3));
        return;
    }

    // Indicates an invalid command if no match
    vec.emplace_back("INVALID");
}

wl::Op wl::Command::GetOperation() const
{
    return this->op;
}

const std::string& wl::Command::GetFirstArg() const
{
    return this->arg_1;
}

uint32_t wl::Command::GetSecondArg() const
{
    return this->arg_2;
}

void wl::Command::Receive()
{
    std::string command;
    std::cout << ">";
    std::getline(std::cin, command);

    std::vector<std::string> ops;
    this->Parse(command, ops);
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
        if (f.is_open())  // if file can open, then it exists
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

int wl::Dictionary::Node::Diff(std::string& str1, std::string& str2) const
{
    size_t len_1 = str1.size(), len_2 = str2.size();
    size_t max_length = std::min(len_1, len_2);
    int i_diff = -1;
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

void wl::Dictionary::Node::Split(Node* node, int i_diff) const
{
    std::string retain = node->prefix.substr(0, i_diff);
    std::string suffix = node->prefix.substr(i_diff);
    node->prefix = retain;

    Node* split_node = new Node(suffix);
    split_node->counts.swap(node->counts);
    split_node->children.swap(node->children);
    node->children.emplace_back(split_node);
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

uint32_t wl::Dictionary::Node::Search(const std::string& word, uint32_t occurrence) const
{
    const Node* curr = this, * next = nullptr;
    size_t length = word.size();
    for (size_t i = 0; i < length; i++, curr = next)
    {
        std::string sub = word.substr(i);
        next = curr->Next(sub.front());
        if (next == nullptr)
        {
            return 0;
        }
        else
        {
            size_t pre_size = next->prefix.size();
            size_t sub_size = sub.size();
            if (pre_size == sub_size && next->prefix.compare(sub) == 0) // Find exact match
            {
                i += sub_size;  // Add a large enough number to end the loop
            }
            else if (pre_size < sub_size && next->prefix.compare(sub.substr(0, pre_size)) == 0)
            {
                i += pre_size - 1;  // Continue searching the next node
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

void wl::Dictionary::Node::Insert(const std::string& word, uint32_t count)
{
    Node* curr = this, * next = nullptr;
    size_t length = word.size();
    for (size_t i = 0; i < length; i++, curr = next)
    {
        std::string sub = word.substr(i);
        next = curr->Next(sub.front());
        if (next == nullptr)  // No match found
        {
            // Create a new node with the entire remaining string
            // as its prefix, and finish insertion
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
                if (sub_size > pre_size)  // Find partial match
                {
                    i += pre_size - 1;  // Continue searching
                }
                else if (sub_size < pre_size)  // Find complete match
                {
                    // Deal with the case where "so" is to be inserted in
                    // the node whose prefix is "song", i.e., the 
                    // original node needs to be split into "so" -> "ng",
                    // and finish insertion
                    this->Split(next, sub_size);
                    next->counts.emplace_back(count);

                    break;
                }
                else  // Find exact match, like "sing" and "sing"
                {
                    // Update the word count at current occurrence
                    next->counts.emplace_back(count);

                    break;
                }
            }
            else  // Find partial match
            {
                // Differing from the above, this deals with the case
                // where "sing" finds "song", which indicates a need of
                // split and coninuation of search (in fact, a new node
                // will always be added at the next iteration).
                this->Split(next, i_diff);
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

void wl::Dictionary::Parse(const std::string& line, std::vector<std::string>& vec) const
{
    std::string word;
    size_t s_index = 0, e_index = 0, length = line.size();
    while (s_index < length && e_index < length)
    {
        char ch = line.at(e_index);
        if (isalnum(ch) || ch == '\'')
        {
            e_index++;
        }
        else
        {
            if (s_index != e_index)
            {
                // Extract valid characters between two invalid characters
                word = line.substr(s_index, e_index - s_index);
                vec.emplace_back(this->ToLower(word));
            }

            // Look for next valid character
            bool found = false;
            for (size_t i = e_index + 1; i < length; i++)
            {
                ch = line.at(i);
                if (isalnum(ch) || ch == '\'')
                {
                    s_index = i;
                    e_index = i;
                    found = true;
                    break;
                }
            }

           
            if (!found)  // Next valid character not found, finish parsing
            {
                s_index = e_index;
                break;
            }
        }
    }

    // Add the last valid word if existed
    if (s_index != e_index)
    {
        word = line.substr(s_index, e_index - s_index);
        vec.emplace_back(this->ToLower(word));
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

void wl::Dictionary::Load(const std::string& path)
{
    if (!this->is_loadable) return;

    std::ifstream f(path);
    std::string line;
    uint32_t total_count = 0;
    std::vector<std::string> buffer;
    if (f.is_open())
    {
        while (std::getline(f, line))  // Parse file line by line
        {
            this->Parse(line, buffer);
            size_t length = buffer.size();
            for (size_t i = 0; i < length; i++)
            {
                this->word_list->Insert(buffer[i], ++total_count);
            }

            buffer.clear();
        }

        f.close();

        this->is_loadable = false;
    }
}

uint32_t wl::Dictionary::Locate(const std::string& word, uint32_t occurrence) const
{
    return this->word_list->Search(word, occurrence);
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

    this->result = -2;  // reset to default value
}

bool wl::Context::Destroyed() const
{
    return this->destroyed;
}

void wl::Context::Execute(const Command& command)
{
    switch (command.GetOperation())
    {
    case wl::Op::END:
        this->destroyed = true;
        this->prev_ops = wl::Op::END;
        break;

    case wl::Op::NEW:
        this->dictionary->New();
        this->prev_ops = wl::Op::NEW;
        break;

    case wl::Op::LOAD:
        // Allow two successive load commands
        if (this->prev_ops == wl::Op::LOAD)
        {
            this->dictionary->New();
        }

        if (this->dictionary->IsLodable())
        {
            this->dictionary->Load(command.GetFirstArg());
            this->prev_ops = wl::Op::LOAD;
        }
        else
        {
            this->result = -1;
        }
        break;

    case wl::Op::LOCATE:
        this->result = this->dictionary->Locate(command.GetFirstArg(), command.GetSecondArg());
        this->prev_ops = wl::Op::LOCATE;
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
