#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

typedef std::pair<std::string, int> WordScore;
typedef std::map<std::string, int> WordScoreMap;
typedef std::vector<WordScore> WordScoreList;

class Solver
{
public:
    Solver(int minLength, int maxLength, const std::string &query);
    ~Solver();

    inline bool queryContains(const std::string &word);

    bool seed(const std::string &filename);

    std::string sanitize(const std::string &word);

    void dump();
    void permute(int length);
    void solve();

protected:
    int minLength_;
    int maxLength_;
    std::string query_;
    std::string sortedQuery_;
    std::vector<WordScoreMap> scores_;
};

Solver::Solver(int minLength, int maxLength, const std::string &query)
: minLength_(minLength)
, maxLength_(maxLength)
, query_(query)
{
    for(int i = 0; i <= maxLength_; ++i) {
        scores_.push_back(WordScoreMap());
    }

    sortedQuery_ = sanitize(query_);
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
    std::ifstream f(filename);
    if(!f) {
        return false;
    }

    std::string word;
    while(std::getline(f, word)) {
        int length = (int)word.size();
        if((length < minLength_) || (length > maxLength_))
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

void Solver::dump()
{
    printf("Current word list counts:\n");
    for(int i = 0; i <= maxLength_; ++i) {
        printf("* Scores[%d]: %d\n", i, (int)scores_[i].size());
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

void Solver::permute(int length)
{
    for(int scores1Length = length - minLength_; scores1Length >= minLength_; --scores1Length) {
        int scores2Length = length - scores1Length;
        if(scores2Length > scores1Length)
            break;

        WordScoreMap &scores1 = scores_[scores1Length];
        WordScoreMap &scores2 = scores_[scores2Length];
        WordScoreMap &destScores = scores_[length];
        if((scores1.size() == 0) || (scores2.size() == 0))
            continue;

        printf("permute into list[%d] -> list[%d] x list[%d] = %d * %d = %d combinations\n",
            length,
            scores1Length,
            scores2Length,
            (int)scores1.size(),
            (int)scores2.size(),
            (int)(scores1.size() * scores2.size()));

        for(WordScoreMap::iterator scores1It = scores1.begin(); scores1It != scores1.end(); ++scores1It) {
            for(WordScoreMap::iterator scores2It = scores2.begin(); scores2It != scores2.end(); ++scores2It) {
                std::string combined = scores1It->first + " " + scores2It->first;
                destScores[combined] = scores1It->second + scores2It->second;
            }
        }
    }
}

void Solver::solve()
{
    int queryLength = (int)sortedQuery_.size();

    for(int i = 0; i <= sortedQuery_.size(); ++i) {
        permute(i);
    }

    printf("\nFound %d possible anagrams.\n",
		(int)scores_[queryLength].size());

    WordScoreList answers;
    for(WordScoreMap::iterator it = scores_[queryLength].begin(); it != scores_[queryLength].end(); ++it) {
        if(queryContains(it->first))
            answers.push_back(*it);
    }

    // Sort by score so cooler anagrams are first
    std::sort(answers.begin(), answers.end(), sortScores);

    printf("Found %d answers.\n", (int)answers.size());
    for(WordScoreList::iterator it = answers.begin(); it != answers.end(); ++it) {
        printf(" * %s [score: %d]\n", it->first.c_str(), it->second);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        printf("Syntax: anagram [letters]\n");
        return 0;
    }

    std::string query = argv[1];

    Solver solver(2, (int)query.size(), query);
    solver.seed("data/words");
    solver.dump();
    solver.solve();

    return 0;
}
