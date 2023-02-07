#include <iostream> //cout cin cerr
#include <string> 
#include <sstream> //stringstream
#include <vector>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <fstream>
#include <cctype>

class SortWords
{
    public:
        SortWords(int argc,
                  char *argv[],
                  char delimit,
                  char sort,
                  std::string outfile);
        ~SortWords() {}

    private:
        char m_sort;
        std::string m_outfile;
        std::vector<std::thread> m_threads;
        std::vector<std::string> m_fileNames;
        std::map<std::string, int> m_wordCount;
        std::unordered_map<char, char> m_delimap;
        typedef typename std::unique_lock<std::mutex> m_unique_lock;
}; // class SortWords

void Reader(const std::string &fileName,
            std::map<std::string, int> &wordCount);

void Writer(std::string fileName,
            std::map<std::string, int> &wordCount,
            char delimiter,
            char sortMethod);

SortWords::SortWords(int argc,
                    char *argv[],
                     char delimit,
                     char sort,
                     std::string outfile) : m_sort(sort), m_outfile(outfile)
{
    char delimiter = ' ';
    for (int i = 1; i < argc; i++)
    {
        m_fileNames.push_back(argv[i]); 
    } 

    m_delimap['s'] = ' ';
    m_delimap['c'] = ',';
    m_delimap['n'] = 10; 

    delimiter = m_delimap[delimit];
 
    for (size_t i = 0; i < m_fileNames.size(); i++)
    {
        m_threads.emplace_back(Reader, m_fileNames[i], std::ref(m_wordCount));
    }

    for (size_t i = 0; i < m_fileNames.size(); i++)
    {
        m_threads[i].join();
    }
  
    std::thread task_write(Writer,
                           m_outfile,
                           std::ref(m_wordCount),
                           delimiter,
                           m_sort);
    task_write.join();
}

void SplitWords(const std::string &line,
                 std::map<std::string, int> &wordCount)
{
    std::stringstream stream(line);
    std::string word;

    while (std::getline(stream, word, ' '))
    {
        std::mutex readMutex;

        word[0]  = tolower(word[0]);
        for (auto c : word)
        {
            if (ispunct(c))
            {
                word.erase(word.find(c));
            }
        }

        readMutex.lock();
        ++wordCount[word];
        readMutex.unlock();
    }
}

void Reader(const std::string &fileName,
            std::map<std::string, int> &wordCount)
{
    std::ifstream my_file(fileName.c_str());
    std::string line;

    while (std::getline(my_file, line))
    {
        SplitWords(line, wordCount);
    }

    my_file.close();
}

void Writer(std::string fileName,
             std::map<std::string, int> &wordCount,
             char delimiter,
             char sortMethod)
{
    std::pair<std::string, int> maxWord;
    std::mutex writeMutex;
    std::unique_lock<std::mutex> lock(writeMutex);

    std::ofstream result(fileName);

    maxWord.first = " ";
    maxWord.second = 0;

    if (sortMethod == 'a')
    {
        for (auto word = wordCount.begin(); word != wordCount.end(); ++word)
        {
            result << word->first << delimiter;
            if (word->second > maxWord.second)
            {
                maxWord.first = word->first;
                maxWord.second = word->second;
            }

        }
    }
    else if (sortMethod == 'd')
    {
        for (auto word = wordCount.rbegin(); word != wordCount.rend(); ++word)
        {
            result << word->first << ',';
            if (word->second >= maxWord.second)
            {
                maxWord.first = word->first;
                maxWord.second = word->second;
            }
        }
    }
    
    result << std::endl;
    result << "The most frequent word: '" <<  maxWord.first 
    << "', count: " << maxWord.second << std::endl;

    result.close();
}

int main(int argc , char *argv[])
{
    char sort = 'a';
    char delimit = ' ';
    std::string outfile = " ";

    std::cout << "Hello, please enter your sort options" << std::endl;
    std::cout << "sort: a-up d-down" << std::endl;
    std::cin >> sort;
    std::cout << "split: s-space, c-comma, n-new line" << std::endl;
    std::cin >> delimit;
    std::cout << "outfilename:" << std::endl;
    std::cin >> outfile;

    SortWords obj(argc, argv, delimit, sort, outfile);

    std::cout <<"Your order is ready, look in output file please" << std::endl;

    return 0;
}