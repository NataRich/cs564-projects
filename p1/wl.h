///////////////////////////////////////////////////////////////////////////////
// 
// Project Name:        Word Locator
// Student:             Zihan Zhao
// Student ID:          908 266 6315
// UW Email Address:    zzhao383@wisc.edu
// 
///////////////////////////////////////////////////////////////////////////////
// 
// This File: wl.h
// Main File: wl.cpp
// 
// Purpose of this file: This file contains the declarations of needed classes,
// functions, etc. which should be implemented in wl.cpp.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <regex>
#include <string>
#include <vector>
#include <cctype>
#include <fstream>
#include <sstream>
#include <iostream>

/// <summary>
/// A scope used to organize identifiers used for Word Locator.
/// </summary>
/// 
/// This namespace contains an enum class `wl::Op` which specifies 6 
/// distinct operations, i.e. each user input must correspond to one of the 6
/// operations; a command parser `wl::Command` which parses and stores the user
/// input; a dictionary `wl::Dictionary` which parses and stores the words read 
/// from the given file in a radix tree for searching/locating; and a context
/// manager `wl::Context` which encapsulates all the details of executions and
/// provides simple interfaces.
namespace wl
{
	/// <summary>
	/// A list of operations that can be perfomed in this program.
	/// </summary>
	enum class Op
	{
		/// <summary>
		/// Waits for the next command, also the default operation.
		/// </summary>
		EMPTY,

		/// <summary>
		/// Ends the program and clears the heap.
		/// </summary>
		END,

		/// <summary>
		/// Clears the old dictionary and creates an empty dictionary.
		/// </summary>
		NEW,

		/// <summary>
		/// Loads words in a file to the empty dictionary usually called after
		/// new command.
		/// </summary>
		LOAD,

		/// <summary>
		/// Searches a given string in the dictionary.
		/// </summary>
		LOCATE,

		/// <summary>
		/// Indicates an invalid command and prints error message.
		/// </summary>
		INVALID
	};

	/// <summary>
	/// An abstract class that provides necessary parsing interfaces.
	/// </summary>
	/// 
	/// `wl::Parser` provides two interfaces `wl::Parser::ToLower()` and 
	/// `wl::Parser::Parse()`, in which the former function has a default 
	/// implementation which can be overwritten, and the latter function 
	/// requires an implementation in derived classes.
	class Parser
	{
	protected:
		/// <summary>
		/// Transforms all uppercase letters of the given string to lowercase 
		/// (ascii letters only).
		/// </summary>
		/// 
		/// This function creates a new empty string first. Then it goes 
		/// through each character in `str` and checks if the current character
		/// can be lowercased. If yes, then append the lowercase letter to the
		/// created empty string; otherwise, append the original character. 
		/// Finally, it returns the newly created string.
		/// 
		/// <param name="str">The ascii string to be lowercased.</param>
		/// <returns>The lowercased ascii string.</returns>
		virtual std::string ToLower(const std::string& str) const;

		/// <summary>
		/// Parses the given string into words and stores them in the given vector.
		/// </summary>
		/// 
		/// This function should parse the string according to respective parsing 
		/// rules. Therefore, it requires to be overwritten in the derived classes.
		/// 
		/// <param name="str">The string to be parsed.</param>
		/// <param name="vec">The store the result of parsing.</param>
		virtual void Parse(const std::string& str, std::vector<std::string>& vec) const = 0;

	public:
		/// <summary>
		/// Clears the dynmically allocated memory space.
		/// </summary>
		virtual ~Parser() {}
	};

	/// <summary>
	/// A command parser that processes and stores user input.
	/// </summary>
	class Command : protected Parser
	{
	private:
		/// <summary>
		/// The operation that this command intends.
		/// </summary>
		/// 
		/// By default, the operation of a command is `wl::Op::EMPTY`.
		Op op;

		/// <summary>
		/// The first argument of this command.
		/// </summary>
		/// 
		/// By default, the first argument is an empty string.
		std::string arg_1;

		/// <summary>
		/// The second argument of this command.
		/// </summary>
		/// 
		/// By default, the second argument is 0.
		uint32_t arg_2;

	private:
		/// <summary>
		/// Modifies the given vector parameter to save the parsing result.
		/// </summary>
		/// 
		/// This function uses regular expressions to match the given command.
		/// If the given command is empty or does not find a match, then it is
		/// an empty command or an invalid command. These regular expressions
		/// define the formats of commands allowed. 
		/// 
		/// <param name="command">The user input.</param>
		/// <param name="vec">The result of parsing.</param>
		void Parse(const std::string& command, std::vector<std::string>& vec) const;

