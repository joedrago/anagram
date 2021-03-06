#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <string.h>

typedef std::pair<std::string, int> WordScore;
typedef std::map<std::string, int> WordScoreMap;
typedef std::vector<WordScore> WordScoreList;

class Solver
{
public:
    Solver(const std::string &query);
    ~Solver();

    inline bool queryContains(const std::string &word);

    bool seed(const std::string &filename);

    std::string sanitize(const std::string &word);

    void dump(bool dumpWords = false);
    int permute(int length, int minLength);
    void solve();

    void forceAll() { forceAll_ = true; }

protected:
    int maxLength_;
    std::string query_;
    std::string sortedQuery_;
    std::vector<WordScoreMap> scores_;
    bool forceAll_;
};

Solver::Solver(const std::string &query)
: query_(query)
, forceAll_(false)
{
    sortedQuery_ = sanitize(query_);
    maxLength_ = (int)sortedQuery_.size();
    for(int i = 0; i <= maxLength_; ++i) {
        scores_.push_back(WordScoreMap());
    }
}

Solver::~Solver()
{
}

inline bool Solver::queryContains(const std::string &word)
{
    if(!word.size()) {
        return false;
    }

    std::string sortedWord = sanitize(word);

    const char *w = sortedWord.c_str();
    const char *q = sortedQuery_.c_str();

    while(*q) {
        if(*w == *q) {
            ++w;
        }
        ++q;

        if(!*w)
            return true;
    }

    return false;
}

bool Solver::seed(const std::string &filename)
{
    std::ifstream f(filename.c_str());
    if(!f) {
        return false;
    }

    std::string word;
    while(std::getline(f, word)) {
        int length = (int)word.size();
        if(length > maxLength_)
            continue;
        if(!queryContains(word))
            continue;

        scores_[length][word] = length * length;
    }

    return true;
}

std::string Solver::sanitize(const std::string &word)
{
    std::string sortedWord = word;
    sortedWord.erase(std::remove(sortedWord.begin(), sortedWord.end(), ' '), sortedWord.end());
    std::sort(sortedWord.begin(), sortedWord.end());
    return sortedWord;
}

void Solver::dump(bool dumpWords)
{
    fprintf(stderr, "Current word list counts:\n");
    for(int i = 0; i <= maxLength_; ++i) {
        printf("* Scores[%d]: %d\n", i, (int)scores_[i].size());
        if(dumpWords) {
            for(WordScoreMap::iterator it = scores_[i].begin(); it != scores_[i].end(); ++it) {
                fprintf(stderr, "  * %s\n", it->first.c_str());
            }
        }
    }
}

static bool sortScores(WordScore a, WordScore b)
{
    if(b.second == a.second) {
        // Sort identical scores alphabetically
        return a.first < b.first;
    }
    return (b.second < a.second);
}

int Solver::permute(int length, int minLength)
{
    int iterations = 0;
    for(int scores1Length = length - minLength; scores1Length >= minLength; --scores1Length) {
        int scores2Length = length - scores1Length;
        if(scores2Length > scores1Length)
            break;

        WordScoreMap &scores1 = scores_[scores1Length];
        WordScoreMap &scores2 = scores_[scores2Length];
        WordScoreMap &destScores = scores_[length];
        if((scores1.size() == 0) || (scores2.size() == 0))
            continue;

        iterations += (int)(scores1.size() * scores2.size());

        fprintf(stderr, "* permute into list[%d] -> list[%d] x list[%d] = %d * %d = %d combinations\n",
            length,
            scores1Length,
            scores2Length,
            (int)scores1.size(),
            (int)scores2.size(),
            (int)(scores1.size() * scores2.size()));

        for(WordScoreMap::iterator scores1It = scores1.begin(); scores1It != scores1.end(); ++scores1It) {
            for(WordScoreMap::iterator scores2It = scores2.begin(); scores2It != scores2.end(); ++scores2It) {
                // sort the word combos prior to concat to eliminate word combo dupes
                std::string combined;
                if(scores1It->first < scores2It->first)
                    combined = scores1It->first + " " + scores2It->first;
                else
                    combined = scores2It->first + " " + scores1It->first;

                // Only add the combo if it could ever be a part an anagram of query_
                if(queryContains(combined))
                    destScores[combined] = scores1It->second + scores2It->second;
            }
        }
    }
    return iterations;
}

void Solver::solve()
{
    int queryLength = (int)sortedQuery_.size();

    int minLength = (queryLength >> 1) - 2;
    if(minLength < 1)
        minLength = 1;
    if(forceAll_) {
        fprintf(stderr, "Force all enabled, setting min length to 1.\n");
        minLength = 1;
    }

    fprintf(stderr, "Finding anagram for word '%s' (letters [%s]), length range [%d-%d].\n",
        query_.c_str(),
        sortedQuery_.c_str(),
        minLength,
        maxLength_);

    int iterations = 0;
    for(int i = 0; i <= sortedQuery_.size(); ++i) {
        iterations += permute(i, minLength);
    }
    fprintf(stderr, "Total iterations: %d\n", iterations);

    fprintf(stderr, "\nFound %d possible anagrams.\n",
		(int)scores_[queryLength].size());

    WordScoreList answers;
    for(WordScoreMap::iterator it = scores_[queryLength].begin(); it != scores_[queryLength].end(); ++it) {
        if(queryContains(it->first))
            answers.push_back(*it);
    }

    // Sort by score so cooler anagrams are first
    std::sort(answers.begin(), answers.end(), sortScores);

    fprintf(stderr, "Found %d answers.\n", (int)answers.size());
    for(WordScoreList::iterator it = answers.begin(); it != answers.end(); ++it) {
        printf("%s\n", it->first.c_str());
    }
}

int main(int argc, char *argv[])
{
    std::string query;
    bool all = false;

    for(int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if(!strcmp(arg, "-a")) {
            all = true;
        } else {
            query = arg;
        }
    }

    if(query.size() < 1) {
        fprintf(stderr, "Syntax: anagram [-a] [letters]\n");
        return 0;
    }

    Solver solver(query);
    if(all) {
        solver.forceAll();
    }
    solver.seed("data/words");
    solver.solve();

    return 0;
}
