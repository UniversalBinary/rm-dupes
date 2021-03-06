/*
 * Copyright (c) 2020 Chris Morrison
 *
 * Filename: main.cpp
 * Created on Thu Dec 24 2020
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <list>
#include <bitset>
#include <boost/program_options.hpp>
#include "oasis/duplicate_files_scanner.hpp"
#include "oasis/duplicate_file_set.hpp"

#define OP_SUMMARY 0
#define OP_DELETE  1
#define OP_LINK    2

#define SORT_FILENAME    0
#define SORT_CREATION    1
#define SORT_LAST_READ 2
#define SORT_LAST_WRITE  3
#define SORT_FILE_SIZE   4


std::string search_directory;
bool recurse = false;
bool follow_symlinks = false;
size_t min_size = 0;
size_t max_size = SIZE_MAX;
bool skip_hidden = false;
bool print_size = false;
bool print_time = false;
bool print_summary = false;
bool quiet = false;
bool delete_files = false;
bool no_prompt = false;
bool creation_sort = false;
bool last_read_sort = false;
bool last_write_sort = true;
bool name_sort = false;
bool size_sort = false;
bool descending = false;
bool link_files;
boost::mutex conlock;

template<typename T>
int perform_operation(const std::string& search_dir, const std::bitset<3>& operation);

inline void print_usage(const boost::program_options::options_description &desc)
{
    std::cout << "Usage: " << "rm-dupes" << " <search_directory> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << desc << std::endl;
}

inline void print_version(const boost::program_options::options_description &desc)
{
    std::cout << "rm-dupes utility version " << VERSION << " (ɔ) Copyleft 2020 Chris Morrison" << std::endl;
}

inline void print_help(const boost::program_options::options_description &desc)
{
    print_usage(desc);
    print_version(desc);
}

// Callbacks

void scan_started_callback(const boost::filesystem::path& search_path)
{
    std::cout << "Starting scan of directory " << search_path.string() << "...\n";
}

void scan_progress_callback(const boost::filesystem::path& search_path, uintmax_t files_encountered, uintmax_t sets_found)
{
    std::locale user_locale("");
    std::cout.imbue(user_locale);
    std::cout << "\033[KSearching path " << search_path.string() << ", files encountered: " << files_encountered << ", duplicate sets found: " << sets_found << "\r" << std::flush;
}

void scan_completed_callback(const boost::filesystem::path& search_path, uintmax_t examined, uintmax_t duplicate_file_count, uintmax_t sets_found, uintmax_t space_occupied)
{
    std::locale user_locale("");
    std::cout.imbue(user_locale);
    std::cout << "\033[KFiles examined: " << examined << ", duplicate sets found: " << sets_found << std::endl;
}

void scan_error_callback(const boost::filesystem::path& search_path, const boost::filesystem::path& error_file, const std::error_condition& error)
{
    conlock.lock();
    if (error_file.empty())
    {
        std::cerr << "\033[31;1mAn error occurred while scanning files and directories in " << search_path << " - " << error.message() << "\033[0m" << std::endl;
    }
    else
    {
        std::cerr << "\033[31;1mAn error occurred while scanning the file or directory " << error_file << " - " << error.message() << "\033[0m" << std::endl;
    }
    conlock.unlock();
}

int main(int argc, char **argv)
{
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test1("thisisthehash1");
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test2(L"thisisthehash2");
    const char *t1 = "thisisthehash3";
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test3(t1);
    std::string t2 = "thisisthehash4";
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test4(t2);
    const wchar_t *t3 = L"thisisthehash5";
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test5(t3);
    std::wstring t4 = L"thisisthehash6";
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test6(t4);
    const char32_t *t5 = U"thisisthehash7";
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test7(t5);
    std::u32string t6 = U"thisisthehash8";
    oasis::filesystem::duplicate_file_set<oasis::filesystem::sort_by_filename> test8(t6);

    if (test1 != test2) std::cout << "Test 1/29: Inequality test holds\n";
    test1 = test2;
    if (test1 == test2) std::cout << "Test 2/29: Equality test holds following assignment\n";
    if (test3 == t1) std::cout << "Test 3/29: Equality test holds\n";
    if (test3 == "thisisthehash3") std::cout << "Test 4/8: Equality test holds\n";
    if (test4 == t2) std::cout << "Test 5/29: Equality test holds\n";
    if (test5 == t3) std::cout << "Test 6/29: Equality test holds\n";
    if (test5 == L"thisisthehash5") std::cout << "Test 7/8: Equality test holds\n";
    if (test6 == t4) std::cout << "Test 8/29: Equality test holds\n";
    if (test7 == t5) std::cout << "Test 9/29: Equality test holds\n";
    if (test8 == t6) std::cout << "Test 10/29: Equality test holds\n";
    if (test3 > test1) std::cout << "Test 11/29: Greater than test holds\n";
    if (test6 > t1) std::cout << "Test 12/29: Greater than test holds\n";
    if (test6 > "thisisthehash3") std::cout << "Test 13/29: Greater than test holds\n";
    if (test6 > L"thisisthehash3") std::cout << "Test 14/29: Greater than test holds\n";
    if (test6 > U"thisisthehash3") std::cout << "Test 15/29: Greater than test holds\n";
    if (test8 > t2) std::cout << "Test 16/29: Greater than test holds\n";
    if (test8 > t3) std::cout << "Test 17/29: Greater than test holds\n";
    if (test8 > t4) std::cout << "Test 18/29: Greater than test holds\n";
    if (test8 > t5) std::cout << "Test 19/29: Greater than test holds\n";
    if (test7 < t6) std::cout << "Test 20/29: Less than test holds\n";
    if (test1 < "thisisthehash3") std::cout << "Test 21/29: Less than test holds\n";
    if (test1 < L"thisisthehash3") std::cout << "Test 22/29: Less than test holds\n";
    if (test1 < U"thisisthehash3") std::cout << "Test 23/29: Less than test holds\n";
    if (test1 < t1) std::cout << "Test 24/29: Less than test holds\n";
    if (test1 < t2) std::cout << "Test 25/29: Less than test holds\n";
    if (test1 < t3) std::cout << "Test 26/29: Less than test holds\n";
    if (test1 < t4) std::cout << "Test 27/29: Less than test holds\n";
    if (test1 < t5) std::cout << "Test 28/29: Less than test holds\n";
    if (test1 < t6) std::cout << "Test 29/29: Less than test holds\n";

    return 0;
    // Declare the hidden (positional) options.
    boost::program_options::options_description hidden("Hidden Options");
    hidden.add_options()
            ("search-directory", boost::program_options::value<std::string>(&search_directory), "Directory to search.");
    // Declare other options.
    boost::program_options::options_description desc("Options");
    desc.add_options()
            ("help,h", "Display help message and exit.")
            ("version,v", "Display version information and exit.")
            ("recurse,r", boost::program_options::bool_switch(&recurse), "For every directory encountered follow subdirectories within.")
            ("symlinks,s", boost::program_options::bool_switch(&follow_symlinks), "Follow symbolic links instead of skipping them.")
            ("minsize", boost::program_options::value<size_t >(&min_size), "Consider only files greater than or equal to SIZE.")
            ("maxsize", boost::program_options::value<size_t >(&max_size), "Consider only files less than or equal to SIZE.")
            ("nohidden", boost::program_options::bool_switch(&skip_hidden), "Do not consider hidden files.")
            ("size,S", boost::program_options::bool_switch(&print_size), "Show size of duplicate files.")
            ("time,t", boost::program_options::bool_switch(&print_time), "Show modified time of duplicate files.")
            ("quiet,q", boost::program_options::bool_switch(&quiet), "Hide the progress indicator.")
            ("summarise,m", boost::program_options::bool_switch(&print_summary), "Show a summary of the duplicate file information.")
            ("delete,d", boost::program_options::bool_switch(&delete_files), "Print a list of the duplicate files in a set and prompt for one to keep, the remainder will be deleted.")
            ("link,l", boost::program_options::bool_switch(&link_files), "Print a list of the duplicate files in a set and prompt for one to keep, the remainder will be deleted and replaced with a symbolic link to the file that has been kept.")
            ("noprompt,N", boost::program_options::bool_switch(&no_prompt), "Together with --delete or --link, preserve the first file in each set of duplicates and delete or link to the rest without prompting.")
            ("creation-time,c", boost::program_options::bool_switch(&creation_sort), "List the duplicate files, in each set, in order of their creation time.")
            ("last-read-time,a", boost::program_options::bool_switch(&last_read_sort), "List the duplicate files, in each set, in order of their last read time.")
            ("last-write-time,M", boost::program_options::bool_switch(&last_write_sort), "List the duplicate files, in each set, in order of their last write time.")
            ("name,n", boost::program_options::bool_switch(&name_sort), "List the duplicate files, in each set, in order of their file name (This is the default).")
            ("size,b", boost::program_options::bool_switch(&size_sort), "List the duplicate files, in each set, in order of their size.")
            ("descending", boost::program_options::bool_switch(&descending), "Files will be sorted in descending order.");

    try
    {
        // Set up positional options
        boost::program_options::positional_options_description p;
        p.add("search-directory", 1);
        p.add("extension", -1);

        // Declare an options description instance which will include all the options.
        boost::program_options::options_description all("All options");
        all.add(hidden).add(desc);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
        boost::program_options::notify(vm);

        // If help was requested, print the help message.
        if (vm.count("help"))
        {
            print_help(desc);
            return 0;
        }
        if (vm.count("version"))
        {
            print_version(desc);
            return 0;
        }

        // Get an iterator for the search directory.
        if (!vm.count("search-directory"))
        {
            std::cerr << "Please specify a search directory." << std::endl;
            return 1;
        }

        // Get the operation flags into a bitfield
        std::bitset<3> op_flags(0);
        op_flags[OP_SUMMARY] = print_summary;
        op_flags[OP_DELETE] = delete_files;
        op_flags[OP_LINK] = link_files;

        if (!op_flags.any())
        {
            std::cerr << "Error: no operation requested - please use the --summary, --delete or --link switch to specify what should be done." << std::endl;
            return 1;
        }

        if (op_flags.count() != 1)
        {
            std::cerr << "Error: the --delete and --link switches are mutually exclusive and cannot be used together." << std::endl;
            return 1;
        }

        std::bitset<5> sort_opts(0);
        sort_opts[SORT_FILENAME] = name_sort;
        sort_opts[SORT_CREATION] = creation_sort;
        sort_opts[SORT_LAST_READ] = last_read_sort;
        sort_opts[SORT_LAST_WRITE] = last_write_sort;
        sort_opts[SORT_FILE_SIZE] = size_sort;

        if (!sort_opts.any()) sort_opts[SORT_FILENAME] = true;

        if (sort_opts.count() != 1)
        {
            std::cerr << "Error: the --name, --size, --creation-time, --last-read-time and --last-write-time switches are mutually exclusive and cannot be used together." << std::endl;
            return 1;
        }

        if (sort_opts.test(SORT_FILENAME)) return perform_operation<oasis::filesystem::sort_by_filename>(search_directory, op_flags);
        if (sort_opts.test(SORT_CREATION)) return perform_operation<oasis::filesystem::sort_by_creation_time>(search_directory, op_flags);
        if (sort_opts.test(SORT_LAST_READ)) return perform_operation<oasis::filesystem::sort_by_last_write_time>(search_directory, op_flags); // Not available yet.
        if (sort_opts.test(SORT_LAST_WRITE)) return perform_operation<oasis::filesystem::sort_by_last_write_time>(search_directory, op_flags);
        if (sort_opts.test(SORT_FILE_SIZE)) return perform_operation<oasis::filesystem::sort_by_file_size>(search_directory, op_flags);

        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
}

template<typename T>
int perform_operation(const std::string& search_dir, const std::bitset<3>& operation)
{
    oasis::filesystem::duplicate_files_scanner<T> scanner(search_directory);
    scanner.set_follow_symlinks(follow_symlinks);
    scanner.set_minimum_size(min_size);
    scanner.set_maximum_size(max_size);
    scanner.set_skip_hidden_files(skip_hidden);

    if (!quiet)
    {
        scanner.set_scan_started_callback(scan_started_callback);
        scanner.set_scan_progress_callback(scan_progress_callback);
        scanner.set_scan_error_callback(scan_error_callback);
        scanner.set_scan_completed_callback(scan_completed_callback);
    }

    scanner.perform_scan(recurse);

    if (operation.test(OP_SUMMARY))
    {
        std::cout << "\nScan completed:-\n";
        std::cout << "  Files examined:        " << scanner.files_examined() << "\n";
        std::cout << "  Duplicate files found: " << scanner.file_count() << "\n";
        std::cout << "  Duplicate sets found:  " << scanner.set_count() << "\n";
        std::cout << "  Space occupied:        " << oasis::storage_formatter(scanner.space_occupied()) << std::endl;

        return 0;
    }

    size_t i = 1;
    for (const auto& ds : scanner)
    {
        i = 1;
        for (const auto& dp : ds)
        {
            std::cout << std::left << std::setfill(' ') << std::setw(10) << i << " " << dp;
            if (print_size) std::cout << " (" << boost::filesystem::file_size(dp) << ")";
        }
    }

    return 0;
}