	public:
		/// <summary>
		/// Initializes the operation to be `wl::Op::EMPTY` and the second
		/// argument to be 0.
		/// </summary>
		Command();

	public:
		/// <summary>
		/// Gets the operation of this command.
		/// </summary>
		/// 
		/// <returns>`op`</returns>
		Op GetOperation() const;

		/// <summary>
		/// Gets the first argument of this command.
		/// </summary>
		/// 
		/// <returns>`arg_1`</returns>
		const std::string& GetFirstArg() const;

		/// <summary>
		/// Gets the second argument of this command.
		/// </summary>
		/// 
		/// <returns>`arg_2`</returns>
		uint32_t GetSecondArg() const;

	public:
		/// <summary>
		/// Receives and processes user input.
		/// </summary>
		/// 
		/// This function prints the required prompt to receive the user input,
		/// parses the user input into a vector of strings, and validates each
		/// part to check if they are a part of a valid command. If yes, then
		/// it will store the corresponding operation and operands; otherwise,
		/// it will set `op` to be `wl::Op::INVALID` to indicate an invalid
		/// command.
		void Receive();
	};

	/// <summary>
	/// A trie-based data structure as a dictionary to load, parse, and store 
	/// texts (or words) read from a given file.
	/// </summary>
	/// 
	/// `wl::Dictionary` has defined a private class `wl::Dictionary::Node` 
	/// which is used to store words, and it provides functions that correspond
	/// to the different operations defined in `wl::Op`.
	class Dictionary : protected Parser
	{
	private:
		/// <summary>
		/// A radix tree that stores the paths to find a given word and the
		/// word counts until the word's nth occurrence.
		/// </summary>
		class Node
		{
		private:
			/// <summary>
			/// A vector of node pointers that contain all the possible next
			/// characters of the character in the current node.
			/// </summary>
			std::vector<Node*> children;

			/// <summary>
			/// A vector of postive integers that contain the word counts.
			/// </summary>
			/// 
			/// At index 0 contains the word count until 1st occurence of the
			/// given string; at index 1 contains the word count until 2nd
			/// occurence of the given string; and so on... If the vector size
			/// is not 0, then it means there exists a word that terminates 
			/// here; otherwise, that word doesn't exist.
			std::vector<uint32_t> counts;

			/// <summary>
			/// The prefix of a word.
			/// </summary>
			/// 
			/// The root node has `prefix` defaulted to an empty string.
			std::string prefix;

		private:
			/// <summary>
			/// Gets the index at which two strings first differ.
			/// </summary>
			/// 
			/// <param name="str1">One of the strings to be compared.</param>
			/// <param name="str2">The other string to be compared.</param>
			/// <returns>The index at which two strings first differ; 
			/// -1, otherwise</returns>
			int Diff(std::string& str1, std::string& str2) const;

			/// <summary>
			/// Splits a node.
			/// </summary>
			/// 
			/// This function splits a node from the given index. Specifically,
			/// if `i_diff` is 1, then for the node "sing", it will split since
			/// the 'i' character. Therefore the result would be "s" -> "ing",
			/// where `counts` of the original "sing" node will be copied to
			/// the split node "ing", and "ing" becomes a child of "s",
			/// 
			/// <param name="node">The node to be split.</param>
			/// <param name="i_diff">The index at which the split starts</param>
			void Split(Node* node, int i_diff) const;

			/// <summary>
			/// Returns the reference of node whose `prefix` begins with `next_ch`.
			/// </summary>
			/// 
			/// This function loops through the current node's `children` to 
			/// find the first and only match where the first character of the 
			/// child's `prefix` is `next_ch`.
			/// 
			/// <param name="next_ch">The next char to be searched for.</param>
			/// <returns>The node pointer that contains `next_ch` or `nullptr`
			/// if not found.</returns>
			Node* Next(char next_ch) const;

		public:
			/// <summary>
			/// Initializes `prefix`, defaulted to "".
			/// </summary>
			/// 
			/// <param name="prefix">The prefix of the node.</param>
			Node(std::string prefix);

			/// <summary>
			/// Clears the dynamically allocated memory.
			/// </summary>
			~Node();

