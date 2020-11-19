#include <iostream>
#include <string>
#include <list>
#include <boost/program_options.hpp>
#include <duplicate_files_scanner.hpp>

std::string search_directory;

bool recurse;

bool follow_symlinks;
size_t min_size = 0;
size_t max_size = SIZE_MAX;
bool skip_hidden = false;
bool print_size = false;
bool print_time = false;
bool print_summary = false;
bool quiet = false;
bool delete_files = false;
bool no_prompt = false;
bool ctime_sort = false;
bool atime_sort = false;
bool mtime_sort = true;
bool name_sort = false;
bool size_sort = false;
bool descending = false;

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

void scan_started_callback(const std::filesystem::path& search_path)
{
    std::cout << "Starting scan of directory " << search_path << "...\n";
}

void scan_progress_callback(const std::filesystem::path& search_path, uintmax_t examined, uintmax_t sets_found)
{
    std::locale user_locale("");
    std::cout.imbue(user_locale);
    std::cout << "Files examined: " << examined << ", duplicate sets found: " << sets_found << "\r";
}

void scan_completed_callback(const std::filesystem::path& , uintmax_t examined, uintmax_t sets_found)
{
    std::locale user_locale("");
    std::cout.imbue(user_locale);
    std::cout << "scan completed, with " << examined << " files examined and " << sets_found << " duplicate sets found." << std::endl;
}

void scan_error_callback(const std::filesystem::path& search_path, const std::filesystem::path& error_file, const std::error_condition& error)
{
    std::cerr << "An error occurred while scanning the file " << error_file << " - " << error.message() << std::endl;
}

int main(int argc, char **argv)
{
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
            ("symlinks,s", boost::program_options::bool_switch(&follow_symlinks), "Follow symbolic links.")
            ("minsize", boost::program_options::value<size_t >(&min_size), "Consider only files greater than or equal to SIZE.")
            ("maxsize", boost::program_options::value<size_t >(&max_size), "Consider only files less than or equal to SIZE.")
            ("nohidden", boost::program_options::bool_switch(&skip_hidden), "Do not consider hidden files.")
            ("size,S", boost::program_options::bool_switch(&print_size), "Show size of duplicate files.")
            ("time,t", boost::program_options::bool_switch(&print_time), "Show modified time of duplicate files.")
            ("summarise,m", boost::program_options::bool_switch(&print_summary), "Show a summary of the duplicate file information.")
            ("quiet,q", boost::program_options::bool_switch(&quiet), "Hide the progress indicator.")
            ("delete,d", boost::program_options::bool_switch(&delete_files), "Show modified time of duplicate files.")
            ("noprompt,N", boost::program_options::bool_switch(&no_prompt), "Together with --delete, preserve the first file in each set of duplicates and delete the rest without prompting.")
            ("ctime,c", boost::program_options::bool_switch(&ctime_sort), "List files in order of their creation time.")
            ("atime,a", boost::program_options::bool_switch(&atime_sort), "List files in order of their last access time.")
            ("mtime,M", boost::program_options::bool_switch(&mtime_sort), "List files in order of their last modified time.")
            ("name,n", boost::program_options::bool_switch(&name_sort), "List files sorted by their filename.")
            ("size,b", boost::program_options::bool_switch(&size_sort), "List files sorted by their size.")
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

        oasis::filesystem::duplicate_files_scanner scanner;
        scanner.set_follow_symlinks(follow_symlinks);
        scanner.set_minimum_size(min_size);
        scanner.set_maximum_size(max_size);
        scanner.set_skip_hidden_files(skip_hidden);

        if (!quiet)
        {
            scanner.set_scan_started_callback(scan_started_callback);
            scanner.set_scan_progress_callback(scan_progress_callback);
            scanner.set_scan_completed_callback(scan_completed_callback);
            scanner.set_scan_error_callback(scan_error_callback);
        }

        scanner.perform_scan(search_directory, recurse);

        std::cout << "\nTesting forward iterator:\n\n";

        size_t i = 1;
        for (const auto& ds : scanner)
        {
            std::cout << "Set " << i++ << "\n";
            for (const auto& dp : ds)
            {
                std::cout << " " << dp << "\n";
            }
        }

        std::cout << "\nTesting reverse iterator:\n\n";

        i--;
        for (auto it = scanner.rbegin(); it != scanner.rend(); it++)
        {
            const auto& ds = *it;
            std::cout << "Set " << i-- << "\n";
            for (const auto& dp : ds)
            {
                std::cout << " " << dp << "\n";
            }
        }

        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
}