		public:
			/// <summary>
			/// Returns the word count until `occurrence`th occurrence of `word`.
			/// </summary>
			/// 
			/// This function searches for `word` iteratively to save memory used
			/// on stack.
			/// 
			/// <param name="word">The word to be searched for.</param>
			/// <param name="occurrence">The occurrence of the word.</param>
			/// <returns>0 if not found; positive integer, otherwise.</returns>
			uint32_t Search(const std::string& word, uint32_t occurrence) const;

		public:
			/// <summary>
			/// Inserts `word` and word `count` into the data structure.
			/// </summary>
			/// 
			/// This function inserts the word iteratively to save memory used
			/// on stack.
			/// 
			/// <param name="word">The word to be stored.</param>
			/// <param name="count">The word count until this word.</param>
			void Insert(const std::string& word, uint32_t count);
		};

	private:
		/// <summary>
		/// The root of the radix tree.
		/// </summary>
		Node* word_list;

		/// <summary>
		/// A bool indicating whether new set of words can be loaded to memory.
		/// </summary>
		bool is_loadable;

	private:
		/// <summary>
		/// Parses a line of a text file into an array of valid words.
		/// </summary>
		/// 
		/// This function treats any characters besides letters, numbers and 
		/// apostrophes as whitespaces, so any parsed words, or words considered
		/// valid, contain only letters, numbers, and apostrophes. Meanwhile,
		/// all uppercase letters are transfomred into lowercase letters.
		/// 
		/// <param name="line">A line of words.</param>
		/// <param name="vec">The result of parsing.</param>
		void Parse(const std::string& line, std::vector<std::string>& vec) const;

	public:
		/// <summary>
		/// Initializes `word_list` as the root node and `is_loadable` to true so
		/// that the first load command doesn't require a new command in prior.
		/// </summary>
		Dictionary();

		/// <summary>
		/// Clears the dynamically allocated memory.
		/// </summary>
		~Dictionary();

	public:
		/// <summary>
		/// Checks if the dictionary is prepared to load a new set of words.
		/// </summary>
		/// 
		/// <returns>`is_loadable`</returns>
		bool IsLodable() const;

		/// <summary>
		/// Returns the word count until `occurrence`th occurrence of `word`.
		/// </summary>
		/// 
		/// <param name="word">The word to be searched for.</param>
		/// <param name="occurrence">The occurrence of the word.</param>
		/// <returns>0 if not found; any positive integer, otherwise.</returns>
		uint32_t Locate(const std::string& word, uint32_t occurrence) const;

	public:
		/// <summary>
		/// Sets `is_loadable` to the given state.
		/// </summary>
		/// 
		/// <param name="is_lodable">State of the dictionary.</param>
		void SetLodable(bool is_lodable);

		/// <summary>
		/// Clears the current dictionary and creates a new one.
		/// </summary>
		void New();

		/// <summary>
		/// Loads the words in the given file to the radix tree.
		/// </summary>
		/// 
		/// <param name="path">The file path.</param>
		void Load(const std::string& path);
	};

	/// <summary>
	/// A context manager that encapsulates executions of different commands.
	/// </summary>
	class Context
	{
	private:
		/// <summary>
		/// The dictionary that stores words and corresponding word counts.
		/// </summary>
		Dictionary* dictionary;

		/// <summary>
		/// A number indicating the result of the execution of a command.
		/// </summary>
		int64_t result;

		/// <summary>
		/// A bool value indicating whether the program should destroy.
		/// </summary>
		bool destroyed;

		/// <summary>
		/// A record of the previous operation so that the program can allow 
		/// any two successive load commands.
		/// </summary>
		Op prev_ops;

	private:
		/// <summary>
		/// Prints the result based on the private member `result`.
		/// </summary>
		void PrintResult();

	public:
		/// <summary>
		/// Initializes a dictionary, `result` to -2 (a default value), 
		/// `destroyed` to `false`, and previous operations to `wl::Op::EMPTY`.
		/// </summary>
		Context();

		/// <summary>
		/// Clears the dynamically allocated memory.
		/// </summary>
		~Context();

	public:
		/// <summary>
		/// Checks if the context has been destroyed.
		/// </summary>
		/// 
		/// <returns>`destroyed`</returns>
		bool Destroyed() const;

	public:
		/// <summary>
		/// Executes a command based on its operation type and operands.
		/// </summary>
		/// 
		/// <param name="command">A command object that has received input.</param>
		void Execute(const Command& command);
	};
};
